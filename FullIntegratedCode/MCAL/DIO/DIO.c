/*
 * DIO.c
 *
 *  Created on: Jul 22, 2025
 *      Author: c.city
 */

#include "../../LIB/BIT_MATH.h"
#include "../../LIB/STD_TYPES.h"

#include "DIO.h"

void DIO_InitPin       (u8 PortName , u8 PinNum , u8 Mode )
{
	if (PortName >= DIO_PORTA && PortName <= DIO_PORTD)
	{
		if (PinNum >= DIO_PIN0 && PinNum <= DIO_PIN7)
		{
			switch (PortName)
			{
			case DIO_PORTA:
				if (Mode == DIO_INPUT)
				{
					CLR_BIT(DDRA , PinNum) ;
				}
				else if (Mode == DIO_OUTPUT)
				{
					SET_BIT(DDRA , PinNum) ;
				}
				break ;
			case DIO_PORTB:
				if (Mode == DIO_INPUT)
				{
					CLR_BIT(DDRB , PinNum) ;
				}
				else if (Mode == DIO_OUTPUT)
				{
					SET_BIT(DDRB , PinNum) ;
				}
				break ;
			case DIO_PORTC:
				if (Mode == DIO_INPUT)
				{
					CLR_BIT(DDRC , PinNum) ;
				}
				else if (Mode == DIO_OUTPUT)
				{
					SET_BIT(DDRC , PinNum) ;
				}
				break ;
			case DIO_PORTD:
				if (Mode == DIO_INPUT)
				{
					CLR_BIT(DDRD , PinNum) ;
				}
				else if (Mode == DIO_OUTPUT)
				{
					SET_BIT(DDRD , PinNum) ;
				}
				break ;
			}
		}
	}
}
void DIO_SetPinValue  (u8 PortName , u8 PinNum , u8 Value)
{
	if (PortName >= DIO_PORTA && PortName <= DIO_PORTD)
	{
		if (PinNum >= DIO_PIN0 && PinNum <= DIO_PIN7)
		{
			switch (PortName)
			{
			case DIO_PORTA:
				if (Value == DIO_LOW)
				{
					CLR_BIT(PORTA , PinNum) ;
				}
				else if (Value == DIO_HIGH)
				{
					SET_BIT(PORTA , PinNum) ;
				}
				break ;
			case DIO_PORTB:
				if (Value == DIO_LOW)
				{
					CLR_BIT(PORTB , PinNum) ;
				}
				else if (Value == DIO_HIGH)
				{
					SET_BIT(PORTB , PinNum) ;
				}
				break ;
			case DIO_PORTC:
				if (Value == DIO_LOW)
				{
					CLR_BIT(PORTC , PinNum) ;
				}
				else if (Value == DIO_HIGH)
				{
					SET_BIT(PORTC , PinNum) ;
				}
				break ;
			case DIO_PORTD:
				if (Value == DIO_LOW)
				{
					CLR_BIT(PORTD , PinNum) ;
				}
				else if (Value == DIO_HIGH)
				{
					SET_BIT(PORTD , PinNum) ;
				}
				break ;
			}
		}
	}
}
u8   DIO_ReadPinValue (u8 PortName , u8 PinNum)
{
	u8 PinStatues ;
	if (PortName >= DIO_PORTA && PortName <= DIO_PORTD)
	{
		if (PinNum >= DIO_PIN0 && PinNum <= DIO_PIN7)
		{
			switch (PortName)
			{
			case DIO_PORTA:
				PinStatues = READ_BIT(PINA , PinNum) ;
				break ;
			case DIO_PORTB:
				PinStatues = READ_BIT(PINB , PinNum) ;
				break ;
			case DIO_PORTC:
				PinStatues = READ_BIT(PINC , PinNum) ;
				break ;
			case DIO_PORTD:
				PinStatues = READ_BIT(PIND , PinNum) ;
				break ;
			}
		}
	}
	return PinStatues ;
}
void DIO_TogPinValue  (u8 PortName , u8 PinNum           )
{
	if (PortName >= DIO_PORTA && PortName <= DIO_PORTD)
	{
		if (PinNum >= DIO_PIN0 && PinNum <= DIO_PIN7)
		{
			switch (PortName)
			{
			case DIO_PORTA:
				TOG_BIT(PORTA , PinNum) ;
				break ;
			case DIO_PORTB:
				TOG_BIT(PORTB , PinNum) ;
				break ;
			case DIO_PORTC:
				TOG_BIT(PORTC , PinNum) ;
				break ;
			case DIO_PORTD:
				TOG_BIT(PORTD , PinNum) ;
				break ;
			}
		}
	}
}
void DIO_InitPort      (u8 PortName , u8 Mode )
{
	if (PortName >= DIO_PORTA && PortName <= DIO_PORTD)
	{
		switch (PortName)
		{
		case DIO_PORTA:
			DDRA = Mode ;
			break ;
		case DIO_PORTB:
			DDRB = Mode ;
			break ;
		case DIO_PORTC:
			DDRC = Mode ;
			break ;
		case DIO_PORTD:
			DDRD = Mode ;
			break ;
		}
	}
}
void DIO_SetPortValue  (u8 PortName , u8 Value)
{
	if (PortName >= DIO_PORTA && PortName <= DIO_PORTD)
	{
		switch (PortName)
		{
		case DIO_PORTA:
			PORTA = Value ;
			break ;
		case DIO_PORTB:
			PORTB = Value ;
			break ;
		case DIO_PORTC:
			PORTC = Value ;
			break ;
		case DIO_PORTD:
			PORTD = Value ;
			break ;
		}
	}
}
void DIO_TogPortValue  (u8 PortName           )
{
	if (PortName >= DIO_PORTA && PortName <= DIO_PORTD)
	{
		switch (PortName)
		{
		case DIO_PORTA:
			PORTA = ~PORTA ;
			break ;
		case DIO_PORTB:
			PORTB = ~PORTB ;
			break ;
		case DIO_PORTC:
			PORTC = ~PORTC ;
			break ;
		case DIO_PORTD:
			PORTD = ~PORTD ;
			break ;
		}
	}
}
u8   DIO_ReadPortValue (u8 PortName           )
{
	u8 PortStatues ;
	if (PortName >= DIO_PORTA && PortName <= DIO_PORTD)
	{
		switch (PortName)
		{
		case DIO_PORTA:
			PortStatues = PINA ;
			break ;
		case DIO_PORTB:
			PortStatues = PINB ;
			break ;
		case DIO_PORTC:
			PortStatues = PINC ;
			break ;
		case DIO_PORTD:
			PortStatues = PIND ;
			break ;
				}
			}
			return PortStatues ;
		}
