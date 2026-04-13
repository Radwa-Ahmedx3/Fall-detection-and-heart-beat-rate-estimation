#ifndef USART_ONLINE_H_
#define USART_ONLINE_H_

/* Make sure F_CPU is defined before including this file */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include "../../LIB/STD_types.h"

/* USART Functions */
void USART_Init(unsigned long baud);
char USART_RxChar(void);
void USART_TxChar(char data);
void USART_SendString(char *str);
void USART_SendNumber(u32 num);

/* MPU Logging Function */
void USART_LogMPUData(float Ax, float Ay, float Az,
                      float Gx, float Gy, float Gz);

void USART_SendNumber(u32 num);

#endif /* USART_RS232_H_FILE_H_ */
