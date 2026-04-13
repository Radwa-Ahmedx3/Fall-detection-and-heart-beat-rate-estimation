
#ifndef MCAL_TIMER1_TIMER1_H_
#define MCAL_TIMER1_TIMER1_H_

#define TCCR1A   (*(volatile u8*)(0x4F))
#define TCCR1B   (*(volatile u8*)(0x4E))
#define TCNT1    (*(volatile u16*)(0x4C))
#define OCR1A    (*(volatile u16*)(0x4A))
#define OCR1B    (*(volatile u16*)(0x48))
#define ICR1     (*(volatile u16*)(0x46))
#define TIMSK    (*(volatile u8*)(0x59))
#define TIFR     (*(volatile u8*)(0x58))
#define SREG     (*(volatile u8*)(0x5F))

#define COM1A1  7
#define COM1A0  6
#define COM1B1  5
#define COM1B0  4
#define FOC1A   3
#define FOC1B   2
#define WGM11   1
#define WGM10   0

#define ICNC1   7
#define ICES1   6
#define WGM13   4
#define WGM12   3
#define CS12    2
#define CS11    1
#define CS10    0

#define TICIE1  5
#define OCIE1A  4
#define OCIE1B  3
#define TOIE1   2

#define ICF1    5
#define OCF1A   4
#define OCF1B   3
#define TOV1    2

#define I_BIT   7


#define TIMER1_NO_PRESCALLER   1
#define TIMER1_PRESCALLER8     8
#define TIMER1_PRESCALLER64    64
#define TIMER1_PRESCALLER256   256
#define TIMER1_PRESCALLER1024  1024

#define TIMER1_NORMAL                    1
#define TIMER1_PWM_PHASE_CORRECT_8BIT    2
#define TIMER1_PWM_PHASE_CORRECT_9BIT    3
#define TIMER1_PWM_PHASE_CORRECT_10BIT   4
#define TIMER1_CTC                       5
#define TIMER1_PWM_FAST_8BIT             6
#define TIMER1_PWM_FAST_9BIT             7
#define TIMER1_PWM_FAST_10BIT            8
#define TIMER1_FREQ_CORRECT_ICR1_TOP     9
#define TIMER1_FREQ_CORRECT_OCR1A_TOP    10
#define TIMER1_PHASE_CORRECT_ICR1_TOP    11
#define TIMER1_PHASE_CORRECT_OCR1A_TOP   12
#define TIMER1_CTC_ICR1_TOP              13
#define TIMER1_FAST_PWM_ICR1_TOP         14
#define TIMER1_FAST_PWM_OCR1A_TOP        15

void Timer1_Init (u32 Prescaller , u8 Mode) ;
void Timer1_SetPWMA (u16 CompareValue) ;
void Timer1_SetPWMB (u16 CompareValue) ;
void Timer1_SetDesiredTime (u32 TimeInMicro , void (*fptr)(void)) ;
u32  Timer1_CalcFreqAtTon (void) ;
#endif /* MCAL_TIMER1_TIMER1_H_ */
