//#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include "../../MCAL/UART/uart.h"
#include "wifi.h"

/* Initialize ESP WiFi module */
void WIFI_Init(void)
{
	_delay_ms(3000);  // wait for ESP boot

	USART_SendString("AT\r\n");
	USART_WaitFor("OK");

	// Set station mode (1 -> station mode) for router connecting
	USART_SendString("AT+CWMODE=1\r\n");
	USART_WaitFor("OK");
}

/* Connect to WiFi */
uint8_t WIFI_Connect(void)
{
	char cmd[100];
	
	// fulling cmd with the SSID and the password (AT+CWJAP="SSID","PASSWORD")
	sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWORD); 
	USART_SendString(cmd); // sending to the wifi module

	if (USART_WaitFor("WIFI GOT IP"))
	return 1;
	else
	return 0;
}

/* Send value to Blynk */
void WIFI_SendBlynkValue(uint8_t value)
{
	char httpRequest[200];
	sprintf(httpRequest,
	"GET /external/api/update?token=%s&v1=%d HTTP/1.1\r\n"
	"Host: blynk.cloud\r\n"
	"Connection: close\r\n\r\n",
	BLYNK_AUTH_TOKEN, value);

	WIFI_SendHTTPRequest("blynk.cloud", httpRequest);
}

/* Generic HTTP request sender */
void WIFI_SendHTTPRequest(const char* host, const char* request)
{
	char cmd[50];

	sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",80\r\n", host);
	USART_SendString(cmd);
	USART_WaitFor("OK"); // not sure but can cause errors if the wifi module sends something different than "OK"

	sprintf(cmd, "AT+CIPSEND=%d\r\n", strlen(request));
	USART_SendString(cmd);
	USART_WaitFor(">");

	USART_SendString(request);
	USART_WaitFor("SEND OK");
	
	// closing the connection 
	USART_SendString("AT+CIPCLOSE\r\n");
	USART_WaitFor("OK");   // optional but recommended
}