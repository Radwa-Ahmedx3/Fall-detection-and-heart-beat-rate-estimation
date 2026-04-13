/*
 * ADC.h
 *
 *  Created on: Jul 28, 2025
 *      Author: c.city
 */

#ifndef MCAL_ADC_ADC_H_
#define MCAL_ADC_ADC_H_

#define ADMUX   (*(volatile u8 *)(0x27))
#define ADCSRA  (*(volatile u8 *)(0x26))
#define ADC     (*(volatile u16*)(0x24)) //pointer to 0x24 with size 16 this takes 0x24 low and 0x25 high
#define SFIOR   (*(volatile u8 *)(0x50))
#define SREG    (*(volatile u8 *)(0x5F))

#define REFS1  7
#define REFS0  6
#define ADLAR  5
#define MUX4   4
#define MUX3   3
#define MUX2   2
#define MUX1   1
#define MUX0   0

#define ADEN   7
#define ADSC   6
#define ADATE  5
#define ADIF   4
#define ADIE   3
#define ADPS2  2
#define ADPS1  1
#define ADPS0  0

#define ADC_REFERENCE_INTERNAL 1
#define ADC_REFERENCE_AVCC     2
#define ADC_REFERENCE_EXTERNAL 3

#define ADC_CHANNEL0 0
#define ADC_CHANNEL1 1
#define ADC_CHANNEL2 2
#define ADC_CHANNEL3 3
#define ADC_CHANNEL4 4
#define ADC_CHANNEL5 5
#define ADC_CHANNEL6 6
#define ADC_CHANNEL7 7

void ADC_init(u8 ADC_ref);
void ADC_startConv(u8 channel);
u16 ADC_readDataPoll(void);
u16 ADC_readDataINT(void);
void ADC_callBack(void (*fptr)(void));


#endif /* MCAL_ADC_ADC_H_ */
