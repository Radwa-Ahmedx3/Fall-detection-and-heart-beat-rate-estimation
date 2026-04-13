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

// --- Configuration Constants ---
#define FINGER_IR_THRESHOLD 50000UL
#define MAX_SESSION_TIMEOUT_MS 30000UL
#define NO_FINGER_TIMEOUT_MS 8000UL
#define VALID_READING_HOLD_MS 3000UL
#define LONG_STABILITY_COUNT 300

// --- Global Variables ---
volatile u32 g_millis = 0;
volatile u8 g_max_trigger_flag = 0; // The "Bridge" between ISR and Main

u8 rates[RATE_SIZE];
u8 rateSpot = 0;
u32 lastBeat = 0;
float beatsPerMinute;
u8 beatAvg;

// --- Interrupt Logic ---
void setup_interrupt() {
    DDRD &= ~(1 << PD2); // PD2 as input
    PORTD |= (1 << PD2); // Internal pull-up
    MCUCR &= ~((1 << ISC01) | (1 << ISC00)); // Low level trigger
    GICR |= (1 << INT0); // Enable INT0
}

ISR(INT0_vect) {
    g_max_trigger_flag = 1; // Signal the main loop to switch
}

ISR(TIMER1_COMPA_vect) {
    g_millis++;
}

u32 millis(void) {
    u32 m;
    cli(); m = g_millis; sei();
    return m;
}

// --- MAIN CODE ---
int main(void) {
    // Local variables
    f32 Ax, Ay, Az, Gx, Gy, Gz;
    u8 fall_step = 0;
    u16 stability_counter = 0;
    u16 lcdTimer = 0;
    u32 currentIR = 0;

    // 1. Initial Hardware Setup
    I2C_Masterinit(100000);
    USART_Init(9600);
    LCD_init();
    MPU6050_Online_Init(); // Only init MPU at start
    setup_interrupt();

    Timer1_Init(TIMER1_PRESCALLER64, TIMER1_CTC);
    OCR1A = 249;
    SET_BIT(TIMSK, 4); // Enable Timer Interrupt
    sei();             // Enable Global Interrupts

    DIO_InitPin(DIO_PORTC, DIO_PIN5, DIO_OUTPUT); // Buzzer
    DDRB |= (1 << DDB0); // Indicator LED

    while (1) {
        u8 fall_detected = 0;
        u16 long_stability_counter = 0;

        // ================= MPU MONITORING MODE =================
        while (fall_detected == 0) {
            // Check if the ISR has requested the MAX sensor
            if (g_max_trigger_flag == 1) {
                g_max_trigger_flag = 0;
                fall_detected = 1; // Exit MPU loop
                break;
            }

            Read_Accel(&Ax, &Ay, &Az);
            Read_Gyro(&Gx, &Gy, &Gz);

            // Display MPU Data
            LCD_movecursor(0, 0);
            LCD_print_3_digit((u16)(Ax * 100));
            LCD_movecursor(1, 0);
            LCD_print_3_digit((u16)(Gx * 100));

            f32 Acc_Total = sqrt((Ax * Ax) + (Ay * Ay) + (Az * Az));
            f32 Gyro_Total = sqrt((Gx * Gx) + (Gy * Gy) + (Gz * Gz));

            // Standard Fall Detection Logic
            if (Acc_Total < 0.4f && fall_step == 0) {
                fall_step = 1;
                PORTD |= (1 << PORTD6); _delay_ms(100); PORTD &= ~(1 << PORTD6);
            }
            if (fall_step == 1 && Acc_Total > 2.5f) {
                fall_step = 2;
                _delay_ms(500);
            }
            if (fall_step == 2) {
                if (Acc_Total > 0.8f && Acc_Total < 1.2f) {
                    stability_counter++;
                    if (stability_counter >= 20) { fall_detected = 1; }
                } else { fall_step = 0; stability_counter = 0; }
            }

            // Long Stability detection (The "Sleep" trigger)
            if (fall_step == 0) {
                if (Acc_Total > 0.95f && Acc_Total < 1.05f && Gyro_Total < 5.0f) {
                    long_stability_counter++;
                    if (long_stability_counter >= LONG_STABILITY_COUNT) {
                        fall_detected = 1;
                    }
                } else { long_stability_counter = 0; }
            }
            _delay_ms(50);
        }

        // ================= MAX VITALS MODE =================
        // Initialize the MAX30102 only when we enter this mode
        MAX30102_Init();
        LCD_clear();
        LCD_writestr("Checking Vitals");

        u32 session_start = millis();
        u32 last_finger_seen = millis();
        u32 valid_since = 0;
        u8 session_done = 0;

        while (!session_done) {
            currentIR = MAX30102_ReadIR();
            u32 now = millis();

            if (currentIR > FINGER_IR_THRESHOLD) last_finger_seen = now;

            if (checkForBeat(currentIR)) {
                if (lastBeat != 0) {
                    u32 delta = now - lastBeat;
                    if (delta > 400 && delta < 1500) {
                        beatsPerMinute = 60000.0 / (float)delta;
                        rates[rateSpot++] = (u8)beatsPerMinute;
                        rateSpot %= RATE_SIZE;
                        u16 sum = 0;
                        for (u8 i = 0; i < RATE_SIZE; i++) sum += rates[i];
                        beatAvg = (u8)(sum / RATE_SIZE);
                    }
                }
                lastBeat = now;
            }

            if (++lcdTimer >= 20) {
                update_display(beatAvg, currentIR);
                lcdTimer = 0;
            }

            // Exit Criteria
            if (beatAvg >= 45 && beatAvg <= 150) {
                if (valid_since == 0) valid_since = now;
                else if (now - valid_since >= VALID_READING_HOLD_MS) session_done = 1;
            }
            if (now - last_finger_seen >= NO_FINGER_TIMEOUT_MS) session_done = 1;
            if (now - session_start >= MAX_SESSION_TIMEOUT_MS) session_done = 1;

            _delay_ms(10);
        }

        // Reset all states before returning to MPU mode
        fall_step = 0;
        stability_counter = 0;
        beatAvg = 0;
        lastBeat = 0;
        LCD_clear();
        USART_SendString("Returning to MPU Mode\r\n");
    }
    return 0;
}
