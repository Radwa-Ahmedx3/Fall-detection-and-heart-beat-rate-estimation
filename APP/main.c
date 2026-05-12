#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Libraries */
#include "../LIB/BIT_MATH.h"
#include "../LIB/STD_TYPES.h"
#include "../HAL/LED/LED.h"
#include "../MCAL/DIO/DIO.h"
#include "../MCAL/I2C/I2C.h"
#include "../MCAL/Timer1/Timer1.h"

/* WiFi / Blynk */
#define BAUD 115200
#define MYUBRR 16
#define BLYNK_AUTH_TOKEN "tXaS2EX1wjwMv1SvCIz-si2XfgGck52e"

/* MAX30102 */
#define MAX30102_ADDR        0x57
#define MAX30102_WRITE_ADDR  ((MAX30102_ADDR << 1) | 0)
#define MAX30102_READ_ADDR   ((MAX30102_ADDR << 1) | 1)

/* MPU6050 */
#define MPU6050_ADDR         0x68
#define MPU6050_WRITE_ADDR   ((MPU6050_ADDR << 1) | 0)
#define MPU6050_READ_ADDR    ((MPU6050_ADDR << 1) | 1)

#define RATE_SIZE 10

/* Fall thresholds */
#define FALL_LOW_THRESHOLD      0.35 //was 0.45
#define FALL_HIGH_THRESHOLD     2.5
#define ANGLE_CHANGE_THRESHOLD  1 //was 1.5

//-------------Crystal 16MHz initialization-----------
void setup_interrupt() {
	DDRD &= ~(1 << PD2);
	PORTD |= (1 << PD2);
	MCUCR &= ~((1 << ISC01) | (1 << ISC00));
	GICR |= (1 << INT0);
	sei();
}

void enter_standby_mode() {
	MCUCR |= (1 << SM2) | (1 << SM1) | (1 << SE);
	MCUCR &= ~(1 << SM0);

	asm volatile("sleep");
	MCUCR &= ~(1 << SE);
}

ISR( INT0_vect) {
}

/* Globals */
u8 rates[RATE_SIZE];
u8 rateSpot = 0;

u32 lastBeat = 0;

float beatsPerMinute;
u8 beatAvg = 0;

volatile u32 g_millis = 0;

/* ===== FIX ADDED HERE ===== */
volatile u8 fallDetected = 0;
volatile u8 emergencyLatched = 0;
u16 safeCounter = 0;
/* ========================== */

u32 currentIR = 0;

/* Prototypes */
void UART_Init(unsigned int ubrr);
void UART_Transmit(char data);
void UART_SendString(const char *str);
void UART_SendNumber(u32 num);
uint8_t UART_WaitFor(const char *target);
void UART_Flush(void);

void MAX30102_Init(void);
u32 MAX30102_ReadIR(void);
u8 checkForBeat(u32 irValue);

void MPU6050_Init(void);
void MPU6050_ReadAccel(int16_t *ax, int16_t *ay, int16_t *az);
u8 detectFall(void);
float absolute(float x);

void send_to_blynk(u8 bpm, const char *status);
void send_fall_status(const char *state);

/* Timer ISR */
void __vector_7(void) __attribute__((signal));
void __vector_7(void) {
	g_millis++;
}

u32 millis(void) {
	return g_millis;
}

int main(void) {

	UART_Init(MYUBRR);

	I2C_Masterinit(100000);

	MAX30102_Init();
	MPU6050_Init();

	Timer1_Init(TIMER1_PRESCALLER64, TIMER1_CTC);
	OCR1A = 249;

	SET_BIT(TIMSK, 4);
	SET_BIT(SREG, 7);

	UART_SendString("\r\n--- SYSTEM START ---\r\n");

	/* WiFi Setup */
	UART_Flush();

	UART_SendString("AT\r\n");
	UART_WaitFor("OK");

	UART_SendString("AT+CWMODE=1\r\n");
	UART_WaitFor("OK");

	UART_SendString("AT+CIPMUX=0\r\n");
	UART_WaitFor("OK");

	char wifi_cmd[100];
	sprintf(wifi_cmd, "AT+CWJAP=\"smartmonitor\",\"smartmonitor123456\"\r\n");
	UART_SendString(wifi_cmd);

	if (UART_WaitFor("WIFI GOT IP")) {
		UART_SendString("WiFi Connected\r\n");
	}

	u32 lastBlynkUpdate = 0;
	u32 lastFallCheck = 0;

	u8 high_bpm_alert_sent = 0;

	DDRA |= (1 << DDA0); //max reading BPM
	DDRB |= (1 << DDB0); //reset
	DDRB |= (1 << DDB1); //low heart rate
	DDRB |= (1 << DDB2); //high heart rate
	DDRD |= (1 << DDD5); //buzzer
	DDRD |= (1 << DDD6); //mpu fall detectionggggg
	DDRD |= (1 << DDD7); //normal

	while (1) {

		PORTB |= (1 << PORTB0);

		u32 now = millis();

		/* =========================
		 FALL DETECTION (FIXED)
		 ========================= */
		if (now - lastFallCheck >= 60) {

			if (detectFall()) {

				safeCounter = 0;

				if (!emergencyLatched) {

					emergencyLatched = 1;
					fallDetected = 1;

					PORTD |= (1 << PORTD6);
					PORTD |= (1 << PORTD5);
					_delay_ms(500);
					PORTD &= ~(1 << PORTD5);
					send_fall_status("Emergency");
					_delay_ms(2000);
				}
			} else {

				/* stability counter before reset */
				if (emergencyLatched) {

					safeCounter++;

					if (safeCounter > 120) {
						emergencyLatched = 0;
						fallDetected = 0;
						PORTD &= ~(1 << PORTD6);
						send_fall_status("No%20Emergency");

						UART_SendString("FALL CLEARED\r\n");

						safeCounter = 0;
					}
				}
			}

			lastFallCheck = now;
		}

		/* =========================
		 HEART RATE (UNCHANGED)
		 ========================= */
		currentIR = MAX30102_ReadIR();

		if (currentIR < 50000) {
			beatAvg = 0;
			lastBeat = 0;
			rateSpot = 0;
			for (u8 i = 0; i < RATE_SIZE; i++)
				rates[i] = 0;
		} else {
			// Timeout logic (No beat detected for 3s while finger is on)
			if (now - lastBeat > 3000 && lastBeat != 0) {
				beatAvg = 0;
				lastBeat = 0;
				rateSpot = 0;
				for (u8 i = 0; i < RATE_SIZE; i++)
					rates[i] = 0;

			}

			if (checkForBeat(currentIR)) {
				if (lastBeat == 0) {
					lastBeat = now;
				} else {
					u32 delta = now - lastBeat;
					lastBeat = now;

					if (delta > 400 && delta < 1500) {
						beatsPerMinute = 60000.0 / (float) delta;

						if (beatAvg == 0) {
							for (u8 i = 0; i < RATE_SIZE; i++)
								rates[i] = (u8) beatsPerMinute;
						}

						rates[rateSpot++] = (u8) beatsPerMinute;
						rateSpot %= RATE_SIZE;

						u16 sum = 0;
						for (u8 i = 0; i < RATE_SIZE; i++)
							sum += rates[i];
						beatAvg = (u8) (sum / RATE_SIZE);

					}
				}
			}
		}

		/* =========================
		 BLYNK UPDATE (UNCHANGED)
		 ========================= */
		if (now - lastBlynkUpdate >= 2000) {

			const char *blynkStatus;

			if (currentIR < 50000)
				blynkStatus = "No%20Finger";
			else if (beatAvg == 0)
				blynkStatus = "Processing";
			else if (beatAvg < 60) {
				blynkStatus = "Low";
				PORTB &= ~(1 << PORTB2);

				PORTD &= ~(1 << PORTD7);

				PORTB |= (1 << PORTB1);
			} else if (beatAvg <= 105) {
				blynkStatus = "Normal";
				PORTB &= ~(1 << PORTB1);

				PORTD &= ~(1 << PORTB2);

				PORTD |= (1 << PORTD7);
			} else {
				blynkStatus = "High";
				PORTB &= ~(1 << PORTB1);

				PORTD &= ~(1 << PORTD7);

				PORTB |= (1 << PORTB2);
			}

			send_to_blynk(beatAvg, blynkStatus);

			if (beatAvg > 200 && high_bpm_alert_sent == 0) {
				high_bpm_alert_sent = 1;
			} else if (beatAvg <= 200) {
				high_bpm_alert_sent = 0;
			}

			lastBlynkUpdate = now;
		}
	}
}

/* UART */
void UART_Init(unsigned int ubrr) {

	UCSRA = (1 << U2X);

	UBRRH = (unsigned char) (ubrr >> 8);
	UBRRL = (unsigned char) ubrr;

	UCSRB = (1 << RXEN) | (1 << TXEN);

	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

void UART_Transmit(char data) {

	while (!(UCSRA & (1 << UDRE)))
		;

	UDR = data;
}

void UART_SendString(const char *str) {

	while (*str) {

		UART_Transmit(*str++);
	}
}

void UART_SendNumber(u32 num) {

	char buffer[11];

	ltoa(num, buffer, 10);

	UART_SendString(buffer);
}

uint8_t UART_WaitFor(const char *target) {

	uint8_t targetLen = strlen(target);

	uint8_t index = 0;

	uint32_t timeout = 500000;

	while (timeout > 0) {

		if (UCSRA & (1 << RXC)) {

			char c = UDR;

			if (c == target[index]) {

				index++;

				if (index == targetLen) {

					return 1;
				}
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

	while (UCSRA & (1 << RXC)) {

		dummy = UDR;
	}
}

/* Blynk */
void send_to_blynk(u8 bpm, const char *status) {

	char httpRequest[250];
	char cipSendCmd[30];

	const char *emergencyStatus;

	if (fallDetected) {

		emergencyStatus = "Emergency";
	} else {

		emergencyStatus = "No%20Emergency";
	}

	sprintf(httpRequest,
			"GET /external/api/batch/update?token=%s&v1=%d&v0=%s&v2=%s HTTP/1.0\r\nHost: blynk.cloud\r\n\r\n",
			BLYNK_AUTH_TOKEN, bpm, status, emergencyStatus);

	UART_Flush();

	UART_SendString("AT+CIPSTART=\"TCP\",\"blynk.cloud\",80\r\n");

	if (UART_WaitFor("CONNECT")) {

		sprintf(cipSendCmd, "AT+CIPSEND=%d\r\n", (int) strlen(httpRequest));

		UART_SendString(cipSendCmd);

		if (UART_WaitFor(">")) {

			UART_SendString(httpRequest);

			UART_WaitFor("SEND OK");
		}
	}

	UART_SendString("AT+CIPCLOSE\r\n");
}
//void trigger_blynk_event(const char *event_code) {
//
//	char httpRequest[150];
//	char cipSendCmd[30];
//
//	sprintf(httpRequest,
//			"GET /external/api/logEvent?token=%s&code=%s HTTP/1.0\r\nHost: blynk.cloud\r\n\r\n",
//			BLYNK_AUTH_TOKEN, event_code);
//
//	UART_Flush();
//
//	UART_SendString("AT+CIPSTART=\"TCP\",\"blynk.cloud\",80\r\n");
//
//	if (UART_WaitFor("CONNECT")) {
//
//		sprintf(cipSendCmd, "AT+CIPSEND=%d\r\n", (int) strlen(httpRequest));
//
//		UART_SendString(cipSendCmd);
//
//		if (UART_WaitFor(">")) {
//
//			UART_SendString(httpRequest);
//
//			UART_WaitFor("SEND OK");
//		}
//	}
//
//	UART_SendString("AT+CIPCLOSE\r\n");
//}

void send_fall_status(const char *state) {

	char httpRequest[200];
	char cipSendCmd[30];

	sprintf(httpRequest,
			"GET /external/api/update?token=%s&v2=%s HTTP/1.0\r\nHost: blynk.cloud\r\n\r\n",
			BLYNK_AUTH_TOKEN, state);

	UART_Flush();

	UART_SendString("AT+CIPSTART=\"TCP\",\"blynk.cloud\",80\r\n");

	if (UART_WaitFor("CONNECT")) {

		sprintf(cipSendCmd, "AT+CIPSEND=%d\r\n", (int) strlen(httpRequest));

		UART_SendString(cipSendCmd);

		if (UART_WaitFor(">")) {

			UART_SendString(httpRequest);
			UART_WaitFor("SEND OK");
		}
	}

	UART_SendString("AT+CIPCLOSE\r\n");
}

/* MAX30102 */
void MAX30102_Init(void) {

	I2C_SendStartCond();
	I2C_SendAdd(MAX30102_WRITE_ADDR);
	I2C_SendData(0x09);
	I2C_SendData(0x40);
	I2C_SendStopCond();

	_delay_ms(100);

	I2C_SendStartCond();
	I2C_SendAdd(MAX30102_WRITE_ADDR);
	I2C_SendData(0x09);
	I2C_SendData(0x02);
	I2C_SendStopCond();

	I2C_SendStartCond();
	I2C_SendAdd(MAX30102_WRITE_ADDR);
	I2C_SendData(0x0C);
	I2C_SendData(0x10);
	I2C_SendStopCond();

	I2C_SendStartCond();
	I2C_SendAdd(MAX30102_WRITE_ADDR);
	I2C_SendData(0x0D);
	I2C_SendData(0x1F);
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

	ir = ((u32) b1 << 16) | ((u32) b2 << 8) | b3;

	return ir & 0x03FFFF;
}

/* Beat Detection */
u8 checkForBeat(u32 irValue) {

	static float lowPassValue = 0;
	static float dcBaseline = 0;
	static u8 beatDetected = 0;

	if (lowPassValue == 0) {

		lowPassValue = irValue;
		dcBaseline = irValue;

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

/* MPU6050 */
void MPU6050_Init(void) {

	I2C_SendStartCond();

	I2C_SendAdd(MPU6050_WRITE_ADDR);

	I2C_SendData(0x6B);

	I2C_SendData(0x00);

	I2C_SendStopCond();

	_delay_ms(100);
}

void MPU6050_ReadAccel(int16_t *ax, int16_t *ay, int16_t *az) {

	u8 data[6];

	I2C_SendStartCond();

	I2C_SendAdd(MPU6050_WRITE_ADDR);

	I2C_SendData(0x3B);

	I2C_SendStartCond();

	I2C_SendAdd(MPU6050_READ_ADDR);

	for (u8 i = 0; i < 5; i++) {

		data[i] = I2C_MasterReadAck();
	}

	data[5] = I2C_MasterReadNack();

	I2C_SendStopCond();

	*ax = (int16_t) ((data[0] << 8) | data[1]);

	*ay = (int16_t) ((data[2] << 8) | data[3]);

	*az = (int16_t) ((data[4] << 8) | data[5]);
}

float absolute(float x) {

	if (x < 0)
		return -x;

	return x;
}

u8 detectFall(void) {

	int16_t ax, ay, az;

	float Ax, Ay, Az;

	float totalAccel;

	static u8 trigger1 = 0;
	static u8 trigger2 = 0;

	MPU6050_ReadAccel(&ax, &ay, &az);

	Ax = ax / 16384.0;
	Ay = ay / 16384.0;
	Az = az / 16384.0;

	totalAccel = sqrt((Ax * Ax) + (Ay * Ay) + (Az * Az));

	/* Free fall */
	if (totalAccel < FALL_LOW_THRESHOLD) {

		trigger1 = 1;
	}

	/* Impact */
	if (trigger1 && totalAccel > FALL_HIGH_THRESHOLD) {

		trigger1 = 0;
		trigger2 = 1;
	}

	/* Orientation change */
	if (trigger2) {

		float angleChange;

		angleChange = absolute(Ax) + absolute(Ay);

		if (angleChange >
		ANGLE_CHANGE_THRESHOLD) {

			trigger2 = 0;

			return 1;
		}
	}

	return 0;
}
