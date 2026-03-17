/*
 * telegram.c
 *
 * Created: 16/03/2026 08:30:24
 *  Author: maria
 */ 

//#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "../../MCAL/UART/uart.h"
#include "../../HAL/WIFI/wifi.h"
#include "telegram.h"

/* Initialize Telegram service (requires WiFi) */
void Telegram_Init(void)
{
	WIFI_Init();
	if(!WIFI_Connect())
	{
		// Optional: handle failure

		LCD_clearScreen();
		LCD_displayString("WiFi Failed");

		while(1); // stop system
	}
}

/* Send simple message to Telegram */
void Telegram_SendMessage(char *message)
{
	char httpRequest[300];
	sprintf(httpRequest,
	"GET /bot%s/sendMessage?chat_id=%s&text=%s HTTP/1.1\r\n"
	"Host: api.telegram.org\r\n"
	"Connection: close\r\n\r\n",
	TELEGRAM_BOT_TOKEN, TELEGRAM_CHAT_ID, message);

	WIFI_SendHTTPRequest("api.telegram.org", httpRequest);
}

/* Send emergency alert with heart rate included */
void Telegram_SendEmergency(int heartRate)
{
	char buffer[100];
	sprintf(buffer, "Emergency! Fall detected! Heart rate: %d bpm", heartRate);
	Telegram_SendMessage(buffer);
}

// for future upgrades when a server is add for holding commands from the guardian
void Telegram_CheckCommands(void)
{
	// Placeholder for server-based polling
}