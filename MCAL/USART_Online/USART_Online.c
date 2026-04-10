#define F_CPU 16000000UL

#include "USART_Online.h"
#include <stdlib.h>
#include <stdio.h>
#include <avr/dtostrf.h>
#include "../../LIB/STD_types.h"

#ifndef BAUDRATE
#define BAUDRATE 9600
#endif

/* Initialize USART */
void USART_Init(unsigned long baud)
{
    unsigned int ubrr = (F_CPU / (16UL * baud)) - 1;

    UBRRH = (unsigned char)(ubrr >> 8);
    UBRRL = (unsigned char)ubrr;

    UCSRB = (1 << RXEN) | (1 << TXEN);   // Enable RX & TX
    UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); // 8-bit data
}

/* Receive character */
char USART_RxChar()
{
    while (!(UCSRA & (1 << RXC)));
    return UDR;
}

/* Transmit character */
void USART_TxChar(char data)
{
    while (!(UCSRA & (1 << UDRE)));
    UDR = data;
}

/* Send string */
void USART_SendString(char *str)
{
    while (*str)
    {
        USART_TxChar(*str++);
    }
}

/* Send float helper */
static void USART_SendFloat(float value)
{
    char buffer[20];
    dtostrf(value, 6, 2, buffer);  // width=6, 2 decimal places
    USART_SendString(buffer);
}

/* Log MPU data nicely */
void USART_LogMPUData(float Ax, float Ay, float Az,
                      float Gx, float Gy, float Gz)
{
    USART_SendString("ACC: ");
    USART_SendFloat(Ax);
    USART_SendString(" ");
    USART_SendFloat(Ay);
    USART_SendString(" ");
    USART_SendFloat(Az);

    USART_SendString("  GYRO: ");
    USART_SendFloat(Gx);
    USART_SendString(" ");
    USART_SendFloat(Gy);
    USART_SendString(" ");
    USART_SendFloat(Gz);

    USART_SendString("\r\n");
}

void USART_SendNumber(u32 num)
{
    char str[12];   // enough for -2147483648
    sprintf(str, "%lu", num);  // signed long
    USART_SendString(str);
    USART_SendString("\r\n");
}
// This function sends JUST the digits, no extra spaces or new lines
void USART_SendNumber_Raw(s32 num) {
    char buffer[11];
    itoa(num, buffer, 10); // Converts integer to string
    USART_SendString(buffer);
}

void USART_SendFloat_chato(f32 number, u8 decimal_places) {
    if (number < 0) {
        USART_SendString("-"); // Use String or Char
        number = -number;
    }

    u32 integer_part = (u32)number;
    USART_SendNumber_Raw(integer_part); // <--- Use Raw version

    USART_SendString(".");

    f32 fraction = number - (f32)integer_part;
    for (u8 i = 0; i < decimal_places; i++) {
        fraction *= 10;
        u8 digit = (u8)fraction;
        USART_SendNumber_Raw(digit);    // <--- Use Raw version
        fraction -= digit;
    }
}
