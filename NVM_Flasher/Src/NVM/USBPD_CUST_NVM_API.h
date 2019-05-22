
int nvm_flash();
int nvm_read(unsigned char * pSectorsOut, int SectorsLength);

int CUST_EnterWriteMode(uint8_t Port,unsigned char ErasedSector);
int CUST_EnterReadMode(uint8_t Port);
int CUST_ReadSector(uint8_t Port,char SectorNum, unsigned char *SectorData);
int CUST_WriteSector(uint8_t Port,char SectorNum, unsigned char *SectorData);
int CUST_ExitTestMode(uint8_t Port);
