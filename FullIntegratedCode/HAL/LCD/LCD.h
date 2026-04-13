/*
 * LCD.h
 *
 *  Created on: Jul 23, 2025
 *      Author: c.city
 */
#ifndef LCD_H_
#define LCD_H_

#define LCD_PORT    DIO_PORTA
#define RS_PORT     DIO_PORTA
#define RS_PIN      DIO_PIN1
#define RW_PORT     DIO_PORTA
#define RW_PIN      DIO_PIN7
#define E_PORT      DIO_PORTA
#define E_PIN       DIO_PIN2

#define MODE 4

void LCD_writedata       (u8 data);
void LCD_writecmd        (u8 cmd);
void LCD_init            (void);
void LCD_writestr        (u8 * ptrstr);
void LCD_clear           (void);
void LCD_shiftdata       (void);
void LCD_movecursor      (u8 row, u8 col);
void LCD_writecostumchar (u8 arr[], u8 block, u8 row, u8 col);
void lcd_displayChr      (u8 chr);
static void lcd_sendData (u8 data);
void LCD_print_3_digit   (u16 digit);

#endif

