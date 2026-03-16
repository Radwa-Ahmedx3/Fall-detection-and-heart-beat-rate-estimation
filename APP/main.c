
#define F_CPU 16000000UL
#include "../LIB/BIT_MATH.h"
#include "../LIB/STD_TYPES.h"
#include "../HAL/LED/LED.h"


#include "../HAL/LCD/LCD.h"
#include "../MCAL/DIO/DIO.h"
//#include "../MCAL/EXTI/EXTI.h"
//#include "../MCAL/ADC/ADC.h"
#include <util/delay.h>
#include <stdlib.h>
#include "../MCAL/I2C/I2C.h"
#include "../MCAL/Timer1/Timer1.h"
#include "../MCAL/USART_Online/USART_Online.h"

//#define BEAT_THRESHOLD_HIGH  40000  // adjust according to your readings
//#define BEAT_THRESHOLD_LOW   30000
#define BEAT_THRESHOLD_HIGH 140000
#define BEAT_THRESHOLD_LOW  100000

#define MAX30102_ADDR        0x57
#define MAX30102_WRITE_ADDR  ((MAX30102_ADDR << 1) | 0)  // 0xAE
#define MAX30102_READ_ADDR   ((MAX30102_ADDR << 1) | 1)  // 0xAF
#define RATE_SIZE 10

u8 rates[RATE_SIZE];
u8 rateSpot = 0;
u32 lastBeat = 0;
float beatsPerMinute;
u8 beatAvg;
f32 removeDC(u32 irValue);
void MAX30102_Init(void);
u32 MAX30102_ReadIR(void);
u8 checkForBeat(u32 irValue);

volatile u32 g_millis = 0;

void Millis_Increment(void)
{
    g_millis++;
}

u32 millis(void)
{
    return g_millis;
}




void update_display(u8 avgBPM, u32 rawIR) {
    LCD_movecursor(0,0);
    LCD_writestr((u8*)"BPM: ");
    char buffer[5];
    itoa(avgBPM, buffer, 10);
    LCD_writestr((u8*)buffer);
    LCD_writestr((u8*)"   ");

    LCD_movecursor(0,1);

    USART_SendString("Raw IR: ");
    USART_SendNumber(rawIR);
    USART_SendString(" | State: ");

    // FIX: Check Raw IR to see if a finger is actually there
    if (rawIR < 50000) {
        LCD_writestr((u8*)"Place Finger    ");
        USART_SendString("No Finger Detected\r\n");
    }
    else {
        // If finger is there, show the heart rate status
        if (avgBPM == 0) {
            LCD_writestr((u8*)"Processing...   ");
            USART_SendString("Calculating BPM...\r\n");
        }
        else if (avgBPM < 60) {
            LCD_writestr((u8*)"Low Heart Rate  ");
            USART_SendString("Low Heart Rate\r\n");
        }
        else if (avgBPM <= 105) {
            LCD_writestr((u8*)"Normal Rate     ");
            USART_SendString("Normal Heart Rate\r\n");
        }
        else {
            LCD_writestr((u8*)"High Heart Rate ");
            USART_SendString("High Heart Rate\r\n");
        }
    }
    USART_SendString("--------------------\r\n");
}

// This stays at the top level, outside any functions
void __vector_7 (void) __attribute__((signal));
void __vector_7 (void)
{
    g_millis++; // Direct 1ms increment
}

int main(void)
{
	// --- 1. Hardware Initializations ---
	LCD_init();
	LCD_clear();
	USART_Init(9600);
	I2C_Masterinit(100000);
	MAX30102_Init();

	Timer1_Init(TIMER1_PRESCALLER64, TIMER1_CTC);
		OCR1A = 249;        // Sets the 1ms interval for 16MHz clock
		SET_BIT(TIMSK, 4);  // Enable Compare Match A interrupt
		SET_BIT(SREG, 7);   // Enable Global Interrupts

	u16 lcdTimer = 0;
	u32 currentIR = 0;

	USART_SendString("System Initialized. Place Finger...\r\n");
	while(1)
	{
	    currentIR = MAX30102_ReadIR();
	    u32 now = millis();

	    // If no beat has been detected for 3 seconds, reset the data
	    if (now - lastBeat > 3000 && lastBeat != 0)
	    {
	        beatAvg = 0;
	        lastBeat = 0;
	        rateSpot = 0;
	        for(u8 i = 0; i < RATE_SIZE; i++) rates[i] = 0;
	        USART_SendString("Resetting due to inactivity...\r\n");
	    }

	    if(checkForBeat(currentIR))
	    {


	        if (lastBeat == 0)
	        {
	            lastBeat = now;
	        }
	        else
	        {
	            u32 delta = now - lastBeat;
	            lastBeat = now; // Always update lastBeat to the most recent pulse

	            USART_SendString("Delta: ");
	            USART_SendNumber(delta);
	            USART_SendString(" ms\r\n");
	            if(delta > 400 && delta < 1500)
	                {
	                  beatsPerMinute = 60000.0 / (float)delta;

	                  if (beatAvg == 0) {
	                       for(u8 i = 0; i < RATE_SIZE; i++) rates[i] = (u8)beatsPerMinute;
	                    }

	                   rates[rateSpot++] = (u8)beatsPerMinute;
	                   rateSpot %= RATE_SIZE;

	                    u16 sum = 0;
	                    for(u8 i = 0; i < RATE_SIZE; i++) sum += rates[i];
	                    beatAvg = (u8)(sum / RATE_SIZE);

	             USART_SendString("BPM Detected: ");
	             USART_SendNumber(beatAvg);
	             USART_SendString("\r\n");
	              }
	            }
	          }

	                // 3. Optimized Display (Every 250ms instead of 500ms)
	                if(++lcdTimer >= 25)
	                {
	                    update_display(beatAvg, currentIR);
	                    lcdTimer = 0;
	                }

	                _delay_ms(10);}}

void MAX30102_Init(void)
{

	//USART_SendString("before beg\r\n");
    I2C_SendStartCond();
    I2C_SendAdd(MAX30102_WRITE_ADDR);
    I2C_SendData(0x09);
    I2C_SendData(0x40);
    I2C_SendStopCond();
    _delay_ms(100);


    I2C_SendStartCond();
    I2C_SendAdd(MAX30102_WRITE_ADDR);
    I2C_SendData(0x09);
    I2C_SendData(0x02);
    I2C_SendStopCond();

    I2C_SendStartCond();
    I2C_SendAdd(MAX30102_WRITE_ADDR);
    I2C_SendData(0x0C);
    I2C_SendData(0x10);
    I2C_SendStopCond();

    I2C_SendStartCond();
    I2C_SendAdd(MAX30102_WRITE_ADDR);
    I2C_SendData(0x0D);
    I2C_SendData(0x1F);
    I2C_SendStopCond();
}
u32 MAX30102_ReadIR(void)
{
    u32 ir = 0;
    u8 b1, b2, b3;

    I2C_SendStartCond();
    I2C_SendAdd(MAX30102_WRITE_ADDR);   // Write mode
    I2C_SendData(0x07);                 // IR register address

    I2C_SendStartCond();                // Repeated start
    I2C_SendAdd(MAX30102_READ_ADDR);    // Read mode

    b1 = I2C_MasterReadAck();
    b2 = I2C_MasterReadAck();
    b3 = I2C_MasterReadNack();
    I2C_SendStopCond();

    ir = ((u32)b1 << 16) | ((u32)b2 << 8) | b3;
    ir &= 0x03FFFF;
    return ir;
}

u8 checkForBeat(u32 irValue) {
    static float lowPassValue = 0;
    static float dcBaseline = 0;
    static u8 beatDetected = 0;

    // 1. Auto-initialize on first run
    if (lowPassValue == 0) {
        lowPassValue = (float)irValue;
        dcBaseline = (float)irValue;
        return 0;
    }

    // 2. Filter logic (Slightly faster tracking)
    lowPassValue = (0.2 * irValue) + (0.8 * lowPassValue);
    dcBaseline = (0.98 * dcBaseline) + (0.02 * lowPassValue);
    float acValue = lowPassValue - dcBaseline;

    // 3. Ultra-sensitive Threshold (Try 12 instead of 20)
    if (acValue > 12 && beatDetected == 0) {
        beatDetected = 1;
        return 1;
    }
    else if (acValue < -2) { // Reset threshold
        beatDetected = 0;
    }
    return 0;
}
f32 removeDC(u32 irValue)
{
    static f32 dc = 0;
    f32 alpha = 0.95;   // smoothing factor

    dc = alpha * dc + (1 - alpha) * irValue;

    return irValue - dc;  // AC signal (heartbeat part)
}
