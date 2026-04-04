#include "MPU_6050.h"
#include "../../MCAL/I2C/I2C.h"
#include "../../LIB/BIT_MATH.h"

/* Internal Functions */
static void MPU6050_Write(uint8 Reg, uint8 Data);
static void MPU6050_ReadMulti(uint8 Reg, uint8 *Buffer, uint8 Size);

/* =============================== */

void MPU6050_Init(void)
{
    /* Wake up sensor */
    MPU6050_Write(MPU6050_PWR_MGMT_1, 0x00);
}

/* =============================== */

void MPU6050_ReadAccel(f32 *Ax, f32 *Ay, f32 *Az)
{
    uint8 Data[6];
    sint16 RawX, RawY, RawZ;

    MPU6050_ReadMulti(MPU6050_ACCEL_XOUT_H, Data, 6);

    RawX = (Data[0] << 8) | Data[1];
    RawY = (Data[2] << 8) | Data[3];
    RawZ = (Data[4] << 8) | Data[5];

    *Ax = RawX / 16384.0;
    *Ay = RawY / 16384.0;
    *Az = RawZ / 16384.0;
}

/* =============================== */

void MPU6050_ReadGyro(f32 *Gx, f32 *Gy, f32 *Gz)
{
    uint8 Data[6];
    sint16 RawX, RawY, RawZ;

    MPU6050_ReadMulti(MPU6050_GYRO_XOUT_H, Data, 6);

    RawX = (Data[0] << 8) | Data[1];
    RawY = (Data[2] << 8) | Data[3];
    RawZ = (Data[4] << 8) | Data[5];

    *Gx = RawX / 131.0;
    *Gy = RawY / 131.0;
    *Gz = RawZ / 131.0;
}

/* =============================== */

static void MPU6050_Write(uint8 Reg, uint8 Data)
{
    I2C_SendStartCond();
    I2C_SendAdd(MPU6050_ADDRESS << 1);
    I2C_SendData(Reg);
    I2C_SendData(Data);
    I2C_SendStopCond();
}

/* =============================== */

static void MPU6050_ReadMulti(uint8 Reg, uint8 *Buffer, uint8 Size)
{
    uint8 i;

    I2C_SendStartCond();
    I2C_SendAdd((MPU6050_ADDRESS << 1) | 0);
    I2C_SendData(Reg);

    I2C_SendStartCond();
    I2C_SendAdd((MPU6050_ADDRESS << 1) | 1);

    for(i = 0; i < Size; i++)
    {
    	if(i == (Size - 1))
    	{
    		Buffer[i] = I2C_MasterReadNack();
    	}
    	else
    	{
    		Buffer[i] = I2C_MasterReadAck();
    	}
    }

    I2C_SendStopCond();
}
