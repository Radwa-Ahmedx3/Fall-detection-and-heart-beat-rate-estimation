/*
 * KEY_PAD.c
 *
 *  Created on: Jul 23, 2025
 *      Author: c.city
 */

#include "../../LIB/BIT_MATH.h"
#include "../../LIB/STD_TYPES.h"
#include "../../MCAL/DIO/DIO.h"
#include "KEY_PAD.h"
#include <util/delay.h>

uint8 KP_array[4][4] = {
		{ '7', '4', '1', 'C' },
		{ '8', '5', '2', '0' },
		{ '9', '6', '3', '=' },
		{ '/', '*', '-', '+' },
};

void Key_pad_init(uint8 KP_Port)
{
	//Columns 4 -> 7 output (F) and Rows 0 -> 3 input (0)
	DIO_InitPort(KP_Port, 0xF0);
	DIO_SetPortValue(KP_Port, 0xFF); //Columns F to be high and Rows F to be PU resistors
}

uint8 Key_Pad_getDATA(uint8 KP_Port) {
	uint8 col, row;
	uint8 state, value = KP_NOT_PRESSED;
	for (col = 0; col < 4; col++) {
		DIO_SetPinValue(KP_Port, col + 4, DIO_LOW); //making the current column zero
		for (row = 0; row < 4; row++) {
			state = DIO_ReadPinValue(KP_Port, row);
			if (state == KP_PRESSED) { //for debouncing of keypad
				_delay_ms(20);
				state = DIO_ReadPinValue(KP_Port, row);
				if (state == KP_PRESSED) {
					value = KP_array[col][row];
					while (DIO_ReadPinValue(KP_Port, row) == KP_PRESSED); //until the user removes his hand
					_delay_ms(20);
					DIO_SetPinValue(KP_Port, col + 4, DIO_HIGH);
					return value;
				}
			}
		}
		DIO_SetPinValue(KP_Port, col + 4, DIO_HIGH); //when finish, return the column to high

	}
	return KP_NOT_PRESSED;
}
