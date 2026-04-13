/*
 * LED.c
 *
 *  Created on: Jul 22, 2025
 *      Author: c.city
 */

#include "../../LIB/BIT_MATH.h"
#include "../../LIB/STD_TYPES.h"
#include "../../MCAL/DIO/DIO.h"
#include "LED.h"


void LED_init(u8 LedPort, u8 LedPin)
{
	DIO_InitPin(LedPort , LedPin , DIO_OUTPUT ) ;
}

void LED_ON(u8 LedPort, u8 LedPin)
{
	DIO_SetPinValue(LedPort, LedPin, DIO_HIGH);
}

void LED_OFF(u8 LedPort, u8 LedPin)
{
	DIO_SetPinValue(LedPort, LedPin, DIO_LOW);
}

void LED_TOG(u8 LedPort, u8 LedPin)
{
	DIO_TogPinValue(LedPort, LedPin);
}
