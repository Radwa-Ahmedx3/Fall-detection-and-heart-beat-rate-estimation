#include "../../LIB/BIT_MATH.h"
#include "../../LIB/STD_TYPES.h"

#include "../DIO/DIO.h"
#include<avr/io.h>
#include "Timer1.h"
u32 G_Prescaller ;
void (*Timer1_ptr)(void) ;
u32 G_OverFlowCounts ;
u32 counter = 0 ;
void Timer1_Init (u32 Prescaller , u8 Mode)
{
	SET_BIT(SREG , I_BIT) ;
	G_Prescaller = Prescaller ;
	switch (Prescaller)
	{
	case TIMER1_NO_PRESCALLER  :
		CLR_BIT(TCCR1B , CS12) ;
		CLR_BIT(TCCR1B , CS11) ;
		SET_BIT(TCCR1B , CS10) ;
		break ;
	case TIMER1_PRESCALLER8    :
		CLR_BIT(TCCR1B , CS12) ;
		SET_BIT(TCCR1B , CS11) ;
		CLR_BIT(TCCR1B , CS10) ;
		break ;
	case TIMER1_PRESCALLER64   :
		CLR_BIT(TCCR1B , CS12) ;
		SET_BIT(TCCR1B , CS11) ;
		SET_BIT(TCCR1B , CS10) ;
		break ;
	case TIMER1_PRESCALLER256  :
		SET_BIT(TCCR1B , CS12) ;
		CLR_BIT(TCCR1B , CS11) ;
		CLR_BIT(TCCR1B , CS10) ;
		break ;
	case TIMER1_PRESCALLER1024 :
		SET_BIT(TCCR1B , CS12) ;
		CLR_BIT(TCCR1B , CS11) ;
		SET_BIT(TCCR1B , CS10) ;
		break ;
	}

	switch (Mode)
	{
	case TIMER1_NORMAL                  :
		CLR_BIT(TCCR1B , WGM13) ;
		CLR_BIT(TCCR1B , WGM12) ;
		CLR_BIT(TCCR1A , WGM11) ;
		CLR_BIT(TCCR1A , WGM10) ;

		SET_BIT(TIMSK , TOIE1) ;
		SET_BIT(TCCR1B ,ICNC1) ;
		break ;
	case TIMER1_PWM_PHASE_CORRECT_8BIT  :
		CLR_BIT(TCCR1B , WGM13) ;
		CLR_BIT(TCCR1B , WGM12) ;
		CLR_BIT(TCCR1A , WGM11) ;
		SET_BIT(TCCR1A , WGM10) ;

		SET_BIT(TCCR1A , COM1A1) ;
		CLR_BIT(TCCR1A , COM1A0) ;
		SET_BIT(TCCR1A , COM1B1) ;
		CLR_BIT(TCCR1A , COM1B0) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN4, DIO_OUTPUT) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN5, DIO_OUTPUT) ;
		break ;
	case TIMER1_PWM_PHASE_CORRECT_9BIT  :
		CLR_BIT(TCCR1B , WGM13) ;
		CLR_BIT(TCCR1B , WGM12) ;
		SET_BIT(TCCR1A , WGM11) ;
		CLR_BIT(TCCR1A , WGM10) ;
		SET_BIT(TCCR1A , COM1A1) ;

		CLR_BIT(TCCR1A , COM1A0) ;
		SET_BIT(TCCR1A , COM1B1) ;
		CLR_BIT(TCCR1A , COM1B0) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN4, DIO_OUTPUT) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN5, DIO_OUTPUT) ;
		break ;
	case TIMER1_PWM_PHASE_CORRECT_10BIT :
		CLR_BIT(TCCR1B , WGM13) ;
		CLR_BIT(TCCR1B , WGM12) ;
		SET_BIT(TCCR1A , WGM11) ;
		SET_BIT(TCCR1A , WGM10) ;

		SET_BIT(TCCR1A , COM1A1) ;
		CLR_BIT(TCCR1A , COM1A0) ;
		SET_BIT(TCCR1A , COM1B1) ;
		CLR_BIT(TCCR1A , COM1B0) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN4, DIO_OUTPUT) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN5, DIO_OUTPUT) ;
	case TIMER1_CTC:
	    // This tells the hardware: "Reset to 0 when TCNT1 == OCR1A"
	    CLR_BIT(TCCR1B, WGM13);
	    SET_BIT(TCCR1B, WGM12);
	    CLR_BIT(TCCR1A, WGM11);
	    CLR_BIT(TCCR1A, WGM10);

	    // Only enable the interrupt for 'A' (which we set to 249 in main)
	    SET_BIT(TIMSK, OCIE1A);
	    CLR_BIT(TIMSK, OCIE1B); // Disable 'B' to be safe
	    break;
	case TIMER1_PWM_FAST_8BIT           :
		CLR_BIT(TCCR1B , WGM13) ;
		SET_BIT(TCCR1B , WGM12) ;
		CLR_BIT(TCCR1A , WGM11) ;
		SET_BIT(TCCR1A , WGM10) ;

		SET_BIT(TCCR1A , COM1A1) ;
		CLR_BIT(TCCR1A , COM1A0) ;
		SET_BIT(TCCR1A , COM1B1) ;
		CLR_BIT(TCCR1A , COM1B0) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN4, DIO_OUTPUT) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN5, DIO_OUTPUT) ;
		break ;
	case TIMER1_PWM_FAST_9BIT           :
		CLR_BIT(TCCR1B , WGM13) ;
		SET_BIT(TCCR1B , WGM12) ;
		SET_BIT(TCCR1A , WGM11) ;
		CLR_BIT(TCCR1A , WGM10) ;

		SET_BIT(TCCR1A , COM1A1) ;
		CLR_BIT(TCCR1A , COM1A0) ;
		SET_BIT(TCCR1A , COM1B1) ;
		CLR_BIT(TCCR1A , COM1B0) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN4, DIO_OUTPUT) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN5, DIO_OUTPUT) ;
		break ;
	case TIMER1_PWM_FAST_10BIT          :
		CLR_BIT(TCCR1B , WGM13) ;
		SET_BIT(TCCR1B , WGM12) ;
		SET_BIT(TCCR1A , WGM11) ;
		SET_BIT(TCCR1A , WGM10) ;

		SET_BIT(TCCR1A , COM1A1) ;
		CLR_BIT(TCCR1A , COM1A0) ;
		SET_BIT(TCCR1A , COM1B1) ;
		CLR_BIT(TCCR1A , COM1B0) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN4, DIO_OUTPUT) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN5, DIO_OUTPUT) ;
		break ;
	case TIMER1_FREQ_CORRECT_ICR1_TOP   :
		SET_BIT(TCCR1B , WGM13) ;
		CLR_BIT(TCCR1B , WGM12) ;
		CLR_BIT(TCCR1A , WGM11) ;
		CLR_BIT(TCCR1A , WGM10) ;

		SET_BIT(TCCR1A , COM1A1) ;
		CLR_BIT(TCCR1A , COM1A0) ;
		SET_BIT(TCCR1A , COM1B1) ;
		CLR_BIT(TCCR1A , COM1B0) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN4, DIO_OUTPUT) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN5, DIO_OUTPUT) ;
		break ;
	case TIMER1_FREQ_CORRECT_OCR1A_TOP  :
		SET_BIT(TCCR1B , WGM13) ;
		CLR_BIT(TCCR1B , WGM12) ;
		CLR_BIT(TCCR1A , WGM11) ;
		SET_BIT(TCCR1A , WGM10) ;

		SET_BIT(TCCR1A , COM1A1) ;
		CLR_BIT(TCCR1A , COM1A0) ;
		SET_BIT(TCCR1A , COM1B1) ;
		CLR_BIT(TCCR1A , COM1B0) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN4, DIO_OUTPUT) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN5, DIO_OUTPUT) ;
		break ;
	case TIMER1_PHASE_CORRECT_ICR1_TOP  :
		SET_BIT(TCCR1B , WGM13) ;
		CLR_BIT(TCCR1B , WGM12) ;
		SET_BIT(TCCR1A , WGM11) ;
		CLR_BIT(TCCR1A , WGM10) ;

		SET_BIT(TCCR1A , COM1A1) ;
		CLR_BIT(TCCR1A , COM1A0) ;
		SET_BIT(TCCR1A , COM1B1) ;
		CLR_BIT(TCCR1A , COM1B0) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN4, DIO_OUTPUT) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN5, DIO_OUTPUT) ;
		break ;
	case TIMER1_PHASE_CORRECT_OCR1A_TOP :
		SET_BIT(TCCR1B , WGM13) ;
		CLR_BIT(TCCR1B , WGM12) ;
		SET_BIT(TCCR1A , WGM11) ;
		SET_BIT(TCCR1A , WGM10) ;

		SET_BIT(TCCR1A , COM1A1) ;
		CLR_BIT(TCCR1A , COM1A0) ;
		SET_BIT(TCCR1A , COM1B1) ;
		CLR_BIT(TCCR1A , COM1B0) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN4, DIO_OUTPUT) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN5, DIO_OUTPUT) ;
		break ;
	case TIMER1_CTC_ICR1_TOP            :
		SET_BIT(TCCR1B , WGM13) ;
		SET_BIT(TCCR1B , WGM12) ;
		CLR_BIT(TCCR1A , WGM11) ;
		CLR_BIT(TCCR1A , WGM10) ;

		SET_BIT(TIMSK , OCIE1A) ;
		SET_BIT(TIMSK , OCIE1B) ;
		break ;
	case TIMER1_FAST_PWM_ICR1_TOP       :
		SET_BIT(TCCR1B , WGM13) ;
		SET_BIT(TCCR1B , WGM12) ;
		SET_BIT(TCCR1A , WGM11) ;
		CLR_BIT(TCCR1A , WGM10) ;

		SET_BIT(TCCR1A , COM1A1) ;
		CLR_BIT(TCCR1A , COM1A0) ;
		SET_BIT(TCCR1A , COM1B1) ;
		CLR_BIT(TCCR1A , COM1B0) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN4, DIO_OUTPUT) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN5, DIO_OUTPUT) ;
		break ;
	case TIMER1_FAST_PWM_OCR1A_TOP      :
		SET_BIT(TCCR1B , WGM13) ;
		SET_BIT(TCCR1B , WGM12) ;
		SET_BIT(TCCR1A , WGM11) ;
		SET_BIT(TCCR1A , WGM10) ;

		SET_BIT(TCCR1A , COM1A1) ;
		CLR_BIT(TCCR1A , COM1A0) ;
		SET_BIT(TCCR1A , COM1B1) ;
		CLR_BIT(TCCR1A , COM1B0) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN4, DIO_OUTPUT) ;
		DIO_InitPin(DIO_PORTD, DIO_PIN5, DIO_OUTPUT) ;
		break ;
	}
}

void Timer1_SetPWMA (u16 CompareValue)
{
	OCR1A = CompareValue ;
}
void Timer1_SetPWMB (u16 CompareValue)
{
	OCR1B = CompareValue ;
}

void Timer1_SetDesiredTime (u32 TimeInMicro , void (*fptr)(void))
{
	u32 OverFlowTimeInMicro = 8192 * G_Prescaller ;
	f32 OverFlowCounts = (f32)TimeInMicro/OverFlowTimeInMicro ;
	u32 PreLoadValue ;
	Timer1_ptr = fptr ;

	if (OverFlowCounts < 1.0)
	{
		G_OverFlowCounts = 1 ;
		PreLoadValue = 65536-65536*OverFlowCounts ;
		TCNT1 = PreLoadValue ;
	}
	else if (OverFlowCounts == 1.0)
	{
		G_OverFlowCounts = 1 ;
	}
	else if (OverFlowCounts > 1.0)
	{
		G_OverFlowCounts = OverFlowCounts ;
		if (OverFlowCounts-(u32)OverFlowCounts != 0.0)
		{
			PreLoadValue = 65536-(65536*(OverFlowCounts-(u32)OverFlowCounts)) ;
			G_OverFlowCounts ++ ;
			TCNT1 = PreLoadValue ;
		}
	}
}

u32  Timer1_CalcFreqAtTon (void)
{
	u32 CountsForFistRising = 0 ;
	u32 CountsForFistFailling = 0 ;
	u32 Counts ;
	u32 Freq ;
	/*detect first rising*/
	SET_BIT(TIFR , ICF1) ;
	SET_BIT(TCCR1B , ICES1) ;
	while (READ_BIT(TIFR , ICF1) == 0 );
	CountsForFistRising = ICR1 ;

	/*detect first failing*/
	SET_BIT(TIFR , ICF1) ;
	CLR_BIT(TCCR1B , ICES1) ;
	while (READ_BIT(TIFR , ICF1) == 0 );
	CountsForFistFailling = ICR1 ;
	Counts = CountsForFistFailling - CountsForFistRising ;

	Freq = 8000000/(Counts*G_Prescaller) ;
	return Freq ;

}
/*
void __vector_9 (void)__attribute__((signal)) ;
void __vector_9 (void)
{
	if (Timer1_ptr != 0)
	{
		counter ++ ;
		if (G_OverFlowCounts == counter )
		{
			Timer1_ptr () ;
		}
	}
}
*/
