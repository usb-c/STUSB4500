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
* COPYRIGHT(c) 2018 STMicroelectronics
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
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */

#include "USB_PD_core.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef *hi2c[I2CBUS_MAX];
unsigned int Address;
unsigned int AddressSize = I2C_MEMADD_SIZE_8BIT;
uint8_t Buffer[12];
USB_PD_I2C_PORT STUSB45DeviceConf[USBPORT_MAX];
uint32_t timer_cnt = 0;
int Flag_count = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void push_button_Action(void);
extern void nvm_flash(uint8_t Usb_Port);
STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef Nego_RDO;
int PB_press=0;
int Time_elapse=1;
uint8_t USB_PD_Interupt_Flag[USBPORT_MAX] ;
uint8_t push_button_Action_Flag[USBPORT_MAX];
uint8_t Timer_Action_Flag[USBPORT_MAX];
extern USB_PD_StatusTypeDef PD_status[USBPORT_MAX] ;
extern USB_PD_CTRLTypeDef   PD_ctrl[USBPORT_MAX];
extern USB_PD_SNK_PDO_TypeDef PDO_SNK[USBPORT_MAX][3];
extern uint8_t PDO_SNK_NUMB[USBPORT_MAX];
uint8_t flag_once=1;

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
  {
    
    /* USER CODE BEGIN 1 */
    uint8_t Cut[USBPORT_MAX];
    int Usb_Port = 0;
    int Status;
    uint8_t Prev_Connection_status;
    
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
    //    MX_DMA_Init();
    MX_I2C1_Init();
#ifdef PRINTF
    MX_USART2_UART_Init();
#endif 
    /* USER CODE BEGIN 2 */
    hi2c[0]= &hi2c1;
    //  hi2c[1]= &hi2c2;
    STUSB45DeviceConf[Usb_Port].I2cBus = Usb_Port;
    STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit = 0x28;
    AddressSize = I2C_MEMADD_SIZE_8BIT; 
 
    
    Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit,DEVICE_ID ,&Cut[Usb_Port], 1 );
#ifdef PRINTF
    if (Cut[Usb_Port] == 0x21 )
      printf(" STUSB45 detected 0x%02x\r\n",Buffer[0]);
    else printf(" STUSB45 Not detected\r\n");
#endif
    HW_Reset_state(Usb_Port);
    Print_PDO_FROM_SRC(Usb_Port);
    Update_PDO(Usb_Port,2,15000,1500);
    Update_PDO(Usb_Port,3,20000,1500);    
    Read_SNK_PDO(Usb_Port);
    Time_elapse = PDO_SNK_NUMB[Usb_Port];
    Print_SNK_PDO(Usb_Port);
    Prev_Connection_status = 0;
    push_button_Action_Flag[Usb_Port]=0;
    /* USER CODE END 2 */
    
    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
      {
        /* USER CODE END WHILE */
        
        /* USER CODE BEGIN 3 */
        
        
        if( push_button_Action_Flag[Usb_Port] == 1  )
          push_button_Action();  
    // UNCOMMENT the bellow 2 lines in order to enable the STUSB4500_PDO_rolling_DEMO
    //    if( Timer_Action_Flag[Usb_Port] == 1  )
    //      Timer_Action();          
        if(USB_PD_Interupt_Flag[Usb_Port] != 1)
          {
            
            if ( flag_once == 1)
              {
                if(PD_status[Usb_Port].Port_Status.b.CC_ATTACH_STATE  !=0)
                  {
                    I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,0x29 ,&Buffer[0], 1 );
                    if (( Buffer[0] == 0x18) )
                      {
                        flag_once = 0;
                        Prev_Connection_status = 0;
                        Print_SNK_PDO(Usb_Port);
                        //  Print_PDO_FROM_SRC(Usb_Port);
                        Read_RDO(Usb_Port);
                        Print_RDO(Usb_Port);
                      }
                  }
                
              }
          }
        if ( HAL_GPIO_ReadPin(ALERT_A_GPIO_Port,ALERT_A_Pin)== GPIO_PIN_RESET)
          ALARM_MANAGEMENT(Usb_Port); 
        
        
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
/**
* @brief  EXTI line detection callback.
* @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line.
* @retval None
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
  {
    int Usb_Port = 0;
    switch(GPIO_Pin)
      {
        
      case ( ALERT_A_Pin)://B1_Pin 
        { 
          USB_PD_Interupt_Flag[Usb_Port] = 1;
        } break;
        
        // SELECT HERE WHICH function must be attached to the BLUE push button
      case ( B1_Pin)://B1_Pin 
        { 
          push_button_Action_Flag[Usb_Port] = 1 ;
          PB_press ++ ;
        } break;
      }
  }

void push_button_Action(void)
  {
    uint8_t Usb_Port = 0;
#ifdef PRINTF
    printf("Push button \r\n");
#endif
    /*update PDO num to 1 then Reset */
    switch (PB_press%5)  /* line for 4 state */
//    switch (PB_press%4) 
      {
      case 0:
        
        break ;
      case 1 : 
        {
          flag_once = 1;
          Set_New_PDO_case1(Usb_Port); /*set PDO2 15V 1.5A , PDO3 20V 1.5A*/
          SW_reset_by_Reg(Usb_Port);
        }
        break;
      case 2 : 
        {
          flag_once = 1;
          Negotiate_5V(Usb_Port); /* set Number of PDO to 1 */
          SW_reset_by_Reg(Usb_Port);
        }
        break;
      case 3 : 
        {
          flag_once = 1;
          if (! Find_Matching_SRC_PDO(Usb_Port,15,14000,20000))
                  SW_reset_by_Reg(Usb_Port);
          
        }
        break;  
      case  4: 
        {
          HW_Reset_state(0);
          PB_press = 0 ;
          Time_elapse = 3;
        }
        break;
      default :
       
        break;
      }
    
    push_button_Action_Flag[Usb_Port] = 0 ;
  }

void Timer_Action(void)
  {
    STUSB_GEN1S_ALERT_STATUS_RegTypeDef Alert_Mask;
    int Usb_Port = 0;
    int i =0;
    int Status;

//    if (PD_status[Usb_Port].Port_status != 0)
    if(PB_press == 0 )
      {
#ifdef PRINTF    
    printf("\r\n--- Timer Action ---\r\n");	
#endif    
        switch(Time_elapse )
          {
          case 1 :
            {
              
              /*update PDO num to 1 then Reset */
#ifdef PRINTF          
              printf(" STUSB45 Reset(1) \r\n");
#endif
              Update_Valid_PDO_Number( Usb_Port , 1 );              
              if (Flag_count)
                Time_elapse ++ ;
              else 
                Time_elapse -- ;  
            } break;      
          case 2 :
            {
#ifdef PRINTF          
              printf(" STUSB45 Reset(2) \r\n");
#endif          /*update PDO num to 1 then Reset */
              Update_Valid_PDO_Number( Usb_Port , 2 );
              
              if (Flag_count)
                Time_elapse ++ ;
              else 
                Time_elapse -- ;  
            } break;  
          case 3 :
            {
#ifdef PRINTF          
              printf(" STUSB45 Reset(3) \r\n");
#endif          /*update PDO num to 1 then Reset */
                Update_PDO(Usb_Port,2,9000,1500);
                Update_PDO(Usb_Port,3,12000,1500);             

              Update_Valid_PDO_Number( Usb_Port , 3 );
              if (Flag_count)
                Time_elapse ++ ;
              else 
                Time_elapse -- ;  
            } break;  

          case 4 :
            {
#ifdef PRINTF          
              printf(" STUSB45 Reset(4) \r\n");
#endif          /*update PDO num to 1 then Reset */
                Update_PDO(Usb_Port,2,15000,1500);
                Update_PDO(Usb_Port,3,20000,1500);             
              Update_Valid_PDO_Number( Usb_Port , 2 );
              if (Flag_count)
                Time_elapse ++ ;
              else 
                Time_elapse -- ;  
            } break;  
 
          case 5 :
            {
#ifdef PRINTF          
              printf(" STUSB45 Reset(5) \r\n");
#endif          /*update PDO num to 1 then Reset */
              Update_PDO(Usb_Port,2,15000,1500);
              Update_PDO(Usb_Port,3,20000,1500);            
              Update_Valid_PDO_Number( Usb_Port , 3 );
              if (Flag_count)
                Time_elapse ++ ;
              else 
                Time_elapse -- ;  
            } break;  
 
          default :
            Time_elapse = 1;
            break;            
          }
        flag_once = 1;
        SW_reset_by_Reg(Usb_Port);
        Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,PORT_STATUS ,&PD_status[Usb_Port].Port_Status.d8, 1 );
        if ( Time_elapse == 1)
          Flag_count = 1 ;
        if ( Time_elapse == 5)
          Flag_count = 0 ;
      }
    Timer_Action_Flag[Usb_Port] = 0 ;
  }





/* USER CODE END 4 */

/**
* @brief  Period elapsed callback in non blocking mode
* @note   This function is called  when TIM1 interrupt took place, inside
* HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
* a global variable "uwTick" used as application time base.
* @param  htim : TIM handle
* @retval None
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
  {
    uint8_t Usb_Port = 0;
    /* USER CODE BEGIN Callback 0 */
    
    /* USER CODE END Callback 0 */
    if (htim->Instance == TIM1) {
      HAL_IncTick();
    }
    /* USER CODE BEGIN Callback 1 */
    if (PD_status[Usb_Port].Port_Status.b.CC_ATTACH_STATE != 0)
      {
        timer_cnt ++ ;
        if (timer_cnt == 5000)
          {
            Timer_Action_Flag[0] = 1;
            timer_cnt = 0;
          }
      }
    /* USER CODE END Callback 1 */
  }

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
