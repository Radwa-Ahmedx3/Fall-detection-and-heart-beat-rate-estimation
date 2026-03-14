/*
 * LCD.c
 *
 *  Created on: Jul 23, 2025
 *      Author: c.city
 */

#include "../../LIB/BIT_MATH.h"
#include "../../LIB/STD_TYPES.h"
#include "../../MCAL/DIO/DIO.h"
#include "LCD.h"
#include <util/delay.h>

void LCD_writedata(u8 data) {
#if MODE == 8
	DIO_SetPinValue(RW_PORT, RW_PIN, DIO_LOW);
	DIO_SetPinValue(RS_PORT, RS_PIN, DIO_HIGH);
	DIO_SetPortValue(LCD_PORT, data);
	DIO_SetPinValue(E_PORT, E_PIN, DIO_HIGH);
	_delay_ms(1);
	DIO_SetPinValue(E_PORT, E_PIN, DIO_LOW);
	_delay_ms(1);

#else if MODE == 4
	//send data by 2 steps (higher 4 then lower 4)
	u8 LowNibble = data & 0x0F;
	u8 HighNibble = data >> 4;

	//send the highnibble first then low nibble
	DIO_SetPinValue(LCD_PORT, 3, READ_BIT(HighNibble, 0));
	DIO_SetPinValue(LCD_PORT, 4, READ_BIT(HighNibble, 1));
	DIO_SetPinValue(LCD_PORT, 5, READ_BIT(HighNibble, 2));
	DIO_SetPinValue(LCD_PORT, 6, READ_BIT(HighNibble, 3));

	DIO_SetPinValue(RW_PORT, RW_PIN, DIO_LOW);
	DIO_SetPinValue(RS_PORT, RS_PIN, DIO_HIGH);
	//E-Pulse
	DIO_SetPinValue(E_PORT, E_PIN, DIO_HIGH);
	_delay_ms(1);
	DIO_SetPinValue(E_PORT, E_PIN, DIO_LOW);
	_delay_ms(1);

	//low nibble
	DIO_SetPinValue(LCD_PORT, 3, READ_BIT(LowNibble, 0));
	DIO_SetPinValue(LCD_PORT, 4, READ_BIT(LowNibble, 1));
	DIO_SetPinValue(LCD_PORT, 5, READ_BIT(LowNibble, 2));
	DIO_SetPinValue(LCD_PORT, 6, READ_BIT(LowNibble, 3));

	DIO_SetPinValue(RW_PORT, RW_PIN, DIO_LOW);
	DIO_SetPinValue(RS_PORT, RS_PIN, DIO_HIGH);

	DIO_SetPinValue(E_PORT, E_PIN, DIO_HIGH);
	_delay_ms(1);
	DIO_SetPinValue(E_PORT, E_PIN, DIO_LOW);
	_delay_ms(1);

#endif
}
void LCD_writecmd(u8 cmd) {
#if MODE ==8
	DIO_SetPinValue(RW_PORT, RW_PIN, DIO_LOW);
	DIO_SetPinValue(RS_PORT, RS_PIN, DIO_LOW);
	DIO_SetPortValue(LCD_PORT, cmd);
	DIO_SetPinValue(E_PORT, E_PIN, DIO_HIGH);
	_delay_ms(1);
	DIO_SetPinValue(E_PORT, E_PIN, DIO_LOW);
	_delay_ms(1);

#else if MODE == 4
	//send data by 2 steps (higher 4 then lower 4)
	u8 LowNibble = cmd & 0x0f;
	u8 HighNibble = cmd >> 4;

	DIO_SetPinValue(LCD_PORT, 3, READ_BIT(HighNibble, 0));
	DIO_SetPinValue(LCD_PORT, 4, READ_BIT(HighNibble, 1));
	DIO_SetPinValue(LCD_PORT, 5, READ_BIT(HighNibble, 2));
	DIO_SetPinValue(LCD_PORT, 6, READ_BIT(HighNibble, 3));

	DIO_SetPinValue(RS_PORT, RS_PIN, DIO_LOW);
	DIO_SetPinValue(E_PORT, E_PIN, DIO_HIGH);
	_delay_ms(1);
	DIO_SetPinValue(E_PORT, E_PIN, DIO_LOW);
	_delay_ms(1);


	DIO_SetPinValue(LCD_PORT, 3, READ_BIT(LowNibble, 0));
	DIO_SetPinValue(LCD_PORT, 4, READ_BIT(LowNibble, 1));
	DIO_SetPinValue(LCD_PORT, 5, READ_BIT(LowNibble, 2));
	DIO_SetPinValue(LCD_PORT, 6, READ_BIT(LowNibble, 3));

	DIO_SetPinValue(RW_PORT, RW_PIN, DIO_LOW);
	DIO_SetPinValue(RS_PORT, RS_PIN, DIO_LOW);
	DIO_SetPinValue(E_PORT, E_PIN, DIO_HIGH);
	_delay_ms(1);
	DIO_SetPinValue(E_PORT, E_PIN, DIO_LOW);
	_delay_ms(1);

#endif
}

void LCD_init(void) {
#if MODE ==8
	_delay_ms(50);
	DIO_InitPort(LCD_PORT, 0xFF);
	DIO_InitPin(RW_PORT, RW_PIN, OUTPUT);
	DIO_InitPin(RS_PORT, RS_PIN, OUTPUT);
	DIO_InitPin(E_PORT, E_PIN, OUTPUT);

	/*modes (found in data sheet)*/

	LCD_writecmd(0x38); //for choosing (5*8 dots)and 2 line
	_delay_ms(1);

	LCD_writecmd(0x0E); //appearing cursor
	_delay_ms(1);

	LCD_writecmd(0x01); //clear command
	_delay_ms(5);

	LCD_writecmd(0x06);
	_delay_ms(1);

#elif MODE == 4
	_delay_ms(50);
	//here
	DIO_InitPin(LCD_PORT, 3, DIO_OUTPUT ); //only using last 4 pins
	DIO_InitPin(LCD_PORT, 4, DIO_OUTPUT );
	DIO_InitPin(LCD_PORT, 5, DIO_OUTPUT );
	DIO_InitPin(LCD_PORT, 6, DIO_OUTPUT );

	DIO_InitPin(RW_PORT, RW_PIN, DIO_OUTPUT );
	DIO_InitPin(RS_PORT, RS_PIN, DIO_OUTPUT );
	DIO_InitPin(E_PORT, E_PIN, DIO_OUTPUT );

	LCD_writecmd(0x02);
	_delay_ms(1);

	LCD_writecmd(0x28); //same but for 4 bit, bit responsible is 0
	_delay_ms(1);

	LCD_writecmd(0x0E); //appearing cursor
	_delay_ms(1);

	LCD_writecmd(0x01); //clear command
	_delay_ms(5);

	LCD_writecmd(0x06);
	_delay_ms(1);

#endif
}

void LCD_writestr(u8 *ptrstr) {
	u8 i = 0;
	while (ptrstr[i] != '\0') {
		LCD_writedata(ptrstr[i]);
		i++;
	}
}

void LCD_clear(void) {
	LCD_writecmd(0x01);
	_delay_ms(5);
}

void LCD_shiftdata(void) {
	LCD_writecmd(0x1C);
	_delay_ms(1);
}

void LCD_movecursor(u8 row, u8 col) {
	u8 AC;
	if (row >= 0 && row <= 1) {
		if (col >= 0 && col <= 15) {
			if (row == 0) {
				AC = col;
			} else {
				AC = col + 0x40;
			}
			LCD_writecmd(AC + 0x80); //to inform that we are calling the DDRAM
		}
	}
}

void LCD_writecostumchar(u8 arr[], u8 block, u8 row, u8 col) {
	u8 i;
	LCD_writecmd(0x40 + (block * 8)); //to talk to cgrom
	for (i = 0; i < 8; i++) {
		LCD_writedata(arr[i]);
	}
	LCD_movecursor(row,col);
	LCD_writedata(block);
}



static void lcd_sendData(u8 data)
{
	DIO_SetPinValue(RS_PORT, RS_PIN, DIO_HIGH);

	DIO_SetPortValue(LCD_PORT, data);

}

void lcd_displayChr(u8 chr)
{
   lcd_sendData (chr);
}


void LCD_print_3_digit(u16 digit) {

	int display;


	display = digit / 100;
	LCD_writedata(display + '0');

	digit = digit % 100;



	display = digit / 10;

	LCD_writedata(display + '0');

	digit = digit % 10;

	LCD_writedata(display + '0');


}
