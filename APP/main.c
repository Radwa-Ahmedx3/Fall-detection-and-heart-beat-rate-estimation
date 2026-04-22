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
	DDRD &= ~(1<<PD2);
	PORTD |= (1 << PD2);
    MCUCR &= ~((1 << ISC01) | (1 << ISC00)); //low level interrupt
    GICR  |= (1 << INT0);
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

	u8 fall_step = 0;
	u16 stability_counter = 0;

//	----------------INITIALIZATION----------------
	USART_Init(9600);
	DIO_InitPin(DIO_PORTC, DIO_PIN5, DIO_OUTPUT);

	LED_init(DIO_PORTC, DIO_PIN4);

	LCD_init();

	// I2C and MPU6050 at address 0x68 (see MPU_6050.h)
	I2C_Masterinit(100000);
	MPU6050_Online_Init();
	///////////MAX30102_Init();

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

		// PHASE 1: WAIT FOR USER TAP
		USART_SendString("System Standby: Fall detection to start...\r\n");

		u8 fall_detected = 0;

		static u8  fall_step = 0;
		static u16 stability_counter = 0;
		static u16 fall_timer = 0;
//=================================MPU while loop=================================
		while (fall_detected == 0) {
			Read_Accel(&Ax, &Ay, &Az);

			USART_SendString("Ax: ");
			USART_SendFloat_chato(Ax, 2); // Shows 2 decimal places, e.g., 0.98
			USART_SendString(" | Ay: ");
			USART_SendFloat_chato(Ay, 2);
			USART_SendString(" | Az: ");
			USART_SendFloat_chato(Az, 2);
			//USART_SendString("\r\n");

			Read_Gyro(&Gx, &Gy, &Gz);

			USART_SendString("---Gx: ");
			USART_SendFloat_chato(Gx, 1); // Gyro usually needs fewer decimals
			USART_SendString(" | Gy: ");
			USART_SendFloat_chato(Gy, 2);
			USART_SendString(" | Gz: ");
			USART_SendFloat_chato(Gz, 2);
			USART_SendString("\r\n");

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

////			------------------------- Algorithm -----------------------------
//

			_delay_ms(20); // sampling rate عالي (50 قراءة في الثانية)

			    // حساب القيمة الكلية للعجلة
			    f32 Acc_Total = sqrt((Ax * Ax) + (Ay * Ay) + (Az * Az));

			    switch (fall_step) {
			        case 0: // المرحلة 0: مراقبة بداية السقوط (Free Fall)
			            // رفعنا الحساسية لـ 0.8f لأن السقوط القصير لا يصل للصفر
			            if (Acc_Total < 0.82f) {
			                fall_step = 1;
			                fall_timer = 0;
			                USART_SendString(">> 1. Falling Started...\r\n");
			            }
			            break;

			        case 1: // المرحلة 1: انتظار الارتطام أو المسك (Impact/Catch)
			            fall_timer++;

			            // إذا زادت القيمة عن 1.25g (قيمة المسك بالإيد عادة ما تكون ضعيفة)
			            if (Acc_Total > 1.25f) {
			                fall_step = 2;
			                stability_counter = 0;
			                USART_SendString(">> 2. Catch/Impact Detected!\r\n");
			            }

			            // Timeout: لو فضل "طاير" أكتر من ثانية ونصف بدون اصطدام (غالباً نويز)
			            if (fall_timer > 75) {
			                fall_step = 0;
			                USART_SendString(">> Reset: No Impact within time window.\r\n");
			            }
			            break;

			        case 2: // المرحلة 2: التأكد من الاستقرار (Hand Stability)
			            // بما أن اليد تهتز، وسعنا النطاق (بين 0.6 و 1.4)
			            if (Acc_Total > 0.6f && Acc_Total < 1.4f) {
			                stability_counter++;

			                // 8 عدات (حوالي 160ms) ثبات في اليد كافية للتأكيد
			                if (stability_counter >= 8) {
			                    USART_SendString("\r\n =============================== \r\n");
			                    USART_SendString(" !!! ALARM: FALL CONFIRMED !!! ");
			                    USART_SendString("\r\n =============================== \r\n");

			                    // تفعيل الإنذار
			                    DDRD |= (1 << DDD6);
			                    PORTD |= (1 << PORTD6);


			                    // ريست لكل شيء للوقعة القادمة
			                    fall_step = 0;
			                    stability_counter = 0;
			                }
			            } else {
			                // لو حصلت خبطة تانية أو حركة عنيفة، بنصفر العداد ونستنى يثبت
			                stability_counter = 0;
			            }

			            // أمان إضافي: لو قعد أكتر من 3 ثواني بيتهز ومش راضي يثبت
			            fall_timer++;
			            if (fall_timer > 150) {
			                fall_step = 0;
			                stability_counter = 0;
			                USART_SendString(">> Reset: Unstable for too long.\r\n");
			            }
			            break;
			    }
			}
	}
}
//			f32 Acc_Total = sqrt((Ax * Ax) + (Ay * Ay) + (Az * Az));
//
//			if (Acc_Total < 0.5f && fall_step == 0) {
//				fall_step = 1;
//				USART_SendString(
//						"\r\n===================Free Fall Detected!===============\r\n");
//			}
//
//			if (fall_step == 1 && Acc_Total > 1.5f) {
//				fall_step = 2;
//				USART_SendString(
//						"\r\n===================Impact Detected!===================\r\n");
//				_delay_ms(500);
//			}
//
//			if (fall_step == 2) {
//
//				if (Acc_Total > 0.8f && Acc_Total < 1.2f) {
//					stability_counter++;
//					if (stability_counter >= 10) {
//						USART_SendString(
//								"\r\n===================ALARM: FALL CONFIRMED!===================\r\n");
//						fall_detected = 1;
//
//						DDRD |= (1 << DDD6); //Buzzer On
//				        PORTD |= (1 << PORTD6);
//					    _delay_ms(20);
//
//						fall_step = 0;
//						stability_counter = 0;
//					}
//				} else {
//					fall_step = 0;
//					stability_counter = 0;
//				}
//			}
//
//			_delay_ms(100);
//		}
//	}
//		return 0;
//	}

////			------------------------- Tap Detection Algorithm -----------------------------
//
//			// Calculate the total acceleration magnitude
//			// Normally, this is ~1.0 when sitting still
//			f32 Acc_Total = sqrt((Ax * Ax) + (Ay * Ay) + (Az * Az));
//
//			// THRESHOLD: 1.5f to 2.0f is usually a good range for a table tap.
//			// If it's too sensitive, increase to 2.5f.
//			if (Acc_Total > 1.2f || Az < 0.6) {
//
//			    // 1. Send Debug message
//			    USART_SendString("\r\n !!! TAP DETECTED !!! \r\n");
//			    USART_SendString("Magnitude: ");
//			    USART_SendFloat_chato(Acc_Total, 2);
//			    USART_SendString("\r\n");
//			    fall_detected = 1;
//
//			    // 2. Visual/Audio Feedback
//
//
//			    DDRD |= (1 << DDD6); //Buzzer On
//			    PORTD |= (1 << PORTD6);
//			    _delay_ms(100);
//
//
//			    // 3. Keep the alarm on long enough to see/hear it
//			    //_delay_ms(1000);
//
//			    // 4. Reset indicator
//			    //LED_OFF(DIO_PORTC, DIO_PIN2);                  // LED OFF (if your driver has LED_OFF)
//			}
//
//			// Short delay to prevent one physical tap from triggering
//			 //  the 'if' statement multiple times (Debouncing).
//
//			_delay_ms(50);
//		}
//	}
//
//
//		return 0;
//	}
//

//=================================MAX while loop=================================
//		while (1) {
//
//			USART_SendString("Reading Heartbeat...\r\n");
//
//			USART_SendString("System Initialized. Place Finger...\r\n");
//			while (1) {
//				currentIR = MAX30102_ReadIR();
//				u32 now = millis();
//
//				// If no beat has been detected for 3 seconds, reset the data
//				if (now - lastBeat > 3000 && lastBeat != 0) {
//					beatAvg = 0;
//					lastBeat = 0;
//					rateSpot = 0;
//					for (u8 i = 0; i < RATE_SIZE; i++)
//						rates[i] = 0;
//					USART_SendString("Resetting due to inactivity...\r\n");
//				}
//
//				if (checkForBeat(currentIR)) {
//
//					if (lastBeat == 0) {
//						lastBeat = now;
//					} else {
//						u32 delta = now - lastBeat;
//						lastBeat = now; // Always update lastBeat to the most recent pulse
//
//						USART_SendString("Delta: ");
//						USART_SendNumber(delta);
//						USART_SendString(" ms\r\n");
//						if (delta > 400 && delta < 1500) {
//							beatsPerMinute = 60000.0 / (float) delta;
//
//							if (beatAvg == 0) {
//								for (u8 i = 0; i < RATE_SIZE; i++)
//									rates[i] = (u8) beatsPerMinute;
//							}
//
//							rates[rateSpot++] = (u8) beatsPerMinute;
//							rateSpot %= RATE_SIZE;
//
//							u16 sum = 0;
//							for (u8 i = 0; i < RATE_SIZE; i++)
//								sum += rates[i];
//							beatAvg = (u8) (sum / RATE_SIZE);
//
//							USART_SendString("BPM Detected: ");
//							USART_SendNumber(beatAvg);
//							USART_SendString("\r\n");
//						}
//					}
//				}
//
//				// 3. Optimized Display (Every 250ms instead of 500ms)
//				if (++lcdTimer >= 25) {
//					update_display(beatAvg, currentIR);
//					lcdTimer = 0;
//				}
//
//				_delay_ms(10);
//			}
//		}
//
//		_delay_ms(1000); // Placeholder for heartbeat sampling
//	}
//
//	return 0;
//}




/*

#define F_CPU 16000000UL // Set clock speed (1MHz is default for ATmega32)
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    // 1. Set PA0 as an output
    // DDRA is the Data Direction Register for Port A
    DDRB |= (1 << DDB0);

    while (1) {
        // 2. Set PA0 HIGH (Turn LED ON)
        PORTB |= (1 << PORTB0);
        _delay_ms(50); // Wait 500ms

        // 3. Set PA0 LOW (Turn LED OFF)
        PORTB &= ~(1 << PORTB0);
        _delay_ms(50); // Wait 500ms
    }

    return 0; // Should never reach here
}
*/
