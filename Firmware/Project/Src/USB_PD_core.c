#include "USB_PD_core.h"

#include "i2c.h"


extern I2C_HandleTypeDef *hi2c[I2CBUS_MAX];	
extern unsigned int Address;

extern int PB_press ;
USB_PD_StatusTypeDef PD_status[USBPORT_MAX] ;
USB_PD_CTRLTypeDef   PD_ctrl[USBPORT_MAX];
USB_PD_SNK_PDO_TypeDef PDO_SNK[USBPORT_MAX][3];

USB_PD_SRC_PDOTypeDef PDO_FROM_SRC[USBPORT_MAX][7];
uint8_t PDO_FROM_SRC_Num[USBPORT_MAX]={0};
extern STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef Nego_RDO;

uint8_t PDO_SRC_NUMB[USBPORT_MAX];
uint8_t PDO_SNK_NUMB[USBPORT_MAX];


extern uint8_t  USB_PD_Interupt_Flag[USBPORT_MAX] ;
extern uint8_t USB_PD_Status_change_flag[USBPORT_MAX] ;

extern USB_PD_I2C_PORT STUSB45DeviceConf[USBPORT_MAX];

/**
* @brief  asserts and de-asserts the STUSB4500 Hardware reset pin.
* @param  I2C Port used (I2C1 or I2C2).
* @param  none
* @retval none
*/

/************************   HW_Reset_state(uint8_t Port)  ***************************
This function asserts and de-asserts the STUSB4500 Hardware reset pin.  
After reset, STUSB4500 behave according to Non Volatile Memory defaults settings. 
************************************************************************************/
void HW_Reset_state(uint8_t Usb_Port)
{
  uint8_t Buffer[2];
  HAL_GPIO_WritePin(Reset_GPIO_Port,Reset_Pin,GPIO_PIN_SET);
  HAL_Delay(15);  /*time to be dedected by the source */
  HAL_GPIO_WritePin(Reset_GPIO_Port,Reset_Pin,GPIO_PIN_RESET);
  usb_pd_init(0);
  Buffer[0] = 3;
  I2C_Write_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,DPM_PDO_NUMB,&Buffer[0],1 ); 
  
}

/************************   SW_reset_by_Reg (uint8_t Port)  *************************
This function resets STUSB45 type-C and USB PD state machines. It also clears any
ALERT. By initialisating Type-C pull-down termination, it forces electrical USB type-C
disconnection (both on SOURCE and SINK sides). 
************************************************************************************/

void SW_reset_by_Reg(uint8_t Usb_Port)
{
  int Status,i;
  uint8_t Buffer[12];
  Buffer[0] = 1;
  Status = I2C_Write_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,STUSB_GEN1S_RESET_CTRL_REG,&Buffer[0],1 );
  
  for (i=0;i<=12;i++)
    Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,ALERT_STATUS_1+i ,&Buffer[0], 1 );  // clear ALERT Status
  HAL_Delay(27); // on source , the debounce time is more than 15ms error recovery > at 25ms 
  Buffer[0] = 0; 
  Status = I2C_Write_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,STUSB_GEN1S_RESET_CTRL_REG,&Buffer[0],1 ); 
  
}

/***************************   usb_pd_init(uint8_t Port)  ***************************
this function clears all interrupts and unmask the usefull interrupt

************************************************************************************/

void usb_pd_init(uint8_t Usb_Port)
{
  STUSB_GEN1S_ALERT_STATUS_MASK_RegTypeDef Alert_Mask;
  int Status;
  static int i,j;
  static unsigned char DataRW[40];	
  DataRW[0]= 0;
  
  uint8_t Cut;
  I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,0x2F ,&Cut, 1 );
  
  Address = DPM_SNK_PDO1;
  I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,Address ,&DataRW[0],12  );
  j=0;
  for ( i = 0 ; i < 3 ; i++)
  {
    PDO_SNK[Usb_Port][i].d32 = (uint32_t )( DataRW[j] +(DataRW[j+1]<<8)+(DataRW[j+2]<<16)+(DataRW[j+3]<<24));
    j +=4;
  }    
  Address = ALERT_STATUS_1;
  for (i=0;i<=12;i++) /* clear ALERT Status */
    Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,Address+i ,&DataRW[0], 1 );  // clear ALERT Status
  
  
  Alert_Mask.d8 = 0xFF;
  
  Alert_Mask.b.CC_DETECTION_STATUS_AL_MASK = 0;
  Alert_Mask.b.PD_TYPEC_STATUS_AL_MASK = 0;
  Alert_Mask.b.PRT_STATUS_AL_MASK = 0;
  DataRW[0]= Alert_Mask.d8;// interrupt unmask 
  Status = I2C_Write_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,ALERT_STATUS_MASK ,&DataRW[0], 1 ); // unmask port status alarm 
  Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,PORT_STATUS ,&DataRW[0], 10 ); 
  USB_PD_Interupt_Flag[Usb_Port] =0;
  PD_status[Usb_Port].Port_Status.d8 = DataRW[ 1 ] ;
  PD_status[Usb_Port].CC_status.d8 = DataRW[3];
  PD_status[Usb_Port].HWFault_status.d8 = DataRW[6];
  PD_status[Usb_Port].Monitoring_status.d8=DataRW[3];
  
  
  return;
}

/**********************   ALARM_MANAGEMENT(uint8_t Port)  ***************************
device interrupt Handler

************************************************************************************/

void ALARM_MANAGEMENT(uint8_t Usb_Port)   
{
  
  int i,Status;
  static unsigned char DataRead,DataWrite=0;
  STUSB_GEN1S_ALERT_STATUS_RegTypeDef Alert_Status;
  STUSB_GEN1S_ALERT_STATUS_MASK_RegTypeDef Alert_Mask;
  static unsigned char DataRW[40];
  
  
  if ( HAL_GPIO_ReadPin(ALERT_A_GPIO_Port,ALERT_A_Pin)== GPIO_PIN_RESET)
  {
    Address = ALERT_STATUS_1; 
    Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,Address ,&DataRW[0], 2 );
    Alert_Mask.d8 = DataRW[1]; 
    Alert_Status.d8 = DataRW[0] & ~Alert_Mask.d8;
    //       printf("\r\n read Status : 0x%04x", (uint16_t)DataRW[1] |DataRW[0]<<8  );
    //                printf("AL status:0x%x Mask:0x%x\n",DataRW[0],DataRW[1]);
    if (Alert_Status.d8 != 0)
    {     
      PD_status[Usb_Port].HW_Reset = (DataRW[ 0 ] >> 7);
      
      if (Alert_Status.b.CC_DETECTION_STATUS_AL !=0)
      {
        Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,PORT_STATUS_TRANS ,&DataRW[0], 2 );
        PD_status[Usb_Port].Port_Status.d8= DataRW[ 1 ]; 
        
      }
      if (Alert_Status.b.MONITORING_STATUS_AL !=0)
      {
        Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,TYPEC_MONITORING_STATUS_0 ,&DataRW[0], 2 );
        PD_status[Usb_Port].Monitoring_status.d8 = DataRW[ 1 ];
      }
      Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,CC_STATUS ,&DataRW[0], 1);
      PD_status[Usb_Port].CC_status.d8 = DataRW[ 0];
      
      if (Alert_Status.b.HW_FAULT_STATUS_AL !=0)
      {
        Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,CC_HW_FAULT_STATUS_0 ,&DataRW[0], 2 );
        PD_status[Usb_Port].HWFault_status.d8 = DataRW[ 1 ]; 
      }
      
      if (Alert_Status.b.PRT_STATUS_AL !=0)
      {
        USBPD_MsgHeader_TypeDef Header;
        STUSB_GEN1S_PRT_STATUS_RegTypeDef Prt_Status;
        Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,PRT_STATUS ,&PD_status[Usb_Port].PRT_status.d8, 1 );
        
        if (PD_status[Usb_Port].PRT_status.b.MSG_RECEIVED == 1)
        {
          Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,RX_HEADER ,&DataRW[0], 2 );
          Header.d16 = LE16(&DataRW[0]);
          if( Header.b.NumberOfDataObjects > 0  )
          {
            switch ( Header.b.MessageType )
            {
            case 0x01 :
              {
                static int i ,j ;
                Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,RX_DATA_OBJ ,&DataRW[0], Header.b.NumberOfDataObjects * 4 );
                j=0;
                
                PDO_FROM_SRC_Num[Usb_Port]= Header.b.NumberOfDataObjects;
                for ( i = 0 ; i < Header.b.NumberOfDataObjects ; i++)
                {
                  PDO_FROM_SRC[Usb_Port][i].d32 = (uint32_t )( DataRW[j] +(DataRW[j+1]<<8)+(DataRW[j+2]<<16)+(DataRW[j+3]<<24));
                  j +=4;
                  
                }
              }
              break;
            default :
              break;
            }
            
            
            
          }
          else 
          {
#ifdef PRINTF            
            __NOP();
            // printf("Ctrl Message :0x%04x \r\n",Header.d16);
#else 
            __NOP();
#endif
          }
          
        }
        
      }
    }
    
    USB_PD_Interupt_Flag[Usb_Port] = 0;
  }
  
}


/**********************     Read_SNK_PDO(uint8_t Port)   ***************************
This function reads the PDO registers. 

************************************************************************************/


void Read_SNK_PDO(uint8_t Usb_Port)
{
  static unsigned char DataRW[12];	
  DataRW[0]= 0;
  
  static int i ,j ;
  
  
  Address = DPM_PDO_NUMB ;
  
  //printf("\n------Read STUSB NVM -------- %x\n", Address);  
  
  if ( I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,Address ,&DataRW[0], 1 )== HAL_I2C_ERROR_NONE  ) 
  {
    
    PDO_SNK_NUMB[Usb_Port] = (DataRW[0] & 0x03 );
    Address = DPM_SNK_PDO1;
    I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,Address ,&DataRW[0],PDO_SNK_NUMB[Usb_Port]*4  );
    j=0;
    for ( i = 0 ; i < PDO_SNK_NUMB[Usb_Port] ; i++)
    {
      PDO_SNK[Usb_Port][i].d32 = (uint32_t )( DataRW[j] +(DataRW[j+1]<<8)+(DataRW[j+2]<<16)+(DataRW[j+3]<<24));
      j +=4;
    }
  }
  USB_PD_Interupt_Flag[Usb_Port] =0;
  return;
}



/**********************     Print_SNK_PDO(uint8_t Port)   ***************************
This function print the STUSB4500 PDO to the serial interface. 

************************************************************************************/

void Print_SNK_PDO(uint8_t Usb_Port)  
{
  static uint8_t i;
  static float PDO_V;
  static float PDO_I;
  static int   PDO_P;
  static int MAX_POWER = 0;
  MAX_POWER = 0;
  
  // Does not work if instanciated here
#ifdef PRINTF
  printf("\r\n---- Usb_Port #%i : Read local Sink PDO  ------\r\n",Usb_Port);
#endif
  Read_SNK_PDO(Usb_Port);
#ifdef PRINTF          
  printf("%x x PDO:\r\n",PDO_SNK_NUMB[Usb_Port]);
#endif
  for (i=0; i< PDO_SNK_NUMB[Usb_Port]; i++)
  {
    switch (PDO_SNK[Usb_Port][i].fix.Fixed_Supply)
    {
    case 0:  /* fixed supply */
      {
        static float PDO_V;
        static float PDO_I;
        static int   PDO_P;          
        PDO_V = (float) (PDO_SNK[Usb_Port][i].fix.Voltage)/20;
        PDO_I = (float) (PDO_SNK[Usb_Port][i].fix.Operationnal_Current)/100;
        PDO_P = (int) PDO_V*PDO_I; 
#ifdef PRINTF   
        printf(" - Fixed PDO%u=(%4.2fV, %4.2fA, = %uW)\r\n",i+1, PDO_V, PDO_I, PDO_P );
#endif 
        if (PDO_P >=MAX_POWER)
        { MAX_POWER = PDO_P;}
      }
      break;
    case 1: /* Variable Supply */
      {
        static float PDO_V_Min;
        static float PDO_V_Max;
        static float PDO_I;
        PDO_V_Max = (float) (PDO_SNK[Usb_Port][i].var.Max_Voltage)/20;
        PDO_V_Min = (float) (PDO_SNK[Usb_Port][i].var.Min_Voltage)/20;
        PDO_I = (float) (PDO_SNK[Usb_Port][i].var.Operating_Current)/100;
#ifdef PRINTF   
        printf(" - Variable PDO%u=(%4.2fV, %4.2fV, = %4.2fA)\r\n",i+1, PDO_V_Min, PDO_V_Max, PDO_I );
#endif 
        
      }
      break;
    case 2: /* Battery Supply */
      {
        static float PDO_V_Min;
        static float PDO_V_Max;
        static int   PDO_P;          
        PDO_V_Max = (float) (PDO_SNK[Usb_Port][i].bat.Max_Voltage)/20;
        PDO_V_Min = (float) (PDO_SNK[Usb_Port][i].bat.Min_Voltage)/20;
        PDO_P = (float) (PDO_SNK[Usb_Port][i].bat.Operating_Power)/4; 
#ifdef PRINTF   
        printf(" -Battery PDO%u=(%4.2fV, %4.2fV, = %uW)\r\n",i+1, PDO_V_Min, PDO_V_Max, PDO_P );
#endif 
        if (PDO_P >=MAX_POWER)
        { MAX_POWER = PDO_P;}
      }
      break;            
    default :
      break;
    }
  }
  
}



/**********************     Read_RDO(uint8_t Port)   ***************************
This function reads the Requested Data Object (RDO) register. 

************************************************************************************/

void Read_RDO(uint8_t Usb_Port) 
{
  I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,RDO_REG_STATUS ,(uint8_t *)&Nego_RDO.d32, 4 );
}


/**********************     Print_RDO(uint8_t Port)   ***************************
This function prints to the serial interface the current contract in case of 
capability MATCH between the STUSB4500 and the SOURCE.

************************************************************************************/

void Print_RDO(uint8_t Usb_Port)
{
  static uint8_t Buffer;
  I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,0x21 ,&Buffer, 1 );
  if (Nego_RDO.d32 != 0)
  {
    printf(" \r\n Requested PDO Position :%d ,Max Current : %d ,Operating Current : %d Capability Mismatch : %d \r\n",Nego_RDO.b.Object_Pos,Nego_RDO.b.MaxCurrent,Nego_RDO.b.OperatingCurrent,Nego_RDO.b.CapaMismatch);
    printf(" Voltage requested :%i mV\r\n" , (uint16_t)Buffer*100);
  }
  else
  {
    printf(" No explicit Contract yet\r\n");
    printf(" Voltage :%i mV \r\n" , (uint16_t)Buffer*100);
  }
}


/******************   Update_PDO(Port, PDO_number, Voltage, Current)   *************
This function must be used to overwrite PDO2 or PDO3 content in RAM.
Arguments are:
- Port Number:
- PDO Number : 2 or 3 , 
- Voltage in(mV) truncated by 50mV ,
- Current in(mv) truncated by 10mA
************************************************************************************/

void Update_PDO(uint8_t Usb_Port,uint8_t PDO_Number,int Voltage,int Current)
{
  uint8_t adresse;
  int Status;
  PDO_SNK[Usb_Port][PDO_Number - 1].fix.Voltage = Voltage /50 ;
  PDO_SNK[Usb_Port][PDO_Number- 1 ].fix.Operationnal_Current = Current / 10;
  if ( (PDO_Number == 2) ||(PDO_Number == 3))
  {
    adresse = DPM_SNK_PDO1 + 4*(PDO_Number - 1) ;
    Status = I2C_Write_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,adresse ,(uint8_t *)&PDO_SNK[Usb_Port][PDO_Number - 1].d32, 4 );
  }
}

/************* Update_Valid_PDO_Number(Port, PDO_Number)  ***************************
This function is used to overwrite the number of valid PDO
Arguments are: 
- Port Number,
- active PDO Number: from 1 to 3 
************************************************************************************/

void Update_Valid_PDO_Number(uint8_t Usb_Port,uint8_t Number_PDO)
{
  
  if (Number_PDO >= 1 && Number_PDO <=3)
  {
    PDO_SNK_NUMB[Usb_Port] = Number_PDO;
    I2C_Write_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,DPM_PDO_NUMB,&Number_PDO,1 ); 
  }
}




/**********************     Set_New_PDO_case1(uint8_t Port)   ************************
Sample function that sets PDO2 and PDO3 to 15V/1.5A and 20V/1.5A respectively.

************************************************************************************/


void Set_New_PDO_case1(uint8_t Usb_Port)
{
  uint8_t Buf[4];
  int Status;
  static uint8_t i,j;
  static float SRCPDO_V,SNKPDO_V;
  static float SRCPDO_I,SNKPDO_I;
  static int   SRCPDO_P,SNKPDO_P;
  static int MAX_POWER = 0;
  uint8_t adresse ;
  
  if (PB_press == 1 )
  {
    Update_PDO(Usb_Port,2,15000,1500);
    Update_PDO(Usb_Port,3,20000,1500);
  }  
    Update_Valid_PDO_Number( Usb_Port , 2);
  Print_SNK_PDO(Usb_Port);
}


/****************************     Negotiate_5V( Port)    ***************************
Sample function that reconfigures the PDO number to only one, so by default PDO1. 
This drives the STUSB4500 to negotiates 5V back with the SOURCE.

************************************************************************************/ 

void Negotiate_5V(uint8_t Usb_Port)  
{
  
    Update_Valid_PDO_Number( Usb_Port , 1 );
}


/**********************     Find_Matching_SRC_PDO(uint8_t Usb_Port,int Min_Power,int Min_V , int Max_V)   ************************
/**
* @brief scans the SOURCE PDO (received at connection). If one of the SOURCE PDO
falls within the range of the functions arguments, ie. within a Voltage range and 
Power Range relevant for the applications, then it redefines the SINK_PDO3 with such
PDO parameters and re-negotiates. This allows STUSB4500 to best match to the SOURCE
capabilities.
* @param  I2C Port used (I2C1 or I2C2).
* @param  Min Power  in W 
* @param  Min Voltage in mV
* @param  Max Voltage in mV
* @retval 0 if PDO3 updated 1 if not 
*/
int Find_Matching_SRC_PDO(uint8_t Usb_Port,int Min_Power,int Min_V , int Max_V)
{
  static uint8_t i;
  int PDO_V;
  int PDO_I;
  int   PDO_P;
  int PDO1_updated = 0 ;
  uint8_t adresse;
    
  int Status;
  if(PDO_FROM_SRC_Num[Usb_Port] > 1 )
  {
    for (i=1; i< PDO_FROM_SRC_Num[Usb_Port]; i++)   // loop started from PDO2 
    {
      PDO_V = PDO_FROM_SRC[Usb_Port][i].fix.Voltage * 50;
      PDO_I = PDO_FROM_SRC[Usb_Port][i].fix.Max_Operating_Current * 10;
      PDO_P = (int)(( PDO_V/1000) * (PDO_I/1000)); 
      if ((PDO_P >=Min_Power ) && (PDO_V > Min_V ) && (PDO_V <= Max_V ))
      {
        Update_PDO( Usb_Port, 3 ,PDO_V , PDO_I );
        PDO1_updated = 1 ;
      }              
    }
    
        Update_Valid_PDO_Number(Usb_Port,3);
  }
  
  if(PDO1_updated)
    return 0;
  
  return 1;
  
}

/**********************     Print_PDO_FROM_SRC(uint8_t Usb_Port)    *************************** 
This function prints the SOURCE capabilities received by the STUSB4500. 
SOURCE capabilities are automatically stored at device connection in a dedicated structure.
************************************************************************************/

void Print_PDO_FROM_SRC(uint8_t Usb_Port)  
{
  static uint8_t i;
  static float PDO_V;
  static float PDO_I;
  static int   PDO_P;
  static int MAX_POWER = 0;
  MAX_POWER = 0;
  
#ifdef PRINTF
  printf("\r\n---- Usb_Port #%i: Read PDO from Source      ----\r\n",Usb_Port);
#endif
  //  Read_PDO_SRC(Usb_Port);
#ifdef PRINTF          
  printf("%x x PDO:\r\n",PDO_FROM_SRC_Num[Usb_Port]);
#endif
  for (i=0; i< PDO_FROM_SRC_Num[Usb_Port]; i++)
  {
    PDO_V = (float) (PDO_FROM_SRC[Usb_Port][i].fix.Voltage)/20;
    PDO_I = (float) (PDO_FROM_SRC[Usb_Port][i].fix.Max_Operating_Current)/100;
    PDO_P = (int) PDO_V*PDO_I; 
#ifdef PRINTF  
    printf(" - PDO_FROM_SRC(%u)=(%4.2fV, %4.2fA, = %uW)\r\n",i+1, PDO_V, PDO_I, PDO_P );
#endif 
    if (PDO_P >=MAX_POWER)
    { MAX_POWER = PDO_P;}
  }
#ifdef PRINTF 
  printf("P(max)=%uW\r\n", MAX_POWER );
#endif  
  
}
