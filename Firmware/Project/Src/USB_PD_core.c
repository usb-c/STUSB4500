#include "USB_PD_core.h"

#include "i2c.h"
#include "USBPD_spec_defines.h"
#include "PostProcessEvents.h"

#if DEBUG_PRINTF
#define DEBUG_IRQ
#endif


extern I2C_HandleTypeDef *hi2c[I2CBUS_MAX];	
extern unsigned int Address;

extern int PB_press ;
USB_PD_StatusTypeDef PD_status[USBPORT_MAX] ;
USB_PD_CTRLTypeDef   PD_ctrl[USBPORT_MAX];
USB_PD_SNK_PDO_TypeDef PDO_SNK[USBPORT_MAX][3];

USB_PD_SRC_PDOTypeDef PDO_FROM_SRC[USBPORT_MAX][SRC_PDO_NUM_MAX];
uint8_t PDO_FROM_SRC_Num[USBPORT_MAX]={0};
extern STUSB_GEN1S_RDO_REG_STATUS_RegTypeDef Nego_RDO;

uint8_t PDO_SRC_NUMB[USBPORT_MAX];
uint8_t PDO_SNK_NUMB[USBPORT_MAX];


extern volatile uint8_t  USB_PD_Interrupt_Flag[USBPORT_MAX] ;
//extern uint8_t USB_PD_Status_change_flag[USBPORT_MAX] ;

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
    
    usb_pd_init(Usb_Port); //refresh main registers & IRQ mask init needed after reset
    
    Buffer[0] = 3;
    I2C_Write_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,DPM_PDO_NUMB,&Buffer[0],1 ); 
}

/************************   SW_reset_by_Reg (uint8_t Port)  *************************
This function resets STUSB45 type-C and USB PD state machines. It also clears any
ALERT. By initialisating Type-C pull-down termination, it forces electrical USB type-C
disconnection (both on SOURCE and SINK sides). 
************************************************************************************/

int SW_reset_by_Reg(uint8_t Usb_Port)
{
    int Status,i;
    uint8_t Buffer[12];
    Buffer[0] = SW_RST;
    Status = I2C_Write_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,STUSB_GEN1S_RESET_CTRL_REG,&Buffer[0],1 );
    if(Status != 0) return -1; //I2C Error
    
    
    //Wait the device is ready to reply after reset
    for(i=0; i<2; i++)
    {
        uint8_t DevId;
        Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit, REG_DEVICE_ID ,&DevId, 1 );
    }
    
    
    for (i=0;i<=12;i++)
    {
        Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,ALERT_STATUS_1+i ,&Buffer[0], 1 );  // clear ALERT Status
        if(Status != 0) return -1; //I2C Error
    }
    
    HAL_Delay(27); // on source , the debounce time is more than 15ms error recovery > at 25ms 
    
    Buffer[0] = No_SW_RST; 
    Status = I2C_Write_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,STUSB_GEN1S_RESET_CTRL_REG,&Buffer[0],1 ); 
    
    return 0;
}

/***************************   usb_pd_init(uint8_t Port)  ***************************
this function clears all interrupts and unmask the usefull interrupt

************************************************************************************/

int usb_pd_init(uint8_t Usb_Port)
{
    STUSB_GEN1S_ALERT_STATUS_MASK_RegTypeDef Alert_Mask;
    int Status;
    int i,j;
    unsigned char DataRW[40];	
    DataRW[0]= 0;
    
    uint8_t Cut;
    I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit , REG_DEVICE_ID ,&Cut, 1 );
    
    Address = DPM_SNK_PDO1;
    I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,Address ,&DataRW[0],12  );
    j=0;
    for ( i = 0 ; i < 3 ; i++)
    {
        PDO_SNK[Usb_Port][i].d32 = (uint32_t )( DataRW[j] +(DataRW[j+1]<<8)+(DataRW[j+2]<<16)+(DataRW[j+3]<<24));
        j +=4;
    }    
    
    // clear all ALERT Status
    Address = ALERT_STATUS_1;
    for (i=0;i<=12;i++) /* clear ALERT Status */
    {
        Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,Address+i ,&DataRW[0], 1 );  // clear ALERT Status
        if(Status != 0) return -1;
    }
    
    
    //Set Interrupt to unmask
    Alert_Mask.d8 = 0xFF;
    Alert_Mask.b.HARD_RESET_AL_MASK = 0; //Added Mask
    Alert_Mask.b.CC_DETECTION_STATUS_AL_MASK = 0;
    Alert_Mask.b.PD_TYPEC_STATUS_AL_MASK = 0;
    Alert_Mask.b.PRT_STATUS_AL_MASK = 0;
    DataRW[0]= Alert_Mask.d8;// interrupt unmask 
    Status = I2C_Write_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,ALERT_STATUS_MASK ,&DataRW[0], 1 ); // unmask port status alarm 
    if(Status != 0) return -1;
    
    Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,PORT_STATUS ,&DataRW[0], 10 ); 
    if(Status != 0) return -1;
    
    USB_PD_Interrupt_Flag[Usb_Port] =0;
    PD_status[Usb_Port].Port_Status.d8 = DataRW[ 1 ] ;
    PD_status[Usb_Port].CC_status.d8 = DataRW[3];
    PD_status[Usb_Port].HWFault_status.d8 = DataRW[6];
    PD_status[Usb_Port].Monitoring_status.d8=DataRW[3];
    
    
    return 0;
}

/**********************   ALARM_MANAGEMENT(uint8_t Port)  ***************************
device interrupt Handler

************************************************************************************/

void ALARM_MANAGEMENT(uint8_t Usb_Port)   
{
    int Status = 0;
    STUSB_GEN1S_ALERT_STATUS_RegTypeDef Alert_Status;
    STUSB_GEN1S_ALERT_STATUS_MASK_RegTypeDef Alert_Mask;
    unsigned char DataRW[40];
    
    UNUSED(Status);
    
    Address = ALERT_STATUS_1; 
    Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,Address ,&DataRW[0], 2 );
    Alert_Mask.d8 = DataRW[1]; 
    Alert_Status.d8 = DataRW[0] & ~Alert_Mask.d8;
    
    
#ifdef DEBUG_IRQ
    Push_IrqReceived(Alert_Status.d8);
#endif
    
    if (Alert_Status.d8 != 0)
    {    
        //bit 7
        PD_status[Usb_Port].HW_Reset = (DataRW[ 0 ] >> 7);
        if (PD_status[Usb_Port].HW_Reset !=0)
        {
            PostProcess_IrqHardreset++;
        }
        
        //bit 6
        if (Alert_Status.b.CC_DETECTION_STATUS_AL !=0)
        {
            //[Read/Clear]
            Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,PORT_STATUS_TRANS ,&DataRW[0], 2 );
            PD_status[Usb_Port].Port_Status_Trans.d8 = DataRW[ 0 ]; 
            PD_status[Usb_Port].Port_Status.d8= DataRW[ 1 ]; 
            
            if( (DataRW[0] & STUSBMASK_ATTACH_STATUS_TRANS) != 0)
            {
                PostProcess_AttachTransition++;
            }
            
        }
        
        //bit 5
        if (Alert_Status.b.MONITORING_STATUS_AL !=0)
        {
            Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,TYPEC_MONITORING_STATUS_0 ,&DataRW[0], 2 );
            PD_status[Usb_Port].Monitoring_status.d8 = DataRW[ 1 ];
        }
        
        //Always read & update CC Attachement status
        Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,CC_STATUS ,&DataRW[0], 1);
        PD_status[Usb_Port].CC_status.d8 = DataRW[0];
        
        //bit 4
        if (Alert_Status.b.HW_FAULT_STATUS_AL !=0)
        {
            Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,CC_HW_FAULT_STATUS_0 ,&DataRW[0], 2 );
            PD_status[Usb_Port].HWFault_status.d8 = DataRW[ 1 ]; 
        }
        
        //bit 1
        if (Alert_Status.b.PRT_STATUS_AL !=0)
        {
            USBPD_MsgHeader_TypeDef Header;
            //STUSB_GEN1S_PRT_STATUS_RegTypeDef Prt_Status;
            
            Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,PRT_STATUS ,&PD_status[Usb_Port].PRT_status.d8, 1 );
            
            if (PD_status[Usb_Port].PRT_status.b.MSG_RECEIVED == 1)
            {
                Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,RX_HEADER ,&DataRW[0], 2 );
                Header.d16 = LE16(&DataRW[0]);
                
                
                if( Header.b.NumberOfDataObjects > 0 ) //Number_of_Data_Objects field > 0 --> Message is a Data Message
                {
#ifdef DEBUG_IRQ
                    Push_PD_MessageReceived('D', Header.b.MessageType); //DataMsg
#endif
                    
                    int RxByteCount;
                    Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,RX_BYTE_CNT ,&DataRW[0], 1 );
                    RxByteCount = DataRW[0];
                    
                    if(RxByteCount != Header.b.NumberOfDataObjects * 4)
                    {
                        //error, missing Data
                        return;
                    }
                    
                    switch ( Header.b.MessageType )
                    {
                    case USBPD_DATAMSG_Source_Capabilities :
                        {
                            // Warning: Short Timing
                            // There is ~3 ms timeframe to read the SourceCap, before the next Message ("Accept") arrives and overwrites the first bytes of RX_DATA_OBJ register
                            
                            int i ,j ;
                            
                            Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,RX_DATA_OBJ ,&DataRW[0], Header.b.NumberOfDataObjects * 4 );
                            
                            j=0;
                            PDO_FROM_SRC_Num[Usb_Port]= Header.b.NumberOfDataObjects;
                            for ( i = 0 ; i < Header.b.NumberOfDataObjects ; i++)
                            {
                                PDO_FROM_SRC[Usb_Port][i].d32 = (uint32_t )( (uint32_t )DataRW[j] | ( (uint32_t )DataRW[j+1]<<8) | ( (uint32_t )DataRW[j+2]<<16) | ( (uint32_t )DataRW[j+3]<<24));
                                j +=4;
                                
                            }
                            
                            PostProcess_SRC_PDO_Received++;
                        }
                        break;
                        
                        
                    case USBPD_DATAMSG_Request:  /* get request message */
                        break;
                        
                    case USBPD_DATAMSG_Sink_Capabilities: /* receive Sink cap  */
                        break;
                        
                    case USBPD_DATAMSG_Vendor_Defined:  /* VDM message */ 
                        break;
                        
                    default :
                        break;
                    }
                }
                else  //Number_of_Data_Objects field == 0 --> Message is a Control-Message
                {
                    __NOP(); //for breakpoint
                    
#ifdef DEBUG_IRQ
                    Push_PD_MessageReceived('C', Header.b.MessageType); //CtrlMsg
#endif
                    
                    switch (Header.b.MessageType )
                    {   
                    case USBPD_CTRLMSG_Reserved1:
                        break;
                        
                    case USBPD_CTRLMSG_GoodCRC:
                        PostProcess_Msg_GoodCRC++;
                        break;
                        
                    case USBPD_CTRLMSG_Accept:
                        PostProcess_Msg_Accept++;
                        break;
                        
                    case USBPD_CTRLMSG_Reject:
                        PostProcess_Msg_Reject++;
                        break;
                        
                    case USBPD_CTRLMSG_PS_RDY:
                        PostProcess_PSRDY_Received++;
                        break;
                        
                    case USBPD_CTRLMSG_Get_Source_Cap:
                        break;
                        
                    case USBPD_CTRLMSG_Get_Sink_Cap:
                        break;
                        
                    case USBPD_CTRLMSG_Wait:
                        break;
                        
                    case USBPD_CTRLMSG_Soft_Reset:
                        break;
                        
                    case USBPD_CTRLMSG_Not_Supported:
                        break;
                        
                    case USBPD_CTRLMSG_Get_Source_Cap_Extended:
                        break;
                        
                    case USBPD_CTRLMSG_Get_Status:
                        break;
                        
                    case USBPD_CTRLMSG_FR_Swap:
                        break;
                        
                    case USBPD_CTRLMSG_Get_PPS_Status:
                        break;
                        
                    case USBPD_CTRLMSG_Get_Country_Codes:
                        break;
                        
                    default:
                        break;
                    }
                } //END if( Header.b.NumberOfDataObjects > 0 )
            }
            else //if (PD_status[Usb_Port].PRT_status.b.MSG_RECEIVED == 0)
            {
            }
            
        } //END if (Alert_Status.b.PRT_STATUS_AL !=0) //bit 1 
        
    }
    //}
    
    USB_PD_Interrupt_Flag[Usb_Port] = 0;
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
    
    //USB_PD_Interrupt_Flag[Usb_Port] =0;
    return;
}



/**********************     Print_SNK_PDO(uint8_t Port)   ***************************
This function print the STUSB4500 PDO to the serial interface. 

************************************************************************************/

void Print_SNK_PDO(uint8_t Usb_Port)  
{
    uint8_t i;
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
                float PDO_V;
                float PDO_I;
                int   PDO_P;          
                PDO_V = (float) (PDO_SNK[Usb_Port][i].fix.Voltage)/20;
                PDO_I = (float) (PDO_SNK[Usb_Port][i].fix.Operationnal_Current)/100;
                PDO_P = (int) (PDO_V*PDO_I); 
#ifdef PRINTF   
                printf(" - Fixed PDO%u=(%4.2fV, %4.2fA, = %uW)\r\n",i+1, PDO_V, PDO_I, PDO_P );
#endif 
                if (PDO_P >=MAX_POWER)
                { MAX_POWER = PDO_P;}
            }
            break;
        case 1: /* Variable Supply */
            {
                float PDO_V_Min;
                float PDO_V_Max;
                float PDO_I;
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
                float PDO_V_Min;
                float PDO_V_Max;
                float PDO_P;          
                PDO_V_Max = (float) (PDO_SNK[Usb_Port][i].bat.Max_Voltage)/20.0f;
                PDO_V_Min = (float) (PDO_SNK[Usb_Port][i].bat.Min_Voltage)/20.0f;
                PDO_P = (float) (PDO_SNK[Usb_Port][i].bat.Operating_Power)/4.0f; 
#ifdef PRINTF   
                printf(" -Battery PDO%u=(%4.2fV, %4.2fV, = %4.2fW)\r\n",i+1, PDO_V_Min, PDO_V_Max, PDO_P );
#endif 
                if (PDO_P >=MAX_POWER)
                { MAX_POWER = (int) PDO_P;}
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


int Get_RDO(uint8_t UsbPort, int * out_PDO_nb , int * out_Voltage_mV, int * out_Current_mA, int * out_MaxCurrent_mA)
{
    int status;
    
    status = I2C_Read_USB_PD(STUSB45DeviceConf[UsbPort].I2cBus,STUSB45DeviceConf[UsbPort].I2cDeviceID_7bit ,RDO_REG_STATUS ,(uint8_t *)&Nego_RDO.d32, 4 );
    if(status != 0) { return -1; }
    
    int PDO_number = Nego_RDO.b.Object_Pos;
    int OpCurrent_mA = Nego_RDO.b.OperatingCurrent * 10;
    int MaxCurrent_mA = Nego_RDO.b.MaxCurrent * 10;
    
    
    if( (out_PDO_nb == NULL) || (out_Voltage_mV == NULL) || (out_Current_mA == NULL) || (out_MaxCurrent_mA == NULL) )
        return -2;
    else
    {
        *out_PDO_nb = PDO_number;
        // *out_Voltage_mV = Voltage_mV;
        *out_Current_mA = OpCurrent_mA;
        *out_MaxCurrent_mA = MaxCurrent_mA;
    }
    
#if 0 //to test
    uint8_t Buffer;
    status = I2C_Read_USB_PD(STUSB45DeviceConf[UsbPort].I2cBus,STUSB45DeviceConf[UsbPort].I2cDeviceID_7bit , STUSB_GEN1S_MONITORING_CTRL_1 ,&Buffer, 1 );
    if(status != 0) { return -1; }
    int Voltage_mV = Buffer * 100;
    *out_Voltage_mV = Voltage_mV;
#endif
    
    
    *out_Voltage_mV = 0;
    if(PDO_number >=1)
    {
        int idx = PDO_number - 1;
        int PDO_mV = (PDO_FROM_SRC[UsbPort][idx].fix.Voltage)*50; // *1000/20 = 50;
        *out_Voltage_mV = PDO_mV;
    }
    
    
    return status;
}

/**********************     Print_RDO(uint8_t Port)   ***************************
This function prints to the serial interface the current contract in case of 
capability MATCH between the STUSB4500 and the SOURCE.

************************************************************************************/
#define USE_FLOAT

void Print_RDO(uint8_t Usb_Port) // RDO = Request Data Object
{
    uint8_t Buffer = 0;
    
    printf("\r\n");
    printf("                         ");
    printf("---- RDO (negociated power) ----\r\n");
    
    I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,RDO_REG_STATUS ,(uint8_t *)&Nego_RDO.d32, 4 );
    
    if (Nego_RDO.d32 != 0)
    {
        printf("                         ");
        printf("Requested position: PDO %d\r\n", Nego_RDO.b.Object_Pos);
        
#ifndef USE_FLOAT
        int OpCurrent_mA = Nego_RDO.b.OperatingCurrent * 10;
        int MaxCurrent_mA = Nego_RDO.b.MaxCurrent * 10;
        printf("                         ");
        printf("Operating Current: %d mA , Max Current: %d mA\r\n", OpCurrent_mA, MaxCurrent_mA);
#else
        float OpCurrent_A = (float) Nego_RDO.b.OperatingCurrent / 100.0f;
        float MaxCurrent_A = (float)  Nego_RDO.b.MaxCurrent / 100.0f;
        printf("                         ");
        printf("Operating Current: %4.2f A , Max Current: %4.2f A\r\n", OpCurrent_A, MaxCurrent_A);
#endif
        
        printf("                         ");
        printf("USB Com capable: %d , Capability Mismatch: %d \r\n", Nego_RDO.b.UsbComCap, Nego_RDO.b.CapaMismatch);
    }
    else
    {
        printf("                         ");
        printf("No explicit Contract yet\r\n");
    }
    
    I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit , STUSB_GEN1S_MONITORING_CTRL_1 ,&Buffer, 1 );
    
#ifndef USE_FLOAT
    int Voltage_mV = Buffer*100;
    printf("                         ");
    printf("Voltage requested: %i mV\r\n" , Voltage_mV);
#else
    float Voltage_V = (float)Buffer / 10.0f;
    printf("                         ");
    printf("Voltage requested: %4.2f V\r\n" , Voltage_V);
#endif
}

void Print_requested_PDO_Voltage(void)
{
    int Usb_Port=0;
    uint8_t Buffer = 0;
    
    I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit , STUSB_GEN1S_MONITORING_CTRL_1 ,&Buffer, 1 );
    
#ifndef USE_FLOAT
    int Voltage_mV = Buffer*100;
    printf("                         ");
    printf("Voltage requested: %i mV\r\n" , Voltage_mV);
#else
    float Voltage_V = (float)Buffer / 10.0f;
    printf("                         ");
    printf("Voltage requested: %4.2f V\r\n" , Voltage_V);
#endif
}

/******************   Update_PDO(Port, PDO_number, Voltage, Current)   *************
This function must be used to overwrite PDO1, PDO2 or PDO3 content in RAM.
Arguments are:
- Port Number:
- PDO Number : 1, 2 or 3 , 
- Voltage in(mV) truncated by 50mV ,
- Current in(mv) truncated by 10mA
************************************************************************************/

int Update_PDO(uint8_t UsbPort, uint8_t PDO_Number, int Voltage_mV, int Current_mA)
{
    uint8_t address;
    int Status = -1;
    
    if ( (PDO_Number == 1) || (PDO_Number == 2) || (PDO_Number == 3))
    {
        PDO_SNK[UsbPort][PDO_Number - 1 ].fix.Operationnal_Current = Current_mA / 10;
        
        if ( PDO_Number == 1)
        {
            //force 5V for PDO_1 to follow the USB PD spec
            PDO_SNK[UsbPort][PDO_Number - 1].fix.Voltage = 100; // 5000/50=100
        }
        else
        {
            PDO_SNK[UsbPort][PDO_Number - 1].fix.Voltage = Voltage_mV /50 ;
        }
        
        address = DPM_SNK_PDO1 + 4*(PDO_Number - 1) ;
        Status = I2C_Write_USB_PD(STUSB45DeviceConf[UsbPort].I2cBus,STUSB45DeviceConf[UsbPort].I2cDeviceID_7bit ,address ,(uint8_t *)&PDO_SNK[UsbPort][PDO_Number - 1].d32, 4 );
    }
    
    return Status;
}

/************* Update_Valid_PDO_Number(Port, PDO_Number)  ***************************
This function is used to overwrite the number of valid PDO
Arguments are: 
- Port Number,
- active PDO Number: from 1 to 3 
************************************************************************************/

int Update_Valid_PDO_Number(uint8_t Usb_Port,uint8_t Number_PDO)
{
    int Status = -1;
    
    if (Number_PDO >= 1 && Number_PDO <=3)
    {
        PDO_SNK_NUMB[Usb_Port] = Number_PDO;
        Status = I2C_Write_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,DPM_PDO_NUMB,&Number_PDO,1 ); 
    }
    
    return Status;
}


int Get_current_Sink_PDO_Numb(uint8_t UsbPort, uint8_t * out_PDO_Count)
{
    int Status = -1;
    int PDO_Count;
    
    if(out_PDO_Count == NULL)
        return -2;
    
    //PDO_Count = PDO_SNK_NUMB[UsbPort];
    Status = I2C_Read_USB_PD(STUSB45DeviceConf[UsbPort].I2cBus, STUSB45DeviceConf[UsbPort].I2cDeviceID_7bit, DPM_PDO_NUMB, &PDO_Count, 1 ); 
    
    if(Status == 0) 
    { 
        *out_PDO_Count = PDO_Count;
    }
    else
    {
        *out_PDO_Count = 0; //error
    }
    
    return Status;
}


/**********************     Set_New_PDO_case1(uint8_t Port)   ************************
Sample function that sets PDO2 and PDO3 to 15V/1.5A and 20V/1.5A respectively.

************************************************************************************/


void Set_New_PDO_case1(uint8_t Usb_Port)
{
    
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


/**********************     Find_Matching_SRC_PDO(uint8_t Usb_Port,int Min_Power,int Min_V , int Max_V)   ************************/
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
    
    int Status = 0;
    UNUSED(Status);
    
    if(PDO_FROM_SRC_Num[Usb_Port] > 1 )
    {
        for (i=1; i< PDO_FROM_SRC_Num[Usb_Port]; i++)   // loop started from PDO2 
        {
            PDO_V = PDO_FROM_SRC[Usb_Port][i].fix.Voltage * 50;
            PDO_I = PDO_FROM_SRC[Usb_Port][i].fix.Max_Operating_Current * 10;
            PDO_P = (int)(( PDO_V/1000) * (PDO_I/1000)); 
            if ((PDO_P >=Min_Power ) && (PDO_V > Min_V ) && (PDO_V <= Max_V ))
            {
                Status = Update_PDO( Usb_Port, 3 ,PDO_V , PDO_I );
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
    uint8_t i;
    float PDO_V;
    float PDO_I;
    int   PDO_P;
    int MAX_POWER;
    
    MAX_POWER = 0;
    
#ifdef PRINTF
    printf("\r\n");
    printf("                         ");
    printf("---- Usb_Port #%i: Read PDO from Source      ----\r\n",Usb_Port);
#endif
    //  Read_PDO_SRC(Usb_Port);
#ifdef PRINTF      
    printf("                         ");
    printf("%x x PDO:\r\n",PDO_FROM_SRC_Num[Usb_Port]);
#endif
    for (i=0; i< PDO_FROM_SRC_Num[Usb_Port]; i++)
    {
        PDO_V = (float) (PDO_FROM_SRC[Usb_Port][i].fix.Voltage)/20;
        PDO_I = (float) (PDO_FROM_SRC[Usb_Port][i].fix.Max_Operating_Current)/100;
        PDO_P = (int) (PDO_V * PDO_I); 
#ifdef PRINTF  
        printf("                         ");
        printf(" - PDO_FROM_SRC(%u)=(%4.2fV, %4.2fA, = %uW)\r\n",i+1, PDO_V, PDO_I, PDO_P );
#endif 
        if (PDO_P >=MAX_POWER)
        { MAX_POWER = PDO_P;}
    }
#ifdef PRINTF 
    printf("                         ");
    printf("P(max)=%uW\r\n", MAX_POWER );
#endif  
    
}

void Clear_PDO_FROM_SRC(uint8_t Usb_Port)
{
    PDO_FROM_SRC_Num[Usb_Port] = 0;
    
    for(int i=0; i< SRC_PDO_NUM_MAX; i++)
    {
        PDO_FROM_SRC[Usb_Port][i].d32 = 0;
    }
    
    Nego_RDO.d32 = 0;
}

//definitions for CC_STATUS register
#define STUSB4500_CC_NOT_LOOKING 0
#define STUSB4500_CC_LOOKING 1

#define STUSB4500_PRESENT_RP 0 //Pull-Up resistor
#define STUSB4500_PRESENT_RD 1 //Pull-Down resistor

#define STUSB4500_CC_SNK_Open 0
#define STUSB4500_CC_SNK_Default 1
#define STUSB4500_CC_SNK_Power15A 2
#define STUSB4500_CC_SNK_Power30A 3


int Print_TypeC_MaxCurrentAt5V_FROM_SRC(uint8_t Usb_Port)  
{
    int Status;
    unsigned char DataRW[2];
    
    
#ifdef PRINTF      
    printf("\r\n");
    printf("                         ");
    printf("--- Type-C Current at 5V (from SRC Pull-up resistor):\r\n");
#endif
    
    // read & update CC Attachement status
    Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,CC_STATUS ,&DataRW[0], 1);
    PD_status[Usb_Port].CC_status.d8 = DataRW[0];
    
    
    if( (PD_status[Usb_Port].CC_status.b.LOOKING_FOR_CONNECTION == STUSB4500_CC_NOT_LOOKING) && (PD_status[Usb_Port].CC_status.b.CONNECT_RESULT == STUSB4500_PRESENT_RD) )
    {
        do
        {
            // read & update CC Attachement status
            Status = I2C_Read_USB_PD(STUSB45DeviceConf[Usb_Port].I2cBus,STUSB45DeviceConf[Usb_Port].I2cDeviceID_7bit ,CC_STATUS ,&DataRW[0], 1);
            if(Status != 0) return -1; //error
            PD_status[Usb_Port].CC_status.d8 = DataRW[0];
        }
        while( (PD_status[Usb_Port].CC_status.b.LOOKING_FOR_CONNECTION == STUSB4500_CC_NOT_LOOKING) &&
              (PD_status[Usb_Port].CC_status.b.CC1_STATE == STUSB4500_CC_SNK_Open) &&
                  (PD_status[Usb_Port].CC_status.b.CC2_STATE == STUSB4500_CC_SNK_Open) );
        
#ifdef DEBUG
        printf("                         ");
        printf("CC1 state: %X\r\n", PD_status[Usb_Port].CC_status.b.CC1_STATE);
        printf("                         ");
        printf("CC2 state: %X\r\n", PD_status[Usb_Port].CC_status.b.CC2_STATE);
#endif
        
        int UsedCCpin_state = -1;
        
        if( (PD_status[Usb_Port].CC_status.b.CC1_STATE == STUSB4500_CC_SNK_Open) && (PD_status[Usb_Port].CC_status.b.CC2_STATE != STUSB4500_CC_SNK_Open) )
        {
#ifdef DEBUG
            printf("                         ");
            printf("CC2 pin attached. \r\n");
#endif
            UsedCCpin_state = PD_status[Usb_Port].CC_status.b.CC2_STATE;
        }
        
        if( (PD_status[Usb_Port].CC_status.b.CC2_STATE == STUSB4500_CC_SNK_Open) && (PD_status[Usb_Port].CC_status.b.CC1_STATE != STUSB4500_CC_SNK_Open) )
        {
#ifdef DEBUG
            printf("                         ");
            printf("CC1 pin attached. \r\n");
#endif
            UsedCCpin_state = PD_status[Usb_Port].CC_status.b.CC1_STATE;
        }
        
        if(UsedCCpin_state != -1)
        {
            if(UsedCCpin_state == STUSB4500_CC_SNK_Default)
            {
                printf("                         ");
                printf("Rp=Legacy Current (100mA, 500mA or 900mA) \r\n");
            }
            
            else if(UsedCCpin_state == STUSB4500_CC_SNK_Power15A)
            {
                printf("                         ");
                printf("Type-C Current: 1.5A\r\n");
            }
            else if(UsedCCpin_state == STUSB4500_CC_SNK_Power30A)
            {
                printf("                         ");
                printf("Type-C Current: 3.0A\r\n");
            }
        }
    }
    else
    {
        printf("USB-C not attached.\r\n");
    }
    
    return 0;
}

//-----------------------------------
//Check USB-C cable attachment status
//return -1 if not connected
//return 1 if connected on CC1
//return 2 if connected on CC2
//-----------------------------------
int CheckCableAttached()  
{
    int status;
    uint8_t UsbPort = 0;
    uint8_t Data;
    
    // read CC pin Attachment status
    status = I2C_Read_USB_PD(STUSB45DeviceConf[UsbPort].I2cBus,STUSB45DeviceConf[UsbPort].I2cDeviceID_7bit, PORT_STATUS, &Data, 1);  if(status != 0) { return -2; }
    
    if( (Data & STUSBMASK_ATTACHED_STATUS) == VALUE_ATTACHED) //only if USB-C cable attached
    {
        Address = TYPE_C_STATUS; //[Read only]
        status = I2C_Read_USB_PD(STUSB45DeviceConf[UsbPort].I2cBus, STUSB45DeviceConf[UsbPort].I2cDeviceID_7bit, Address , &Data, 1 ); if(status != 0) return -2; //I2C Error
        if(status != 0)
        {
          return -2; //I2C error
        }
        
        if(( Data & MASK_REVERSE) == 0)
        {
            return 1;  //OK, cable attached on CC1 pin
        }
        else
        {
            return 2;  //OK, cable attached on CC2 pin
        }
    }
    else
    {
        return -1; //Error, USB-C cable not attached
    }
}

int Change_PDO_WithoutLosingVbus(unsigned int New_PDO_Voltage)
{
    int Usb_Port = 0;
    int status = 0;
    
    if( (New_PDO_Voltage < 5000) || (New_PDO_Voltage > 20000) )
    {
        return -1; //Error, incorrect parameter
    }
    
    Update_Valid_PDO_Number( Usb_Port, 2 );
    Update_PDO(Usb_Port, 1, 5000, 1500);
    Update_PDO(Usb_Port, 2, New_PDO_Voltage, 1000); 
    status = PdMessage_SoftReset(); //force the new PDO negociation, by resetting the Source (Vbus is not lost during the reset)
    
    if(status != 0) return status;
    
    return 0; //OK
}

int Change_PDO_WithoutLosingVbus_WithTimeout(unsigned int New_PDO_Voltage)
{
    int Usb_Port = 0;
    int status = 0;
    
    if( (New_PDO_Voltage < 5000) || (New_PDO_Voltage > 20000) )
    {
        return -1; //Error, incorrect parameter
    }
    
    Update_Valid_PDO_Number( Usb_Port, 2 );
    Update_PDO(Usb_Port, 1, 5000, 1500);
    Update_PDO(Usb_Port, 2, New_PDO_Voltage, 1000); 
    status = PdMessage_SoftReset_WithTimeout(); //force the new PDO negociation, by resetting the Source (Vbus is not lost during the reset)
    
    if(status != 0) return status; //error
    
    return 0; //OK
}

#define PD_HEADER_SOFTRESET 0x000D

int PdMessage_SoftReset()
{
    //Sink send "Soft_reset" message to Source:
    //Set TX_header to soft reset: @x51 = x0D
    //Send PD_command: @x1A = x26
    
    int status = 0;
    uint8_t UsbPort = 0;
    uint8_t Data;
    
    //-------------------------------------
    
    // read CC pin Attachement status
    status = I2C_Read_USB_PD(STUSB45DeviceConf[UsbPort].I2cBus,STUSB45DeviceConf[UsbPort].I2cDeviceID_7bit ,PORT_STATUS ,&Data, 1);  if(status != 0) { return -1; }
    
    if( (Data & STUSBMASK_ATTACHED_STATUS) == VALUE_ATTACHED) //only if cable attached
    {
        uint16_t Data16 = PD_HEADER_SOFTRESET;
        uint8_t Data8[2];
        Data8[0] = Data16 & 0xFF;
        Data8[1] = (Data16 >> 8) & 0xFF;
        status = I2C_Write_USB_PD(STUSB45DeviceConf[UsbPort].I2cBus, STUSB45DeviceConf[UsbPort].I2cDeviceID_7bit, TX_HEADER, (uint8_t *)&Data8, 2 );
        if(status != 0) { return -1; }
        
        //-------------------------------------
        
        uint8_t New_CMD = 0x26;
        status = I2C_Write_USB_PD(STUSB45DeviceConf[UsbPort].I2cBus, STUSB45DeviceConf[UsbPort].I2cDeviceID_7bit, STUSB_GEN1S_CMD_CTRL, &New_CMD, 1 );
        if(status != 0) { return -1; }
    }
    else
    {
        return -2; //Error, cable not attached
    }
    
    return 0;  //OK
}

int PdMessage_SoftReset_WithTimeout()
{
    //Sink send "Soft_reset" message to Source:
    //Set TX_header to soft reset: @x51 = x0D
    //Send PD_command: @x1A = x26
    
    int status = 0;
    uint8_t UsbPort = 0;
    volatile int TimeoutEvent;
    uint8_t Data;
    
    uint32_t Tickstart = 0U;
    uint32_t TimeoutValue = 1000; //ms
    
    //-------------------------------------
    
    // read CC pin Attachement status
    status = I2C_Read_USB_PD(STUSB45DeviceConf[UsbPort].I2cBus,STUSB45DeviceConf[UsbPort].I2cDeviceID_7bit ,PORT_STATUS ,&Data, 1);
    if(status != 0) { return -1; }
    
    if( (Data & STUSBMASK_ATTACHED_STATUS) == VALUE_ATTACHED) //only if cable attached
    {
        uint16_t Data16 = PD_HEADER_SOFTRESET;
        uint8_t Data8[2];
        Data8[0] = Data16 & 0xFF;
        Data8[1] = (Data16 >> 8) & 0xFF;
        status = I2C_Write_USB_PD(STUSB45DeviceConf[UsbPort].I2cBus, STUSB45DeviceConf[UsbPort].I2cDeviceID_7bit, TX_HEADER, (uint8_t *)&Data8, 2 );
        if(status != 0) { return -1; }
        
        //-------------------------------------
        
        PostProcess_Msg_Accept = 0; //clear
        PostProcess_Msg_Reject = 0;  //clear
        TimeoutEvent = 0;  //clear
        
        /* Init tickstart for timeout management*/
        Tickstart = HAL_GetTick();
        
        //-------------------------------------
        
        uint8_t New_CMD = 0x26;
        status = I2C_Write_USB_PD(STUSB45DeviceConf[UsbPort].I2cBus, STUSB45DeviceConf[UsbPort].I2cDeviceID_7bit, STUSB_GEN1S_CMD_CTRL, &New_CMD, 1 );
        if(status != 0) { return -1; }
        
        //-------------------------------------
        
        while( (PostProcess_Msg_Accept == 0) && (PostProcess_Msg_Reject == 0) && (TimeoutEvent == 0)) //wait PD message is accepted by SOURCE
        {
            /* Check for the TimeoutEvent */
            if((HAL_GetTick() - Tickstart) > TimeoutValue)
            {
                TimeoutEvent = 1;
            }
            
        }
        
        //-------------------------------------
        
        if( PostProcess_Msg_Accept > 0)
        {
            return 0; //OK
        }
        else if( PostProcess_Msg_Reject > 0)
        {
            return -3; //Error
        }
        else if( TimeoutEvent > 0)
        {
            return -4; //Error
        }
        else
        {
            return -5; //Error
        }
        
    }
    else
    {
        return -2; //Error, cable not attached
    }
}

int GetSrcCap(uint8_t Usb_Port) //GetSourceCapabilities by sending a request or sw-reset to the USB-PD Source
{
    int status;
    
    Clear_PDO_FROM_SRC(Usb_Port); //clear ISR variables for sanity check
    
    status = PdMessage_SoftReset_WithTimeout(); //make the Source send its PDO Capabilities
    
    if(status != 0)
    {     
        printf("GetSrcCap Error\r\n");
        return status; //error
    }
    else
    {
        //Note: the PDO_FROM_SRC variable is updated during ISR
        
        Print_TypeC_MaxCurrentAt5V_FROM_SRC(Usb_Port);
        Print_PDO_FROM_SRC(Usb_Port);
    }
    
    return 0; //OK
}
