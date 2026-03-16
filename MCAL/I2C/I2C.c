/*
 * I2C.c
 *
 *  Created on: Jul 31, 2025
 *      Author: c.city
 */

#include "../../LIB/BIT_MATH.h"
#include "../../LIB/STD_types.h"
#include "../DIO/DIO.h"
#include "I2C.h"
#include <util\delay.h>
#include "../USART_Online/USART_Online.h"


u8 I2C_GetStatus(void)
{
	return (TWSR & 0xF8);
}


void I2C_Masterinit(u32 SCL){
	TWBR = ((16000000UL/SCL) - 16) /(2*I2C_PRESCALER) ;

	switch(I2C_PRESCALER)
	{
	case 1:  TWSR =0; break;
	case 4:  TWSR =1; break;
	case 16: TWSR =2; break;
	case 64: TWSR =3; break;
	}

	 TWCR = (1<<TWEN);
}


void I2C_SendStartCond(void){
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTA);
    while(READ_BIT(TWCR, TWINT) == 0);
    while( (I2C_GetStatus() != 0x08) &&
           (I2C_GetStatus() != 0x10) );
}

void I2C_SendAdd(u8 Add){
    TWDR = Add;
    TWCR = (1<<TWINT) | (1<<TWEN);
    while(READ_BIT(TWCR, TWINT) == 0);
    u8 status = I2C_GetStatus();
    /*if(status != 0x20)
    {
        USART_SendString("Address NACK\r\n");
    }*/
}

/*void I2C_SendData(u8 Data){
      TWDR=Data;
      TWCR = (1<<TWINT) | (1<<TWEN);
      while(READ_BIT(TWCR, TWINT) == 0);
      //while((TWSR & 0xF8) != 0x28);
}*/

void I2C_SendData(u8 Data){
    TWDR = Data;
    TWCR = (1<<TWINT) | (1<<TWEN);

    // Wait for hardware to finish
    while(READ_BIT(TWCR, TWINT) == 0);


    // Check status WITHOUT blocking
    /*if((TWSR & 0xF8) != 0x28) {
        USART_SendString("Data NACK! ");
    } else {
        USART_SendString("Data ACK! ");
    }*/
}

void I2C_SendStopCond(void){
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	 while(TWCR & (1<<TWSTO));
}


void I2C_SetAdd(u8 Add){
    TWAR = Add << 1 ; //shift the address not put 1 in a bit
}

u8 I2C_SlaveRead(void){
    //one wait for slave address and another for data

	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(READ_BIT(TWCR, TWINT) == 0);
	while((TWSR & 0xF8) != 0x60); //wait for address

	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(READ_BIT(TWCR, TWINT) == 0);
	while((TWSR & 0xF8) != 0x81);
	//wait for address (it should be 80 but 81 to READ to put 1 at the end of r/w bit)
	return TWDR;

}


u8 I2C_MasterReadAck(void){
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(READ_BIT(TWCR, TWINT) == 0);
	while(I2C_GetStatus() != 0x50);
	return TWDR;
}

u8 I2C_MasterReadNack(void){
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(READ_BIT(TWCR, TWINT) == 0);
	while(I2C_GetStatus() != 0x58);
	return TWDR;
}
