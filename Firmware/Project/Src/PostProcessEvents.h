int PostProcess_Events();
int Push_PD_MessageReceived(char MessageType, char MessageValue);
int Pop_PD_MessageReceived(void);
int Push_IrqReceived(char MessageType);
int Pop_IrqReceived(void);


extern volatile int PostProcess_AttachTransition;
extern volatile int PostProcess_PD_MessageReceived;
extern volatile int PostProcess_SRC_PDO_Received;
extern volatile int PostProcess_PSRDY_Received;
extern volatile int PostProcess_IrqReceived;


