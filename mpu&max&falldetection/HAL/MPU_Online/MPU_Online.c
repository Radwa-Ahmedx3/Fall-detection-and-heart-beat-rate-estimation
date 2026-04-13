#include "MPU_Online.h"
#include "../../MCAL/I2C/I2C.h"
#include "../../MCAL/USART_Online/USART_Online.h"

#include "../../LIB/STD_TYPES.h"
#include <util/delay.h>

static s16 Acc_x, Acc_y, Acc_z, Temperature, Gyro_x, Gyro_y, Gyro_z;

#define MPU_WRITE_ADDR 0xD0
#define MPU_READ_ADDR  0xD1

void MPU6050_Online_Init() {

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

}

void Read_RawValue(void) {
	// 1. Start communication
	I2C_SendStartCond();
	I2C_SendAdd(MPU_WRITE_ADDR);

	// 2. Point to the starting register (ACCEL_XOUT_H)
	I2C_SendData(ACCEL_XOUT_H);

	// 3. Repeated Start to switch to Read mode
	I2C_SendStartCond();
	I2C_SendAdd(MPU_READ_ADDR);

	// 4. Read data (High byte then Low byte)

	Acc_x = (s16) ((I2C_MasterReadAck_mpu() << 8) | I2C_MasterReadAck_mpu());
	/*USART_SendString(" -----RAW VALUES------ \r\n");
	USART_SendString("Ax: ");
	USART_SendNumber(Acc_x);*/

	Acc_y = (s16) ((I2C_MasterReadAck_mpu() << 8) | I2C_MasterReadAck_mpu());
	/*USART_SendString(" | Ay: ");
	USART_SendNumber(Acc_y);
*/
	Acc_z = (s16) ((I2C_MasterReadAck_mpu() << 8) | I2C_MasterReadAck_mpu());
	/*USART_SendString(" | Az: \r\n");
	USART_SendNumber(Acc_z);*/


	Temperature = (s16) ((I2C_MasterReadAck_mpu() << 8)
			| I2C_MasterReadAck_mpu());

	Gyro_x = (s16) ((I2C_MasterReadAck_mpu() << 8) | I2C_MasterReadAck_mpu());
	/*USART_SendString("Gx: ");
	USART_SendNumber(Gyro_x);*/


	Gyro_y = (s16) ((I2C_MasterReadAck_mpu() << 8) | I2C_MasterReadAck_mpu());
	/*USART_SendString(" | Gy: ");
	USART_SendNumber(Gyro_y);*/

	Gyro_z = (s16) ((I2C_MasterReadAck_mpu() << 8) | I2C_MasterReadNack_mpu()); // Last byte NACK
	/*USART_SendString(" | Gz: \r\n");
	USART_SendNumber(Gyro_z);
	USART_SendString(" -------------------- \r\n");*/

	// 5. Stop communication
	I2C_SendStopCond();

}

void Read_Accel(f32 *Ax, f32 *Ay, f32 *Az) {
	Read_RawValue();

	*Ax = (f32) Acc_x / 16384.0f;
	*Ay = (f32) Acc_y / 16384.0f;
	*Az = (f32) Acc_z / 16384.0f;
}

void Read_Gyro(f32 *Gx, f32 *Gy, f32 *Gz) {
	Read_RawValue();

	*Gx = (f32) Gyro_x / 131.0f;
	*Gy = (f32) Gyro_y / 131.0f;
	*Gz = (f32) Gyro_z / 131.0f;
}

void Read_Temp(f32 *temp) {
	Read_RawValue();
	*temp = (Temperature / 340.00) + 36.53;
}
