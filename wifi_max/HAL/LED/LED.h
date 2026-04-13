/*
 * LED.h
 *
 *  Created on: Jul 22, 2025
 *      Author: c.city
 */

#ifndef HAL_LED_LED_H_
#define HAL_LED_LED_H_

void LED_init(u8 LedPort, u8 LedPin);
void LED_ON(u8 LedPort, u8 LedPin);
void LED_OFF(u8 LedPort, u8 LedPin);
void LED_TOG(u8 LedPort, u8 LedPin);

#endif /* HAL_LED_LED_H_ */
