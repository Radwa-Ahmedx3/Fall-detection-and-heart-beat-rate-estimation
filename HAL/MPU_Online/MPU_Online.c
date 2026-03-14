
#include "MPU_Online.h"
#include "../../MCAL/I2C/I2C.h"
#include "../../MCAL/USART_Online/USART_Online.h"

#include "../../LIB/STD_TYPES.h"
#include <util/delay.h>

float Acc_x, Acc_y, Acc_z, Temperature, Gyro_x, Gyro_y, Gyro_z;


#define MPU_WRITE_ADDR 0xD0
#define MPU_READ_ADDR  0xD1

void MPU6050_Online_Init() {
    USART_SendString("Entering MPU6050_WakeupOnly()\r\n");
    _delay_ms(150);

    // 1. Send Start Condition
    I2C_SendStartCond();

    // 2. Send Slave Address with Write bit
    I2C_SendAdd(MPU_WRITE_ADDR);

    // 3. Write to Power Management Register to wake up
    I2C_SendData(PWR_MGMT_1);

    // 4. Send 0 to wake it up
    I2C_SendData(0x00);

    // 5. Send Stop Condition
    I2C_SendStopCond();

    USART_SendString("MPU Woke up!\r\n");
}

void Read_RawValue() {
    // 1. Start communication
    I2C_SendStartCond();
    I2C_SendAdd(MPU_WRITE_ADDR);

    // 2. Point to the starting register (ACCEL_XOUT_H)
    I2C_SendData(ACCEL_XOUT_H);

    // 3. Repeated Start to switch to Read mode
    I2C_SendStartCond();
    I2C_SendAdd(MPU_READ_ADDR);

    // 4. Read data (High byte then Low byte)

    Acc_x = (float)((I2C_MasterReadAck() << 8) | I2C_MasterReadAck());
    Acc_y = (float)((I2C_MasterReadAck() << 8) | I2C_MasterReadAck());
    Acc_z = (float)((I2C_MasterReadAck() << 8) | I2C_MasterReadAck());

    Temperature = (float)((I2C_MasterReadAck() << 8) | I2C_MasterReadAck());

    Gyro_x = (float)((I2C_MasterReadAck() << 8) | I2C_MasterReadAck());
    Gyro_y = (float)((I2C_MasterReadAck() << 8) | I2C_MasterReadAck());
    Gyro_z = (float)((I2C_MasterReadAck() << 8) | I2C_MasterReadNack()); // Last byte NACK

    // 5. Stop communication
    I2C_SendStopCond();
}

void Read_Accel(f32 *Ax, f32 *Ay, f32 *Az) {
    Read_RawValue();

    *Ax = Acc_x / 16384.0;
    *Ay = Acc_y / 16384.0;
    *Az = Acc_z / 16384.0;
}

void Read_Gyro(f32 *Gx, f32 *Gy, f32 *Gz) {
    Read_RawValue();

    *Gx = Gyro_x / 131.0;
    *Gy = Gyro_y / 131.0;
    *Gz = Gyro_z / 131.0;
}

void Read_Temp(f32 *temp) {
    Read_RawValue();
    *temp = (Temperature / 340.00) + 36.53;
}
