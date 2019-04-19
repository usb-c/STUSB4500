
//From USB-PD specification R3.0 v1.1


//Figure 6-2 USB Power Delivery Packet Format including Data Message Payload
// Preamble + SOP* + MessageHeader(16bit LSB+MSB) + DataObject 0..7 (32bit LSB+8+8+MSB) + CRC + EOP


//Table 6-1 Message Header (16 bit)
#define USBPD_MSGHEAD_MASK_Extended              (0x1 << 15) //Bits 15
#define USBPD_MSGHEAD_MASK_NumberOfDataObjects   (0x3 << 12) //Bits 14..12
#define USBPD_MSGHEAD_MASK_MessageID             (0x3 << 9)  //Bits 11..9
#define USBPD_MSGHEAD_MASK_PortPowerRole         (0x1 << 8)  //Bits 8   // SOP only
#define USBPD_MSGHEAD_MASK_CablePlug             (0x1 << 8)  //Bits 8   // SOP’/SOP’’
#define USBPD_MSGHEAD_MASK_SpecificationRevision (0x3 << 6)  //Bits 7..6
#define USBPD_MSGHEAD_MASK_PortDataRole          (0x1 << 5)  //Bits 5   // SOP only
#define USBPD_MSGHEAD_MASK_Reserved              (0x1 << 5)  //Bits 5   // SOP’/SOP’’
#define USBPD_MSGHEAD_MASK_MessageType           (0x1F)      //Bits 4..0


#define USBPD_MSGHEAD_Extended(x)              ((x >> 15) & 0x01) //Bits 15
#define USBPD_MSGHEAD_NumberOfDataObjects(x)   ((x >> 12) & 0x03) //Bits 14..12
#define USBPD_MSGHEAD_MessageID(x)             ((x >> 9) & 0x03)  //Bits 11..9
#define USBPD_MSGHEAD_PortPowerRole(x)         ((x >> 8) & 0x01)  //Bits 8   // SOP only
#define USBPD_MSGHEAD_CablePlug(x)             ((x >> 8) & 0x01)  //Bits 8   // SOP’/SOP’’
#define USBPD_MSGHEAD_SpecificationRevision(x) ((x >> 6) & 0x03)  //Bits 7..6
#define USBPD_MSGHEAD_PortDataRole(x)          ((x >> 5) & 0x01)  //Bits 5   // SOP only
#define USBPD_MSGHEAD_Reserved(x)              ((x >> 5) & 0x01)  //Bits 5   // SOP’/SOP’’
#define USBPD_MSGHEAD_MessageType(x)           ( x & 0x1F)        //Bits 4..0



//Table 6-9 Fixed Supply PDO - Source
#define USBPD_FIXPDOSRC_MASK_FixedSupply                 (0x3 << 30)  //Bits 31..30
#define USBPD_FIXPDOSRC_MASK_DualRolePower               (0x1 << 29)  //Bits 29
#define USBPD_FIXPDOSRC_MASK_UsbSuspendSupported         (0x1 << 28)  //Bits 28
#define USBPD_FIXPDOSRC_MASK_UnconstrainedPower          (0x1 << 27)  //Bits 27
#define USBPD_FIXPDOSRC_MASK_UsbCommunicationsCapable    (0x1 << 26)  //Bits 26
#define USBPD_FIXPDOSRC_MASK_DualRoleData                (0x1 << 25)  //Bits 25
#define USBPD_FIXPDOSRC_MASK_UnchunkedExtendedMessagesSupported (0x1 << 24)  //Bits 24
#define USBPD_FIXPDOSRC_MASK_Reserved                    (0x3 << 22)  //Bits 23..22
#define USBPD_FIXPDOSRC_MASK_PeakCurrent                 (0x3 << 20)  //Bits 21..20
#define USBPD_FIXPDOSRC_MASK_Voltage_in50mV_units        (0x3FF << 10) //Bits 19..10
#define USBPD_FIXPDOSRC_MASK_MaximumCurrent_in10mA_units (0x3FF)       //Bits 9..0

#define USBPD_FIXPDOSRC_FixedSupply(x)                 ((x >> 30) & 0x03)  //Bits 31..30
#define USBPD_FIXPDOSRC_DualRolePower(x)               ((x >> 29) & 0x01)  //Bits 29
#define USBPD_FIXPDOSRC_UsbSuspendSupported(x)         ((x >> 28) & 0x01)  //Bits 28
#define USBPD_FIXPDOSRC_UnconstrainedPower(x)          ((x >> 27) & 0x01)  //Bits 27
#define USBPD_FIXPDOSRC_UsbCommunicationsCapable(x)    ((x >> 26) & 0x01)  //Bits 26
#define USBPD_FIXPDOSRC_DualRoleData(x)                ((x >> 25) & 0x01)  //Bits 25
#define USBPD_FIXPDOSRC_UnchunkedExtendedMessagesSupported(x) ((x >> 24) & 0x01)  //Bits 24
#define USBPD_FIXPDOSRC_Reserved(x)                    ((x >> 22) & 0x03)  //Bits 23..22
#define USBPD_FIXPDOSRC_PeakCurrent(x)                 ((x >> 20) & 0x03)  //Bits 21..20
#define USBPD_FIXPDOSRC_Voltage_in50mV_units(x)        ((x >> 10) & 0x3FF)  //Bits 19..10
#define USBPD_FIXPDOSRC_MaximumCurrent_in10mA_units(x) ( x & 0x3FF)         //Bits 9..0


//Table 6-11 Variable Supply (non-Battery) PDO - Source
//...

//Table 6-12 Battery Supply PDO - Source
//...


//Table 6-14 Fixed Supply PDO - Sink
#define USBPD_FIXPDOSNK_MASK_FixedSupply                (0x3 << 30)  //Bits 31..30
#define USBPD_FIXPDOSNK_MASK_DualRolePower              (0x1 << 29)  //Bits 29
#define USBPD_FIXPDOSNK_MASK_HigherCapability           (0x1 << 28)  //Bits 28
#define USBPD_FIXPDOSNK_MASK_UnconstrainedPower         (0x1 << 27)  //Bits 27
#define USBPD_FIXPDOSNK_MASK_UsbCommunicationsCapable   (0x1 << 26)  //Bits 26
#define USBPD_FIXPDOSNK_MASK_DualRoleData               (0x1 << 25)  //Bits 25
#define USBPD_FIXPDOSNK_MASK_FastRoleSwapRequiredUsbcCurrent (0x3 << 23) //Bits 24..23
#define USBPD_FIXPDOSNK_MASK_Reserved                   (0x3 << 20)  //Bits 22..20
#define USBPD_FIXPDOSNK_MASK_Voltage_in50mV_units       (0x3FF << 10) //Bits 19..10
#define USBPD_FIXPDOSNK_MASK_OperationalCurrent_in10mA_units (0x3FF)  //Bits 9..0

//Table 6-18 Fixed and Variable Request Data Object



//Table 6-5 Control Message Types
#define USBPD_CTRLMSG_Reserved1                 0x00
#define USBPD_CTRLMSG_GoodCRC                   0x01
#define USBPD_CTRLMSG_GotoMin                   0x02
#define USBPD_CTRLMSG_Accept                    0x03
#define USBPD_CTRLMSG_Reject                    0x04
#define USBPD_CTRLMSG_Ping                      0x05
#define USBPD_CTRLMSG_PS_RDY                    0x06
#define USBPD_CTRLMSG_Get_Source_Cap            0x07
#define USBPD_CTRLMSG_Get_Sink_Cap              0x08
#define USBPD_CTRLMSG_DR_Swap                   0x09
#define USBPD_CTRLMSG_PR_Swap                   0x0A
#define USBPD_CTRLMSG_VCONN_Swap                0x0B
#define USBPD_CTRLMSG_Wait                      0x0C
#define USBPD_CTRLMSG_Soft_Reset                0x0D
#define USBPD_CTRLMSG_Reserved2                 0x0E
#define USBPD_CTRLMSG_Reserved3                 0x0F
#define USBPD_CTRLMSG_Not_Supported             0x10
#define USBPD_CTRLMSG_Get_Source_Cap_Extended   0x11
#define USBPD_CTRLMSG_Get_Status                0x12
#define USBPD_CTRLMSG_FR_Swap                   0x13
#define USBPD_CTRLMSG_Get_PPS_Status            0x14
#define USBPD_CTRLMSG_Get_Country_Codes         0x15
#define USBPD_CTRLMSG_Reserved4                 0x16
#define USBPD_CTRLMSG_Reserved5                 0x1F



//Table 6-6 Data Message Types
#define USBPD_DATAMSG_Reserved1             0x00
#define USBPD_DATAMSG_Source_Capabilities   0x01
#define USBPD_DATAMSG_Request               0x02
#define USBPD_DATAMSG_BIST                  0x03
#define USBPD_DATAMSG_Sink_Capabilities     0x04
#define USBPD_DATAMSG_Battery_Status        0x05
#define USBPD_DATAMSG_Alert                 0x06
#define USBPD_DATAMSG_Get_Country_Info      0x07
#define USBPD_DATAMSG_Reserved2             0x08
#define USBPD_DATAMSG_Reserved3             0x0E
#define USBPD_DATAMSG_Vendor_Defined        0x0F
#define USBPD_DATAMSG_Reserved4             0x10
#define USBPD_DATAMSG_Reserved5             0x1F




