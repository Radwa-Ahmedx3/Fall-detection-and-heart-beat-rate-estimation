#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Libraries */
#include "../LIB/BIT_MATH.h"
#include "../LIB/STD_TYPES.h"
#include "../HAL/LED/LED.h"
#include "../MCAL/DIO/DIO.h"
#include "../MCAL/I2C/I2C.h"
#include "../MCAL/Timer1/Timer1.h"

/* Wi-Fi / Blynk Config */
#define BAUD 115200
#define MYUBRR 16
#define BLYNK_AUTH_TOKEN "tXaS2EX1wjwMv1SvCIz-si2XfgGck52e"

/* MAX30102 Config */
#define MAX30102_ADDR        0x57
#define MAX30102_WRITE_ADDR  ((MAX30102_ADDR << 1) | 0)
#define MAX30102_READ_ADDR   ((MAX30102_ADDR << 1) | 1)
#define RATE_SIZE 10

/* Global Variables */
u8 rates[RATE_SIZE];
u8 rateSpot = 0;
u32 lastBeat = 0;
float beatsPerMinute;
u8 beatAvg = 0;
volatile u32 g_millis = 0;

/* Prototypes */
void UART_Init(unsigned int ubrr);
void UART_SendString(const char* str);
void UART_SendNumber(u32 num);
uint8_t UART_WaitFor(const char* target);
void UART_Flush(void);
void MAX30102_Init(void);
u32 MAX30102_ReadIR(void);
u8 checkForBeat(u32 irValue);
void send_to_blynk(u8 bpm);

/* Timer Interrupt for Millis */
void __vector_7 (void) __attribute__((signal));
void __vector_7 (void) {
    g_millis++;
}

u32 millis(void) {
    return g_millis;
}

int main(void) {
    /* 1. Hardware Initializations */
    UART_Init(MYUBRR);
    I2C_Masterinit(100000);
    MAX30102_Init();

    // Timer1 for 1ms ticks (CTC Mode)
    Timer1_Init(TIMER1_PRESCALLER64, TIMER1_CTC);
    OCR1A = 249;
    SET_BIT(TIMSK, 4);
    SET_BIT(SREG, 7);

    UART_SendString("\r\n--- System Booting ---\r\n");

    /* 2. ESP8266 / WiFi Setup */
    UART_Flush();
    UART_SendString("AT\r\n");
    UART_WaitFor("OK");

    UART_SendString("AT+CWMODE=1\r\n");
    UART_WaitFor("OK");

    UART_SendString("AT+CIPMUX=0\r\n");
    UART_WaitFor("OK");

    UART_SendString("Connecting to Wi-Fi...\r\n");
    char wifi_cmd[100];
    sprintf(wifi_cmd, "AT+CWJAP=\"nana\",\"01023025564\"\r\n");
    UART_SendString(wifi_cmd);

    if (UART_WaitFor("WIFI GOT IP")) {
        UART_SendString("Wi-Fi Online!\r\n");
    } else {
        UART_SendString("Wi-Fi Timeout/Error\r\n");
    }

    u32 lastBlynkUpdate = 0;
    u32 currentIR = 0;

    while(1) {
        currentIR = MAX30102_ReadIR();
        u32 now = millis();

        // Data Reset Logic (No finger for 3s)
        if (now - lastBeat > 3000 && lastBeat != 0) {
            beatAvg = 0;
            lastBeat = 0;
            rateSpot = 0;
            for(u8 i = 0; i < RATE_SIZE; i++) rates[i] = 0;
            UART_SendString("Sensor Timeout: Resetting BPM\r\n");
        }

        // Heartbeat Detection logic
        if(checkForBeat(currentIR)) {
            if (lastBeat == 0) {
                lastBeat = now;
            } else {
                u32 delta = now - lastBeat;
                lastBeat = now;

                if(delta > 400 && delta < 1500) {
                    beatsPerMinute = 60000.0 / (float)delta;

                    if (beatAvg == 0) {
                        for(u8 i = 0; i < RATE_SIZE; i++) rates[i] = (u8)beatsPerMinute;
                    }

                    rates[rateSpot++] = (u8)beatsPerMinute;
                    rateSpot %= RATE_SIZE;

                    u16 sum = 0;
                    for(u8 i = 0; i < RATE_SIZE; i++) sum += rates[i];
                    beatAvg = (u8)(sum / RATE_SIZE);

                    UART_SendString("Beat Detected! Current Avg BPM: ");
                    UART_SendNumber(beatAvg);
                    UART_SendString("\r\n");
                }
            }
        }

        // Upload to Blynk every 5 seconds
        if (now - lastBlynkUpdate > 5000) {
            if (beatAvg > 0 && currentIR > 50000) {
                send_to_blynk(beatAvg);
                UART_SendString(">> Sent to Blynk: ");
                UART_SendNumber(beatAvg);
                UART_SendString("\r\n");
            }
            lastBlynkUpdate = now;
        }

        _delay_ms(10); // Maintain sampling stability
    }
}

/* --- UART / Communication Functions --- */
void UART_Init(unsigned int ubrr) {
    UCSRA = (1 << U2X); // Double speed for 115200 baud
    UBRRH = (unsigned char)(ubrr >> 8);
    UBRRL = (unsigned char)ubrr;
    UCSRB = (1 << RXEN) | (1 << TXEN);
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

void UART_Transmit(char data) {
    while (!(UCSRA & (1 << UDRE)));
    UDR = data;
}

void UART_SendString(const char* str) {
    while (*str) UART_Transmit(*str++);
}

void UART_SendNumber(u32 num) {
    char buffer[11];
    ltoa(num, buffer, 10);
    UART_SendString(buffer);
}

uint8_t UART_WaitFor(const char* target) {
    uint8_t targetLen = strlen(target);
    uint8_t index = 0;
    uint32_t timeout = 500000;
    while (timeout > 0) {
        if (UCSRA & (1 << RXC)) {
            char c = UDR;
            if (c == target[index]) {
                index++;
                if (index == targetLen) return 1;
            } else {
                index = (c == target[0]) ? 1 : 0;
            }
        }
        timeout--;
    }
    return 0;
}

void UART_Flush(void) {
    unsigned char dummy;
    while (UCSRA & (1 << RXC)) dummy = UDR;
}

void send_to_blynk(u8 bpm) {
    char httpRequest[180];
    char cipSendCmd[30];
    sprintf(httpRequest, "GET /external/api/update?token=%s&v1=%d HTTP/1.0\r\nHost: blynk.cloud\r\n\r\n", BLYNK_AUTH_TOKEN, bpm);

    UART_Flush();
    UART_SendString("AT+CIPSTART=\"TCP\",\"blynk.cloud\",80\r\n");
    if (UART_WaitFor("CONNECT")) {
        sprintf(cipSendCmd, "AT+CIPSEND=%d\r\n", (int)strlen(httpRequest));
        UART_SendString(cipSendCmd);
        if (UART_WaitFor(">")) {
            UART_SendString(httpRequest);
            UART_WaitFor("SEND OK");
        }
    }
    UART_SendString("AT+CIPCLOSE\r\n");
}

/* --- Sensor Driver Functions --- */
void MAX30102_Init(void) {
    I2C_SendStartCond();
    I2C_SendAdd(MAX30102_WRITE_ADDR);
    I2C_SendData(0x09);
    I2C_SendData(0x40); // Reset command
    I2C_SendStopCond();
    _delay_ms(100);

    I2C_SendStartCond();
    I2C_SendAdd(MAX30102_WRITE_ADDR);
    I2C_SendData(0x09);
    I2C_SendData(0x02); // Mode: Heart Rate Only
    I2C_SendStopCond();

    I2C_SendStartCond();
    I2C_SendAdd(MAX30102_WRITE_ADDR);
    I2C_SendData(0x0C);
    I2C_SendData(0x10); // Pulse Amplitude
    I2C_SendStopCond();

    I2C_SendStartCond();
    I2C_SendAdd(MAX30102_WRITE_ADDR);
    I2C_SendData(0x0D);
    I2C_SendData(0x1F); // Pulse Width
    I2C_SendStopCond();
}

u32 MAX30102_ReadIR(void) {
    u32 ir = 0;
    u8 b1, b2, b3;
    I2C_SendStartCond();
    I2C_SendAdd(MAX30102_WRITE_ADDR);
    I2C_SendData(0x07);
    I2C_SendStartCond();
    I2C_SendAdd(MAX30102_READ_ADDR);
    b1 = I2C_MasterReadAck();
    b2 = I2C_MasterReadAck();
    b3 = I2C_MasterReadNack();
    I2C_SendStopCond();
    ir = ((u32)b1 << 16) | ((u32)b2 << 8) | b3;
    return ir & 0x03FFFF;
}

u8 checkForBeat(u32 irValue) {
    static float lowPassValue = 0;
    static float dcBaseline = 0;
    static u8 beatDetected = 0;

    if (lowPassValue == 0) {
        lowPassValue = (float)irValue;
        dcBaseline = (float)irValue;
        return 0;
    }

    lowPassValue = (0.2 * irValue) + (0.8 * lowPassValue);
    dcBaseline = (0.98 * dcBaseline) + (0.02 * lowPassValue);
    float acValue = lowPassValue - dcBaseline;

    if (acValue > 12 && beatDetected == 0) {
        beatDetected = 1;
        return 1;
    } else if (acValue < -2) {
        beatDetected = 0;
    }
    return 0;
}
