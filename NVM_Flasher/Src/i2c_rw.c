
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"


I2C_HandleTypeDef *hi2c[2];
unsigned int I2cDeviceID_7bit;
unsigned int AddressSize = I2C_MEMADD_SIZE_8BIT;



/* USER CODE BEGIN 1 */

HAL_StatusTypeDef I2C_Read_USB_PD(uint8_t Port, uint16_t Address ,uint8_t *DataR ,uint16_t Length)
{
  return  HAL_I2C_Mem_Read(hi2c[Port],(I2cDeviceID_7bit<<1), Address ,AddressSize, DataR, Length ,2000);
}


HAL_StatusTypeDef I2C_Write_USB_PD(uint8_t Port, uint16_t Address ,uint8_t *DataW ,uint16_t Length)
{
  return  HAL_I2C_Mem_Write(hi2c[Port],(I2cDeviceID_7bit<<1), Address ,AddressSize, DataW, Length, 2000 ); // unmask all alarm status
}

/* USER CODE END 1 */
