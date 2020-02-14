/**
******************************************************************************
* File Name          : USB_PD_core.h
* Date               : 03/09/2015 10:51:46
* Description        : This file contains all the functions prototypes for 
*                      the USB_PD_core  
******************************************************************************
*
* COPYRIGHT(c) 2015 STMicroelectronics
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_PD_CORE_H
#define __USB_PD_CORE_H
#ifdef __cplusplus
extern "C" {
#endif
  
  /* Includes ------------------------------------------------------------------*/
#if defined (STM32L476xx)
#include "stm32l4xx_hal.h"
#elif defined(STM32F072xB)
#include "stm32f0xx_hal.h"
#elif defined(STM32F401xE)
#include "stm32f4xx_hal.h"
  
#endif


#include "USB_PD_defines_STUSB-GEN1S.h"
  
#define USBPORT_MAX 1
#define I2CBUS_MAX 1
#define SRC_PDO_NUM_MIN 1
#define SRC_PDO_NUM_MAX 7

#define true 1
#define false 0    
  
#define LE16(addr) (((uint16_t)(*((uint8_t *)(addr))))\
  + (((uint16_t)(*(((uint8_t *)(addr)) + 1))) << 8))

#define LE32(addr) ((((uint32_t)(*(((uint8_t *)(addr)) + 0))) + \
(((uint32_t)(*(((uint8_t *)(addr)) + 1))) << 8) + \
  (((uint32_t)(*(((uint8_t *)(addr)) + 2))) << 16) + \
    (((uint32_t)(*(((uint8_t *)(addr)) + 3))) << 24)))


typedef uint8_t bool;


  typedef struct  
    {
      uint8_t Usb_Port;
      uint8_t I2cBus ;
      uint8_t I2cDeviceID_7bit;
      uint8_t Dev_Cut;
      uint8_t Alert_GPIO_Pin;
      uint8_t Alert_GPIO_Bank;
    }USB_PD_I2C_PORT;


typedef struct  
  {
    uint8_t                                         HW_Reset ; 
    STUSB_GEN1S_CC_DETECTION_STATUS_TRANS_RegTypeDef Port_Status_Trans;
    STUSB_GEN1S_CC_DETECTION_STATUS_RegTypeDef      Port_Status;/*!< Specifies the Port status register */
    uint8_t TypeC;
    STUSB_GEN1S_CC_STATUS_RegTypeDef                CC_status;
    STUSB_GEN1S_MONITORING_STATUS_RegTypeDef        Monitoring_status;         /*!< Specifies the  */
    STUSB_GEN1S_HW_FAULT_STATUS_RegTypeDef          HWFault_status;
    STUSB_GEN1S_PRT_STATUS_RegTypeDef               PRT_status;    /*!< Specifies t */
    STUSB_GEN1S_PHY_STATUS_RegTypeDef               Phy_status;     /*!<  */
    
    
  }USB_PD_StatusTypeDef;

typedef struct
  {
    uint8_t Role;
    uint8_t Device;
    uint8_t Command;
    uint8_t Port_Tx;
    uint8_t Port_Rx;
    uint8_t I_SEL_VCONN;
    uint8_t TOP;
    uint8_t Phy_CTRL;     /*!<  */
    bool PDO_Ready;
    bool RDO_Ready;
    bool VDM_Ready;
    bool DR_Swap_en ;
    bool PR_Swap_en ;
    bool VCONN_Swap_en;
    bool VCONN_en;
    uint8_t TypeC_req;
    uint8_t Messages_Info;
    uint32_t Monitor;
    uint8_t Discharge_time_to0;
    uint8_t Discharge_time_transition;
  }USB_PD_CTRLTypeDef;
/** @defgroup USBPD_MsgHeaderStructure_definition USB PD Message header Structure definition
* @brief USB PD Message header Structure definition
* @{
*/
typedef union
  {
    uint16_t d16;
    struct
      {
#if defined(USBPD_REV30_SUPPORT)
        uint16_t MessageType :                  /*!< Message Header's message Type                      */
          5;
#else /* USBPD_REV30_SUPPORT */
          uint16_t MessageType :                  /*!< Message Header's message Type                      */
            4;
            uint16_t Reserved4 :                    /*!< Reserved                                           */
              1;
#endif /* USBPD_REV30_SUPPORT */
              uint16_t PortDataRole :                 /*!< Message Header's Port Data Role                    */
                1;                                    
                uint16_t SpecificationRevision :        /*!< Message Header's Spec Revision                     */
                  2;       
                  uint16_t PortPowerRole_CablePlug :      /*!< Message Header's Port Power Role/Cable Plug field  */
                    1;       
                    uint16_t MessageID :                    /*!< Message Header's message ID                        */
                      3;    
                      uint16_t NumberOfDataObjects :          /*!< Message Header's Number of data object             */
                        3;                
                        uint16_t Extended :                     /*!< Reserved                                           */
                          1; 
      }
    b;
  } USBPD_MsgHeader_TypeDef;




typedef union 
  {
    uint32_t d32;        
    struct
      {       
        //Table 6-9 Fixed Supply PDO - Source
        uint32_t Max_Operating_Current :10; //Bits 9..0
        uint32_t Voltage :10;  //Bits 19..10
        uint8_t PeakCurrent:2; //Bits 21..20
        uint8_t Reserved :3;
        uint8_t DataRoleSwap:1; //Bits 25
        uint8_t Communication:1; //Bits 26
        uint8_t ExternalyPowered:1; //Bits 27
        uint8_t SuspendSuported:1;  //Bits 28
        uint8_t DualRolePower :1;  //Bits 29
        uint8_t FixedSupply:2;  //Bits 31..30
      } fix;		
    
    struct
      {
        //Table 6-11 Variable Supply (non-Battery) PDO - Source
        uint32_t Operating_Current :10;
        uint32_t Min_Voltage:10;
        uint32_t Max_Voltage:10;
        uint8_t VariableSupply:2; 
      }var;
    
    struct
      {
        //Table 6-12 Battery Supply PDO - Source
        uint32_t Operating_Power :10;
        uint32_t Min_Voltage:10;
        uint32_t Max_Voltage:10;
        uint8_t Battery:2; 
      }bat;   
    
  } USB_PD_SRC_PDOTypeDef;



typedef struct 
  {
    uint8_t PHY;
    uint8_t PRL;
    uint8_t BIST;
    uint8_t PE;
    uint8_t TypeC;      
  }USB_PD_Debug_FSM_TypeDef;


void HW_Reset_state(uint8_t Usb_Port);
int SW_reset_by_Reg(uint8_t Usb_Port);
int usb_pd_init(uint8_t Usb_Port);
void ALARM_MANAGEMENT(uint8_t Usb_Port);
void Read_SNK_PDO(uint8_t Usb_Port);
void Print_SNK_PDO(uint8_t Usb_Port);
void Print_PDO_FROM_SRC(uint8_t Usb_Port);
void Read_RDO(uint8_t Usb_Port);
int Get_RDO(uint8_t UsbPort, int * out_PDO_nb , int * out_Voltage_mV, int * out_Current_mA, int * out_MaxCurrent_mA);
void Print_RDO(uint8_t Usb_Port);
int Update_PDO(uint8_t Usb_Port,uint8_t PDO_Number,int Voltage,int Current);
int Update_Valid_PDO_Number(uint8_t Usb_Port,uint8_t Number_PDO);
int Get_current_Sink_PDO_Numb(uint8_t UsbPort, uint8_t * out_PDO_Count);
int Find_Matching_SRC_PDO(uint8_t Usb_Port,int Min_Power,int Min_V , int Max_V);


void Set_New_PDO_case1(uint8_t Usb_Port);
void Negotiate_5V(uint8_t Usb_Port);
//void Print_PDO_FROM_SRC(uint8_t Usb_Port);
int CheckCableAttachementStatus(void);
void Clear_PDO_FROM_SRC(uint8_t Usb_Port);
int Print_TypeC_MaxCurrentAt5V_FROM_SRC(uint8_t Usb_Port);
void Print_requested_PDO_Voltage(void);

int CheckCableAttached();
int Change_PDO_WithoutLosingVbus(unsigned int New_PDO_Voltage);
int Change_PDO_WithoutLosingVbus_WithTimeout(unsigned int New_PDO_Voltage);
int PdMessage_SoftReset();
int PdMessage_SoftReset_WithTimeout();
int GetSrcCap(uint8_t Usb_Port); //GetSourceCapabilities

#ifdef __cplusplus
}
#endif

#endif /*usbpd core header */

/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
