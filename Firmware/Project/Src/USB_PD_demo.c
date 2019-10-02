
#include "USB_PD_demo.h"
#include "USB_PD_core.h"

#include "PostProcessEvents.h"

//Demo USB PD clicker
//Example to automatically select the next PDO from Source, each time we press the Button

int Select_Next_PDO_SRC(uint8_t UsbPort, int * out_NewPDO_mV)
{
    extern USB_PD_SRC_PDOTypeDef PDO_FROM_SRC[USBPORT_MAX][7];
    extern uint8_t PDO_FROM_SRC_Num[USBPORT_MAX];
    
    int PDO_mV;
    int PDO_I_mA;
    
    int PDO_SRC_selected;
    uint8_t PDO_SNK_selected;
    int PDO_count = 0;
    int status = -1;
    volatile int Timeout = 0; //to be implemented
    
    if(CheckCableAttached() < 0)
    {
        return -9; //error, cable not attached
    }
    
    if(PDO_FROM_SRC_Num[UsbPort] >= 1 )
    {
        status = Get_current_Sink_PDO_Numb(UsbPort, &PDO_SNK_selected); //current PDO on Sink side
        if(status != 0) return -1;
        
        int Voltage_mV; 
        int Current_mA;
        int MaxCurrent_mA;
        int status;
        status = Get_RDO(UsbPort, &PDO_SRC_selected, &Voltage_mV, &Current_mA, &MaxCurrent_mA);
        
        
        PDO_SRC_selected++; //select next one
        if(PDO_SRC_selected > PDO_FROM_SRC_Num[UsbPort])
            PDO_SRC_selected = 1;
        
        int idx = PDO_SRC_selected - 1;
        PDO_mV = PDO_FROM_SRC[UsbPort][idx].fix.Voltage * 50;
        PDO_I_mA = PDO_FROM_SRC[UsbPort][idx].fix.Max_Operating_Current * 10;
        
        
        if(PDO_SRC_selected == 1) //PDO1: 5V
        {
            PDO_count = 1;
            PDO_SNK_selected = 1;
        }
        else //PDO2 or PDO3
        {
            PDO_count = 2;
            PDO_SNK_selected = 2;
        }
        
        
        Update_PDO( UsbPort, PDO_SNK_selected , PDO_mV , PDO_I_mA );
        Update_Valid_PDO_Number(UsbPort, PDO_count);
        
        
        //Reset to take into account the new PDO
        //-----------------------------------
        
#if 1  //PD Protocol reset: VBUS is kept alive
        PostProcess_PSRDY_Received = 0;  //clear
        
        status = PdMessage_SoftReset();
        if(status != 0) return -3; //Error, SoftReset was rejected by SRC
        
        while( (PostProcess_PSRDY_Received == 0) && (Timeout == 0) ); //wait VBUS Voltage ready
        
#else  //Chip reset: VBUS is lost for 1sec
        SW_reset_by_Reg(UsbPort);
        usb_pd_init(UsbPort); //refresh main registers & IRQ mask init needed after reset
#endif
        //-----------------------------------
        
        if(out_NewPDO_mV != NULL)
        {
            *out_NewPDO_mV = PDO_mV;
        }
        
    }
    else //(PDO_FROM_SRC_Num[UsbPort] == 0 )
    {
        status = -2; //Error
    }
    
    return status;
}
