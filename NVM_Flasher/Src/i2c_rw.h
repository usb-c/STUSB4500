
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "stm32f0xx_hal.h"
//#include "main.h"


/* USER CODE BEGIN Prototypes */
HAL_StatusTypeDef I2C_Read_USB_PD(uint8_t Port, uint16_t Address ,uint8_t *DataR ,uint16_t Length);
HAL_StatusTypeDef I2C_Write_USB_PD(uint8_t Port, uint16_t Address ,uint8_t *DataW ,uint16_t Length);
/* USER CODE END Prototypes */


#ifdef __cplusplus
}
#endif
