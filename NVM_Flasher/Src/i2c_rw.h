
#ifdef __cplusplus
 extern "C" {
#endif

/* Defines ------------------------------------------------------------------*/
#define STUSBxx_DEVICEID_7BIT  0x28
#define STUSBxx_DEVICEID_8BIT  0x50
#define STUSB4500_I2C_DEVID_7BIT  0x28
#define STUSB4500_I2C_DEVID_8BIT  0x50



/* USER CODE BEGIN Prototypes */
HAL_StatusTypeDef I2C_Read_USB_PD(uint8_t Port, uint16_t Address ,uint8_t *DataR ,uint16_t Length);
HAL_StatusTypeDef I2C_Write_USB_PD(uint8_t Port, uint16_t Address ,uint8_t *DataW ,uint16_t Length);
/* USER CODE END Prototypes */


#ifdef __cplusplus
}
#endif
