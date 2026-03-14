#define F_CPU 16000000UL

#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <math.h>

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

uint8_t interrupt_toggle = 0;

int main(void) {
	/* Local variables for MPU data (pass addresses to driver) */
	f32 Ax, Ay, Az, Gx, Gy, Gz;

	u8 fall_step = 0;
	u16 stability_counter = 0;

	/*----------------INITIALIZATION----------------*/
	USART_Init(9600);
	DIO_InitPin(DIO_PORTC, DIO_PIN5, DIO_OUTPUT);

	LED_init(DIO_PORTC, DIO_PIN2);

	LCD_init();

	/* I2C and MPU6050 at address 0x68 (see MPU_6050.h) */
	I2C_Masterinit(100000);
	MPU6050_Online_Init();

	/* Enable global interrupts if needed */
	SREG |= (1 << 7);

	/* Buzzer off */
	DIO_SetPinValue(DIO_PORTC, DIO_PIN5, DIO_LOW);

	while (1) {
		/*----------------------- Serial monitor -----------------------------*/
		Read_Accel(&Ax, &Ay, &Az);

		USART_SendString("Ax: ");
		USART_SendFloat_chato(Ax, 2); // Shows 2 decimal places, e.g., 0.98
		USART_SendString(" | Ay: ");
		USART_SendFloat_chato(Ay, 2);
		USART_SendString(" | Az: ");
		USART_SendFloat_chato(Az, 2);
		USART_SendString("\r\n");

		Read_Gyro(&Gx, &Gy, &Gz);

		USART_SendString("Gx: ");
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

		/*------------------------- Algorithm -----------------------------*/

		Read_Accel(&Ax, &Ay, &Az);
		Read_Gyro(&Gx, &Gy, &Gz);

		f32 Acc_Total = sqrt((Ax * Ax) + (Ay * Ay) + (Az * Az));

		if (Acc_Total < 0.4f && fall_step == 0) {
			fall_step = 1;
			USART_SendString("Free Fall Detected!\r\n");
		}


		if (fall_step == 1 && Acc_Total > 2.5f) {
			fall_step = 2;
			USART_SendString("Impact Detected!\r\n");
			_delay_ms(500);
		}


		if (fall_step == 2) {

			if (Acc_Total > 0.8f && Acc_Total < 1.2f) {
				stability_counter++;
				if (stability_counter >= 20) {
					USART_SendString("ALARM: FALL CONFIRMED!\r\n");


					DIO_SetPinValue(DIO_PORTC, DIO_PIN5, DIO_HIGH); // Buzzer
					LED_ON(DIO_PORTC, DIO_PIN2);

					fall_step = 0;
					stability_counter = 0;
				}
			} else {
				fall_step = 0;
				stability_counter = 0;
			}
		}

		_delay_ms(100);
	}

	return 0;
}
