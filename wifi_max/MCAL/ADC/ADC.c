/*
 * ADC.c
 *
 *  Created on: Jul 28, 2025
 *      Author: c.city
 */
#include "../../LIB/BIT_MATH.h"
#include "../../LIB/STD_types.h"
#include "ADC.h"

void (*ADC_fptr)(void);

void ADC_init (u8 ADC_ref)
{
   switch (ADC_ref){
   case ADC_REFERENCE_INTERNAL:
	   CLR_BIT(ADMUX,REFS1);
	   CLR_BIT(ADMUX,REFS0);
	   break;
   case ADC_REFERENCE_AVCC:
	   CLR_BIT(ADMUX,REFS1);
	   SET_BIT(ADMUX,REFS0);
	   break;
   case ADC_REFERENCE_EXTERNAL:
	   SET_BIT(ADMUX,REFS1);
	   SET_BIT(ADMUX,REFS0);
	   break;
   }

   //Right adjust
   CLR_BIT(ADMUX, ADLAR);

   //ADC enable
   SET_BIT(ADCSRA, ADEN);

   //Enable ADC DIE
   SET_BIT(ADCSRA, ADIE);

   //Enable Prescaler /64
   SET_BIT(ADCSRA, ADPS2);
   SET_BIT(ADCSRA, ADPS1);
   SET_BIT(ADCSRA, ADPS0);

   //Disable auto trig
   CLR_BIT(ADCSRA, ADATE);

}

void ADC_startConv(u8 channel)
{
   switch(channel){
   case ADC_CHANNEL0:
	   CLR_BIT(ADMUX, MUX2);
	   CLR_BIT(ADMUX, MUX1);
	   CLR_BIT(ADMUX, MUX0);
	   break;
   case ADC_CHANNEL1:
	   CLR_BIT(ADMUX, MUX2);
	   CLR_BIT(ADMUX, MUX1);
	   SET_BIT(ADMUX, MUX0);
	   break;
   case ADC_CHANNEL2:
	   CLR_BIT(ADMUX, MUX2);
	   SET_BIT(ADMUX, MUX1);
	   CLR_BIT(ADMUX, MUX0);
	   break;
   case ADC_CHANNEL3:
	   CLR_BIT(ADMUX, MUX2);
	   SET_BIT(ADMUX, MUX1);
	   SET_BIT(ADMUX, MUX0);
	   break;
   case ADC_CHANNEL4:
	   SET_BIT(ADMUX, MUX2);
	   CLR_BIT(ADMUX, MUX1);
	   CLR_BIT(ADMUX, MUX0);
	   break;
   case ADC_CHANNEL5:
	   SET_BIT(ADMUX, MUX2);
	   CLR_BIT(ADMUX, MUX1);
	   SET_BIT(ADMUX, MUX0);
	   break;
   case ADC_CHANNEL6:
	   SET_BIT(ADMUX, MUX2);
	   SET_BIT(ADMUX, MUX1);
	   CLR_BIT(ADMUX, MUX0);
	   break;
   case ADC_CHANNEL7:
	   SET_BIT(ADMUX, MUX2);
	   SET_BIT(ADMUX, MUX1);
	   SET_BIT(ADMUX, MUX0);
	   break;
   }

   //start conversion
   SET_BIT(ADCSRA, ADSC);
}

u16 ADC_readDataPoll (void)
{
	while(READ_BIT(ADCSRA , ADIF) ==0); //stuck when flag is zero
    SET_BIT(ADCSRA, ADIF); //to be zeroed by hardware
	return ADC;
}

u16 ADC_readDataINT (void)
{

    return ADC;
}

void ADC_callBack (void(*fptr)(void))
{
   ADC_fptr = fptr;
}

void __vector_16(void)__attribute((signal));
void __vector_16(void)
{
	if(ADC_fptr !=0)
		ADC_fptr();
}
