
/* Defines ------------------------------------------------------------------*/
#define READ_NVM 1   //0:false, 1:true
#define FLASH_NVM 1   //0:false, 1:true
#define VERIFY_NVM 1   //0:false, 1:true


/* Includes ------------------------------------------------------------------*/
#include "i2c.h"
#include <stdio.h> //for printf()

/* USER CODE BEGIN Includes */
#include "i2c_rw.h"
#include "USB_PD_defines.h"
#include "USBPD_CUST_NVM_API.h"
/* USER CODE END Includes */


/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
extern I2C_HandleTypeDef *hi2c[2];	
extern unsigned int I2cDeviceID_7bit;
/* USER CODE END PV */


int main_NVM(void)
{
    
    /* USER CODE BEGIN 2 */
    int status;
    
    
    hi2c[0]= &hi2c1;
    hi2c[1]= &hi2c2;
    I2cDeviceID_7bit = STUSBxx_DEVICEID_7BIT;
    
    
    printf("*** STUSB NVM programming ***\r\n");
    
    
    //Reset STUSBxx
    HAL_GPIO_WritePin(Reset_GPIO_Port,Reset_Pin,GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(Reset_GPIO_Port,Reset_Pin,GPIO_PIN_RESET);
    HAL_Delay(200);
    
    
#if READ_NVM
    {
        unsigned char NVM_Sectors[5][8] = {0, 0};
        status = nvm_read( &NVM_Sectors[0][0], sizeof(NVM_Sectors) );
        if(status != 0) //Error
        {
            printf("Error NVM read\r\n");
            return -1;
        }
    }
#endif //READ_NVM
    
    
#if FLASH_NVM
    {
        uint8_t Buffer;
        status = I2C_Read_USB_PD(0,DEVICE_ID, &Buffer, 1);     //ID=0x21 for STUSB4500
        if(status != 0)  { printf("Error I2C \r\n"); return -1;}
        
        if(Buffer & ID_Reg >= CUT)  
        {
            status = nvm_flash();
            if ( status != 0) 
                printf("STUSB Flashing Failed \r\n"); // Port 0 mean I2C1 For I2C2 change to Port 1
            else 
                printf("STUSB Flashing OK\r\n");
        }
        else  printf("STUSB Flashing Not done : Bad version\r\n");
    }
#endif //FLASH_NVM
    
    
#if VERIFY_NVM
    {
        uint8_t Verification_Sector[8];
        uint8_t Sector40[5][8];
        
        extern uint8_t Sector0[8];
        extern uint8_t Sector1[8];
        extern uint8_t Sector2[8];
        extern uint8_t Sector3[8];
        extern uint8_t Sector4[8];
        
        
        for(int i=0; i<8; i++) //copy sectors to 40byte array
        {
            Sector40[0][i] = Sector0[i];
            Sector40[1][i] = Sector1[i];
            Sector40[2][i] = Sector2[i];
            Sector40[3][i] = Sector3[i];
            Sector40[4][i] = Sector4[i];
        }
        
        if (CUST_EnterReadMode(0) == 0 ) //OK
        {
            for(int bk=0; bk<5; bk++)
            {
                if (CUST_ReadSector(0,bk, &Verification_Sector[0]) ==0)
                {
                    if (  *(uint64_t *) Verification_Sector != *(uint64_t *) (Sector40[bk])   )
                    {
                        for ( int j = 0 ;j < 8 ; j++)
                        {
                            if (  Verification_Sector[j] != Sector40[bk][j])
                                printf("NVM verification issue byte %i bank %i \r\n", j, bk);
                        }
                    }  
                    else   printf("NVM verification bank %i : OK \r\n", bk);
                }
                else  printf("NVM read bank %i : failed \r\n", bk);
            }
        }
        else  printf("NVM read initialization: failed \r\n");
    }
#endif //VERIFY_NVM

    
    /* USER CODE END 2 */
    
    return 0;
}

