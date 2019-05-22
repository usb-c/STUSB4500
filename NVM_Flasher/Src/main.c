/**
******************************************************************************
* File Name          : main.c
* Description        : Main program body
******************************************************************************
** This notice applies to any and all portions of this file
* that are not between comment pairs USER CODE BEGIN and
* USER CODE END. Other portions of this file, whether 
* inserted by the user or by software development tools
* are owned by their respective copyright owners.
*
* COPYRIGHT(c) 2017 STMicroelectronics
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright notice,
*      this list of conditions and the following disclaimer in the documentation
*      and/or other materials provided with the distribution.
*   3. Neither the name of STMicroelectronics nor the names of its contributors
*      may be used to endorse or promote products derived from this software
*      without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx_hal.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "USB_PD_defines.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef *hi2c[2];	
unsigned int I2cDeviceID_7bit;
unsigned int Address;
unsigned int AddressSize = I2C_MEMADD_SIZE_8BIT;

//UART_HandleTypeDef huart2;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void _Error_Handler(char * file, int line);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
extern uint8_t nvm_flash(uint8_t Port);
extern uint8_t CUST_EnterWriteMode(uint8_t Port,unsigned char ErasedSector);
extern uint8_t CUST_EnterReadMode(uint8_t Port);
extern uint8_t CUST_ReadSector(uint8_t Port,char SectorNum, unsigned char *SectorData);
extern uint8_t CUST_WriteSector(uint8_t Port,char SectorNum, unsigned char *SectorData);
extern uint8_t CUST_ExitTestMode(uint8_t Port);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
extern uint8_t Sector0[8];
extern uint8_t Sector1[8];
extern uint8_t Sector2[8];
extern uint8_t Sector3[8];
extern uint8_t Sector4[8];

/* USER CODE END 0 */

int main(void)
{
  
  /* USER CODE BEGIN 1 */
  uint8_t Buffer;
  uint8_t j;
  uint8_t Verification_Sector0[8];
  uint8_t Verification_Sector1[8];
  uint8_t Verification_Sector2[8];
  uint8_t Verification_Sector3[8];
  uint8_t Verification_Sector4[8];
  //uint64_t toto;
  /* USER CODE END 1 */
  
  /* MCU Configuration----------------------------------------------------------*/
  
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  
  /* USER CODE BEGIN Init */
  
  /* USER CODE END Init */
  
  /* Configure the system clock */
  SystemClock_Config();
  
  /* USER CODE BEGIN SysInit */
  
  /* USER CODE END SysInit */
  
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_USART2_UART_Init();
  
  /* USER CODE BEGIN 2 */
  hi2c[0]= &hi2c1;
  hi2c[1]= &hi2c2;
  I2cDeviceID_7bit = 0x28;
  AddressSize = I2C_MEMADD_SIZE_8BIT; 
  HAL_GPIO_WritePin(Reset_GPIO_Port,Reset_Pin,GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(Reset_GPIO_Port,Reset_Pin,GPIO_PIN_RESET);
  HAL_Delay(200);
  
  I2C_Read_USB_PD(0,DEVICE_ID,&Buffer,1); 
  // DEV_CUT 0 : 1600 , 1 Reserved , 2 : 1602 , 3 Autorun 
  
  if(Buffer & ID_Reg >= CUT)  
  {
    if ( nvm_flash(0) == 1) 
      printf("STUSB Flashing Failed \r\n"); // Port 0 mean I2C1 For I2C2 change to Port 1
    else  printf("STUSB Flashing Done\r\n");
  }
  else  printf("STUSB Flashing Not done : Bad version\r\n");
  if (CUST_EnterReadMode(0) == 0 )
  {
    if (CUST_ReadSector(0,0, &Verification_Sector0[0]) ==0)
      
      if (  *(uint64_t *) Verification_Sector0 != *(uint64_t *)Sector0   )
      {
        for ( j = 0 ;j < 8 ; j++)
          if (  Verification_Sector0[j] != Sector0[j])
            printf("NVM verification issue byte %i bank 0\r\n",j);
        
      } else   printf("NVM verification bank 0 : OK \r\n");
    else  printf("NVM read bank 0 : failed \r\n");
    if (CUST_ReadSector(0,1, &Verification_Sector1[0]) ==0)
       if (  *(uint64_t *) Verification_Sector1 != *(uint64_t *)Sector1   )
      {
        for ( j = 0 ;j < 8 ; j++) 
          if (  Verification_Sector1[j] != Sector1[j])
            printf("NVM verification issue byte %i bank 1\r\n",j);
        
      } else   printf("NVM verification bank 1 : OK \r\n");
    else  printf("NVM read bank 1 : failed \r\n");
    if (CUST_ReadSector(0,2, &Verification_Sector2[0]) ==0)
     if (  *(uint64_t *) Verification_Sector2 != *(uint64_t *)Sector2   )
      {
      for ( j = 0 ;j < 8 ; j++)
      if (  Verification_Sector2[j] != Sector2[j])
        printf("NVM verification issue byte %i bank 2\r\n",j);
     }  else   printf("NVM verification bank 2 : OK \r\n");
    else  printf("NVM read bank 2 : failed \r\n");
    if (CUST_ReadSector(0,3, &Verification_Sector3[0]) ==0)
     if (  *(uint64_t *) Verification_Sector3 != *(uint64_t *)Sector3   )
      {
      for ( j = 0 ;j < 8 ; j++)
      if (  Verification_Sector3[j] != Sector3[j])
        printf("NVM verification issue byte %i bank 3\r\n",j);    
    }  else   printf("NVM verification bank 3 : OK \r\n");
    else  printf("NVM read bank 3 : failed \r\n");
    if (CUST_ReadSector(0,4, &Verification_Sector4[0]) ==0)
     if (  *(uint64_t *) Verification_Sector4 != *(uint64_t *)Sector4   )
      {
      for ( j = 0 ;j < 8 ; j++)
      if (  Verification_Sector4[j] != Sector4[j])
        printf("NVM verification issue byte %i bank 4\r\n",j);
      
    }  else   printf("NVM verification bank 4 : OK \r\n");
    else  printf("NVM read bank 4 : failed \r\n");
  }
  else  printf("NVM read initialisation: failed \r\n");
  
  /* USER CODE END 2 */
  

/* Infinite loop */
/* USER CODE BEGIN WHILE */
while (1)
{
  /* USER CODE END WHILE */
  
  /* USER CODE BEGIN 3 */
  
}
/* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{
  
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;
  
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
    |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  
  /**Configure the Systick interrupt time 
  */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
  
  /**Configure the Systick 
  */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
  
  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
* @brief  This function is executed in case of error occurrence.
* @param  None
* @retval None
*/
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT

/**
* @brief Reports the name of the source file and the source line number
* where the assert_param error has occurred.
* @param file: pointer to the source file name
* @param line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
  
}

#endif

/**
* @}
*/ 

/**
* @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
