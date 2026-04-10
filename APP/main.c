#define F_CPU 16000000UL
//comment
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

//-------------Crystal 16MHz initialization-----------
void setup_interrupt() {
	DDRD &= ~(1 << PD2);
	PORTD |= (1 << PD2);
	MCUCR &= ~((1 << ISC01) | (1 << ISC00)); //low level interrupt
	GICR |= (1 << INT0);
	sei(); //global interrupt
}

void enter_standby_mode() {
	MCUCR |= (1 << SM2) | (1 << SM1) | (1 << SE);
	MCUCR &= ~(1 << SM0);

	asm volatile("sleep");
	MCUCR &= ~(1 << SE);
}

ISR(INT0_vect) {

}
//--------------------------------------------------------

uint8_t interrupt_toggle = 0;

#define FREE_FALL_THRESHOLD 0.5f
#define IMPACT_THRESHOLD 2.5f
#define STABILITY_MIN 0.8f
#define STABILITY_MAX 1.2f
#define STABILITY_COUNT 20

//-------------Max initialization-------
u8 rates[RATE_SIZE];
u8 rateSpot = 0;
u32 lastBeat = 0;
float beatsPerMinute;
u8 beatAvg;

//--------------Max functions----------
volatile u32 g_millis = 0;
void Millis_Increment(void) {
	g_millis++;
}

u32 millis(void) {
	return g_millis;
}

// This stays at the top level, outside any functions
void __vector_7(void) __attribute__((signal));
void __vector_7(void) {
	g_millis++; // Direct 1ms increment
}

int main(void) {
	//Local variables for MPU data (pass addresses to driver)
	f32 Ax, Ay, Az, Gx, Gy, Gz;

	DDRC &= ~((1 << PC0) | (1 << PC1));
	PORTC &= ~((1 << PC0) | (1 << PC1));

	u8 fall_step = 0;
	u16 stability_counter = 0;

//	----------------INITIALIZATION----------------
	DIO_InitPin(DIO_PORTC, DIO_PIN5, DIO_OUTPUT);

	LED_init(DIO_PORTC, DIO_PIN4);

	LCD_init();

	// I2C and MPU6050 at address 0x68 (see MPU_6050.h)
	I2C_Masterinit(100000);
	MPU6050_Online_Init();
	MAX30102_Init();

	// Enable global interrupts if needed
	SREG |= (1 << 7);

	// Buzzer off
	DIO_SetPinValue(DIO_PORTC, DIO_PIN5, DIO_LOW);

	//-------MAX
	Timer1_Init(TIMER1_PRESCALLER64, TIMER1_CTC);
	OCR1A = 249;        // Sets the 1ms interval for 16MHz clock
	SET_BIT(TIMSK, 4);  // Enable Compare Match A interrupt
	SET_BIT(SREG, 7);   // Enable Global Interrupts >>>>>>>>>>>>>>>

	u16 lcdTimer = 0;
	u32 currentIR = 0;

	while (1) {

		DDRB |= (1 << DDB0); //toggle led
		// 2. Set PA0 HIGH (Turn LED ON)
		PORTB |= (1 << PORTB0);
		_delay_ms(50); // Wait 500ms

		// 3. Set PA0 LOW (Turn LED OFF)
		PORTB &= ~(1 << PORTB0);
		_delay_ms(50); // Wait 500ms

		u8 fall_detected = 0;
//=================================MPU while loop=================================
		while (fall_detected == 0) {
			Read_Accel(&Ax, &Ay, &Az);

			Read_Gyro(&Gx, &Gy, &Gz);

			LCD_movecursor(0, 1);
			LCD_print_3_digit((u16) (Ax * 100));
			LCD_movecursor(0, 5);
			LCD_print_3_digit((u16) (Ay * 100));
			LCD_movecursor(0, 9);
			LCD_print_3_digit((u16) (Az * 100));

			LCD_movecursor(1, 1);
			LCD_print_3_digit((u16) (Gx * 100));
			LCD_movecursor(1, 5);
			LCD_print_3_digit((u16) (Gy * 100));
			LCD_movecursor(1, 9);
			LCD_print_3_digit((u16) (Gz * 100));

/*//			------------------------- Algorithm -----------------------------

			f32 Acc_Total = sqrt((Ax * Ax) + (Ay * Ay) + (Az * Az));



			if (Acc_Total < 0.4f && fall_step == 0) {
				fall_step = 1;

				DDRD |= (1 << DDD6); //Buzzer On
				PORTD |= (1 << PORTD6);
				_delay_ms(100);
				PORTD &= ~(1 << PORTD6);
				_delay_ms(100);
			}

				if (fall_step == 1 && Acc_Total > 2.5f) {
					fall_step = 2;

					_delay_ms(500);
				}

				if (fall_step == 2) {

					if (Acc_Total > 0.8f && Acc_Total < 1.2f) {
						stability_counter++;
						if (stability_counter >= 20) {

							fall_detected = 1;

							fall_step = 0;
							stability_counter = 0;
						}
					} else {
						fall_step = 0;
						stability_counter = 0;
					}
				}

				_delay_ms(100);
			}*/


//			------------------------- Tap Detection Algorithm -----------------------------

			// Calculate the total acceleration magnitude
			// Normally, this is ~1.0 when sitting still
			f32 Acc_Total = sqrt((Ax * Ax) + (Ay * Ay) + (Az * Az));
		    DDRD |= (1 << DDD6); //Buzzer On
		    PORTD |= (1 << PORTD6);
		    _delay_ms(100);
		    PORTD &= ~(1 << PORTD6);
		    _delay_ms(100);

			// THRESHOLD: 1.5f to 2.0f is usually a good range for a table tap.
			// If it's too sensitive, increase to 2.5f.
			if (Acc_Total > 1.2f) {

			    // 1. Send Debug message
			    USART_SendString("\r\n !!! TAP DETECTED !!! \r\n");
			    USART_SendString("Magnitude: ");
			    USART_SendFloat_chato(Acc_Total, 2);
			    USART_SendString("\r\n");
			    fall_detected = 1;

			    // 2. Visual/Audio Feedback
			    DIO_SetPinValue(DIO_PORTC, DIO_PIN5, DIO_HIGH); // Buzzer ON




			    // 3. Keep the alarm on long enough to see/hear it
			    //_delay_ms(1000);

			    // 4. Reset indicators
			    DIO_SetPinValue(DIO_PORTC, DIO_PIN5, DIO_LOW);  // Buzzer OFF
			    //LED_OFF(DIO_PORTC, DIO_PIN2);                  // LED OFF (if your driver has LED_OFF)
			}

			// Short delay to prevent one physical tap from triggering
			 //  the 'if' statement multiple times (Debouncing).

			_delay_ms(50);
		}
//=================================MAX while loop=================================
			while (1) {
				PORTD |= (1 << PORTD7);
				_delay_ms(1000);
				PORTD &= ~(1 << PORTD7);
				_delay_ms(100);

				while (1) {
					currentIR = MAX30102_ReadIR();
					u32 now = millis();

					// If no beat has been detected for 3 seconds, reset the data
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
							lastBeat = now; // Always update lastBeat to the most recent pulse

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

					// 3. Optimized Display (Every 250ms instead of 500ms)
					if (++lcdTimer >= 25) {
						update_display(beatAvg, currentIR);
						lcdTimer = 0;
					}

					_delay_ms(10);
				}
			}

			_delay_ms(1000); // Placeholder for heartbeat sampling
		}

		return 0;
	}
