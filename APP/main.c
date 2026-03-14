#define F_CPU 16000000UL
#include "../LIB/BIT_MATH.h"
#include "../LIB/STD_TYPES.h"

#include "../HAL/LED/LED.h"
#include "../HAL/LCD/LCD.h"
#include "../HAL/MAX_10502/MAX.h"

#include <util/delay.h>
#include <stdlib.h>
#include "../MCAL/I2C/I2C.h"
#include "../MCAL/DIO/DIO.h"
#include "../MCAL/Timer1/Timer1.h"
#include "../MCAL/USART_Online/USART_Online.h"

u8 rates[RATE_SIZE];
u8 rateSpot = 0;
u32 lastBeat = 0;
float beatsPerMinute;
u8 beatAvg;

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
	// --- 1. Hardware Initializations ---
	LCD_init();
	LCD_clear();
	USART_Init(9600);
	I2C_Masterinit(100000);
	MAX30102_Init();

	Timer1_Init(TIMER1_PRESCALLER64, TIMER1_CTC);
	OCR1A = 249;        // Sets the 1ms interval for 16MHz clock
	SET_BIT(TIMSK, 4);  // Enable Compare Match A interrupt
	SET_BIT(SREG, 7);   // Enable Global Interrupts

	u16 lcdTimer = 0;
	u32 currentIR = 0;

	USART_SendString("System Initialized. Place Finger...\r\n");
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
			USART_SendString("Resetting due to inactivity...\r\n");
		}

		if (checkForBeat(currentIR)) {

			if (lastBeat == 0) {
				lastBeat = now;
			} else {
				u32 delta = now - lastBeat;
				lastBeat = now; // Always update lastBeat to the most recent pulse

				USART_SendString("Delta: ");
				USART_SendNumber(delta);
				USART_SendString(" ms\r\n");
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

					USART_SendString("BPM Detected: ");
					USART_SendNumber(beatAvg);
					USART_SendString("\r\n");
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
