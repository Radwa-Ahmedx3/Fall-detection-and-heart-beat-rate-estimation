#include "MAX.h"
#include "../../LIB/BIT_MATH.h"
#include "../../LIB/STD_TYPES.h"


#include "../../HAL/LCD/LCD.h"



#include <util/delay.h>
#include <stdlib.h>

#include "../../MCAL/DIO/DIO.h"
#include "../../MCAL/I2C/I2C.h"
#include "../../MCAL/Timer1/Timer1.h"
#include "../../MCAL/USART_Online/USART_Online.h"



void MAX30102_Init(void){
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
u32 MAX30102_ReadIR(void){
    u32 ir = 0;
    u8 b1, b2, b3;

    I2C_SendStartCond();
    I2C_SendAdd(MAX30102_WRITE_ADDR);   // Write mode
    I2C_SendData(0x07);                 // IR register address

    I2C_SendStartCond();                // Repeated start
    I2C_SendAdd(MAX30102_READ_ADDR);    // Read mode

    b1 = I2C_MasterReadAck_max();
    b2 = I2C_MasterReadAck_max();
    b3 = I2C_MasterReadNack_max();
    I2C_SendStopCond();
    _delay_ms(10);

    ir = ((u32)b1 << 16) | ((u32)b2 << 8) | b3;
    ir &= 0x03FFFF;
    return ir;
}

u8 checkForBeat(u32 irValue){
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
f32 removeDC(u32 irValue){
    static f32 dc = 0;
    f32 alpha = 0.95;   // smoothing factor

    dc = alpha * dc + (1 - alpha) * irValue;

    return irValue - dc;  // AC signal (heartbeat part)
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
