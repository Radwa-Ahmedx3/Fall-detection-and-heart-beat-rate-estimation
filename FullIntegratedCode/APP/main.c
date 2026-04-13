#define F_CPU 16000000UL
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <math.h>
#include <avr/io.h>

#include "../LIB/BIT_MATH.h"
#include "../LIB/STD_TYPES.h"
#include "../HAL/LED/LED.h"
#include "../HAL/LCD/LCD.h"
#include "../HAL/MAX_10502/MAX.h"
#include "../HAL/MPU_Online/MPU_Online.h"
#include "../MCAL/I2C/I2C.h"
#include "../MCAL/DIO/DIO.h"
#include "../MCAL/Timer1/Timer1.h"
#include "../MCAL/USART_Online/USART_Online.h"

// --- Global Variables ---
volatile u32 g_millis = 0;
u8 rates[RATE_SIZE];
u8 rateSpot = 0;
u32 lastBeat = 0;
float beatsPerMinute;
u8 beatAvg;

// --- Timer/Millis Functions ---
ISR(TIMER1_COMPA_vect) {
    g_millis++;
}

u32 millis(void) {
    u32 m;
    cli(); // Disable interrupts to read 32-bit value safely
    m = g_millis;
    sei();
    return m;
}

// --- Setup Functions ---
void system_init() {
    // Communication & Sensors
    I2C_Masterinit(100000);
    USART_Init(9600);
    MPU6050_Online_Init();
    MAX30102_Init();
    LCD_init();

    // Peripheral Pins
    DIO_InitPin(DIO_PORTC, DIO_PIN5, DIO_OUTPUT); // Buzzer
    DDRB |= (1 << DDB0);                          // Debug LED
    DDRD |= (1 << DDD6);                          // Secondary Buzzer/LED

    // Timer1 for Millis (1ms at 16MHz)
    Timer1_Init(TIMER1_PRESCALLER64, TIMER1_CTC);
    OCR1A = 249;
    SET_BIT(TIMSK, OCIE1A);

    sei(); // Enable Global Interrupts
}

int main(void) {
    system_init();

    // Data Variables
    f32 Ax, Ay, Az, Gx, Gy, Gz;
    u16 displayUpdateCounter = 0;
    u32 currentIR = 0;

    while (1) {
        // --- 1. MPU6050 Data Handling ---
        Read_Accel(&Ax, &Ay, &Az);
        Read_Gyro(&Gx, &Gy, &Gz);

        f32 Acc_Total = sqrt((Ax * Ax) + (Ay * Ay) + (Az * Az));

        // Tap Detection (Non-blocking)
        if (Acc_Total > 1.5f) {
            USART_SendString("!!! MOTION DETECTED !!!\r\n");
            DIO_SetPinValue(DIO_PORTC, DIO_PIN5, DIO_HIGH);
            _delay_ms(50); // Minimal delay for physical feedback
            DIO_SetPinValue(DIO_PORTC, DIO_PIN5, DIO_LOW);
        }

        // --- 2. MAX30102 Data Handling ---
        currentIR = MAX30102_ReadIR();
        u32 now = millis();

        // Heartbeat Logic
        if (checkForBeat(currentIR)) {
            u32 delta = now - lastBeat;
            lastBeat = now;

            if (delta > 400 && delta < 1500) {
                beatsPerMinute = 60000.0 / (float)delta;

                rates[rateSpot++] = (u8)beatsPerMinute;
                rateSpot %= RATE_SIZE;

                u16 sum = 0;
                for (u8 i = 0; i < RATE_SIZE; i++) sum += rates[i];
                beatAvg = (u8)(sum / RATE_SIZE);
            }
        }

        // Reset if finger is removed (3 seconds timeout)
        if (now - lastBeat > 3000 && lastBeat != 0) {
            beatAvg = 0;
            lastBeat = 0;
            for (u8 i = 0; i < RATE_SIZE; i++) rates[i] = 0;
        }

        // --- 3. Non-Blocking Display Refresh ---
        // Refreshing the LCD every loop is too slow for I2C.
        // We refresh every ~100ms instead.
        displayUpdateCounter++;
        if (displayUpdateCounter >= 10) {
            // Row 0: Accel Data
            LCD_movecursor(0, 0);
            LCD_print_3_digit((u16)(Ax * 100));
            LCD_writestr(' ');
            LCD_print_3_digit((u16)(Ay * 100));
            LCD_writestr(' ');
            LCD_print_3_digit((u16)(Az * 100));

            // Row 1: Heart Rate
            LCD_movecursor(1, 0);
            LCD_writestr("BPM: ");
            if (beatAvg > 30) {
                LCD_print_3_digit(beatAvg);
            } else {
            	LCD_writestr("---");
            }

            displayUpdateCounter = 0;
        }

        // Small loop delay to stabilize I2C traffic
        _delay_ms(10);

        // Blink internal LED to show code is alive
        if (now % 500 < 10) {
            PORTB ^= (1 << PORTB0);
        }
    }

    return 0;
}
