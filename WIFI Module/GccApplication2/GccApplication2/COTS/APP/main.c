/*
 * GccApplication2.c
 *
 * Created: 15/03/2026 20:50:39
 * Author : maria
 */ 

//#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "../MCAL/UART/uart.h"
#include "../HAL/LCD/lcd.h"
#include "../HAL/WIFI/wifi.h"
#include "../HAL/TELEGRAM/telegram.h"

int main(void)
{
	u8 heartrate = 85; // test heart rate sample
	USART_Init();
	LCD_init();

	LCD_displayString("System Boot");

	Telegram_Init();  // initializes WiFi and connects

	LCD_clearScreen();
	LCD_displayString("WiFi Connected");

	uint8_t counter = 0;

	while(1)
	{
		// Example Blynk update
		WIFI_SendBlynkValue(counter);
		LCD_clearScreen();
		LCD_displayString("Blynk Sent");

		counter++;
		if(counter > 100) counter = 0;

		// Example Telegram emergency alert
		if(counter == 10)
		{
			Telegram_SendEmergency(heartrate); // sending heart rate value from the sensor
			LCD_clearScreen();
			LCD_displayString("Telegram Alert Sent");
		}

		_delay_ms(1500);
		
		//////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////// Testing Telegram Bot ///////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////
		
		//Telegram_SendMessage("Hallo");
	}
}