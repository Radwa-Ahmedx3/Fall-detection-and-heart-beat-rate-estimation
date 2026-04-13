/*
 * I2C.h
 *
 *  Created on: Jul 31, 2025
 *      Author: c.city
 */

#ifndef MCAL_I2C_I2C_H_
#define MCAL_I2C_I2C_H_

#define TWBR   (*(volatile u8 *)(0x20))
#define TWCR   (*(volatile u8 *)(0x56))
#define TWSR   (*(volatile u8 *)(0x21))
#define TWDR   (*(volatile u8 *)(0x23))
#define TWAR   (*(volatile u8 *)(0x22))

#define TWINT      7
#define TWEA       6
#define TWSTA      5
#define TWSTO      4
#define TWWC       3
#define TWEN       2
#define TWIE       0

#define I2C_PRESCALER 4

void I2C_Masterinit(u32 SCL);
void I2C_SendStartCond(void);
void I2C_SendAdd(u8 Add);  //add.ofslave
void I2C_SendData(u8 Data);
void I2C_SendStopCond(void);
void I2C_SetAdd(u8 Add);   //set my add.
u8 I2C_SlaveRead(void);

/* Master Receiver */
u8 I2C_MasterReadAck(void);
u8 I2C_MasterReadNack(void);

#endif /* MCAL_I2C_I2C_H_ */
