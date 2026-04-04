

#ifndef HAL_MPU6050_MPU6050_H_
#define HAL_MPU6050_MPU6050_H_

#include "../../LIB/STD_TYPES.h"

/* Slave Address */
#define MPU6050_ADDRESS 0x68

/* Registers */
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_GYRO_XOUT_H 0x43

/* Functions */
void MPU6050_Init(void);
void MPU6050_ReadAccel(f32 *Ax, f32 *Ay, f32 *Az);
void MPU6050_ReadGyro(f32 *Gx, f32 *Gy, f32 *Gz);

#endif
