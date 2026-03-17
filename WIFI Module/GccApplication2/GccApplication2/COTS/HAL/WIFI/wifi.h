/*
 * wifi.h
 *
 * Created: 15/03/2026 21:31:27
 *  Author: maria
 */ 

#ifndef WIFI_H_
#define WIFI_H_

//#include <stdint.h>
#include "../../LIB/std_types.h"

/* WiFi credentials */
#define WIFI_SSID       "nana"
#define WIFI_PASSWORD   "01023025564"

/* Blynk token */
#define BLYNK_AUTH_TOKEN   "tXaS2EX1wjwMv1SvCIz-si2XfgGck52e"


/* WiFi control */
void WIFI_Init(void);
uint8_t WIFI_Connect(void);

/* Blynk communication */
void WIFI_SendBlynkValue(uint8_t value);

/* Generic HTTP request (for Service methods like Telegram) */
void WIFI_SendHTTPRequest(const char* host, const char* request);

#endif /* WIFI_H_ */