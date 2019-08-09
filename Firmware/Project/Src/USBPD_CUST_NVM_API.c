#include "i2c.h"
#include "STUSB_NVM.h"
#include "USB_PD_defines_STUSB-GEN1S.h"

extern I2C_HandleTypeDef *hi2c[2];	
extern unsigned int I2cDeviceID_7bit;
extern unsigned int Address;
extern unsigned int AddressSize ;

#define STUSB4500_I2C_SLAVE_ADDR_7BIT  0x28
#define STUSB4500_I2C_SLAVE_ADDR_8BIT  0x50
#define I2C_Write_USB_PD(a,b,c,d) I2C_Write_USB_PD(a,STUSB4500_I2C_SLAVE_ADDR_7BIT, b,c,d)
#define I2C_Read_USB_PD(a,b,c,d) I2C_Read_USB_PD(a,STUSB4500_I2C_SLAVE_ADDR_7BIT, b,c,d)

void nvm_flash(uint8_t Port);
void CUST_EnterWriteMode(uint8_t Port,unsigned char ErasedSector);
void CUST_EnterReadMode(uint8_t Port);
void CUST_ReadSector(uint8_t Port,char SectorNum, unsigned char *SectorData);
void CUST_WriteSector(uint8_t Port,char SectorNum, unsigned char *SectorData);
void CUST_ExitTestMode(uint8_t Port);

void nvm_flash(uint8_t Port)
{
	CUST_EnterWriteMode(0, SECTOR_0 |SECTOR_1  |SECTOR_2 |SECTOR_3  | SECTOR_4 );

	CUST_WriteSector(0,0,Sector0);
	CUST_WriteSector(0,1,Sector1);
	CUST_WriteSector(0,2,Sector2);
	CUST_WriteSector(0,3,Sector3);
	CUST_WriteSector(0,4,Sector4);

	CUST_ExitTestMode(0);
}


/*
FTP Registers
o FTP_CUST_PWR (0x9E b(7), ftp_cust_pwr_i in RTL); power for FTP
o FTP_CUST_RST_N (0x9E b(6), ftp_cust_reset_n_i in RTL); reset for FTP
o FTP_CUST_REQ (0x9E b(4), ftp_cust_req_i in RTL); request bit for FTP operation
o FTP_CUST_SECT (0x9F (2:0), ftp_cust_sect1_i in RTL); for customer to select between sector 0 to 4 for read/write operations (functions as lowest address bit to FTP, remainders are zeroed out)
o FTP_CUST_SER[4:0] (0x9F b(7:4), ftp_cust_ser_i in RTL); customer Sector Erase Register; controls erase of sector 0 (00001), sector 1 (00010), sector 2 (00100), sector 3 (01000), sector 4 (10000) ) or all (11111).
o FTP_CUST_OPCODE[2:0] (0x9F b(2:0), ftp_cust_op3_i in RTL). Selects opcode sent to
FTP. Customer Opcodes are:
o 000 = Read sector
o 001 = Write Program Load register (PL) with data to be written to sector 0 or 1
o 010 = Write Sector Erase Register (SER) with data reflected by state of FTP_CUST_SER[4:0]
o 011 = Read Program Load register (PL)
o 100 = Read SER;
o 101 = Erase sector 0 to 4  (depending upon the mask value which has been programmed to SER)
o 110 = Program sector 0  to 4 (depending on FTP_CUST_SECT1)
o 111 = Soft program sector 0 to 4 (depending upon the value which has been programmed to SER)*/

void CUST_EnterWriteMode(uint8_t Port,unsigned char ErasedSector)
{
	unsigned char Buffer[2];

	Buffer[0]=FTP_CUST_PASSWORD;   /* Set Password*/
	I2C_Write_USB_PD(Port,FTP_CUST_PASSWORD_REG,Buffer,1);

	Buffer[0]= 0 ;   /* this register must be NULL for Partial Erase feature */
	I2C_Write_USB_PD(Port,RW_BUFFER,Buffer,1);

	{  
		//NVM Power-up Sequence
		//After STUSB start-up sequence, the NVM is powered off.
        
		Buffer[0]=0;  /* NVM internal controller reset */
		I2C_Write_USB_PD(Port,FTP_CTRL_0,Buffer,1);
        
		Buffer[0]=FTP_CUST_PWR | FTP_CUST_RST_N; /* Set PWR and RST_N bits */
		I2C_Write_USB_PD(Port,FTP_CTRL_0,Buffer,1);
	}
        
	Buffer[0]=((ErasedSector << 3) & FTP_CUST_SER) | ( WRITE_SER & FTP_CUST_OPCODE) ;  /* Load 0xF1 to erase all sectors of FTP and Write SER Opcode */
	I2C_Write_USB_PD(Port,FTP_CTRL_1,Buffer,1); /* Set Write SER Opcode */

	Buffer[0]=FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ ; 
	I2C_Write_USB_PD(Port,FTP_CTRL_0,Buffer,1); /* Load Write SER Opcode */
	do 
	{
	I2C_Read_USB_PD(Port,FTP_CTRL_0,Buffer,1); /* Wait for execution */
	}
	while(Buffer[0] & FTP_CUST_REQ); 
	Buffer[0]=  SOFT_PROG_SECTOR & FTP_CUST_OPCODE ;  
	I2C_Write_USB_PD(Port,FTP_CTRL_1,Buffer,1);  /* Set Soft Prog Opcode */

	Buffer[0]=FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ ; 
	I2C_Write_USB_PD(Port,FTP_CTRL_0,Buffer,1); /* Load Soft Prog Opcode */

	do 
	{
	I2C_Read_USB_PD(Port,FTP_CTRL_0,Buffer,1); /* Wait for execution */
	}
	while(Buffer[0] & FTP_CUST_REQ);
	Buffer[0]= ERASE_SECTOR & FTP_CUST_OPCODE ;  
	I2C_Write_USB_PD(Port,FTP_CTRL_1,Buffer,1); /* Set Erase Sectors Opcode */

	Buffer[0]=FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ ;  
	I2C_Write_USB_PD(Port,FTP_CTRL_0,Buffer,1); /* Load Erase Sectors Opcode */

	do 
	{
	I2C_Read_USB_PD(Port,FTP_CTRL_0,Buffer,1); /* Wait for execution */
	}
	while(Buffer[0] & FTP_CUST_REQ);	



}

void CUST_EnterReadMode(uint8_t Port)
{
	unsigned char Buffer[2];

	Buffer[0]=FTP_CUST_PASSWORD;  /* Set Password*/
	I2C_Write_USB_PD(Port,FTP_CUST_PASSWORD_REG,Buffer,1);
    
	{  
		//NVM Power-up Sequence
		//After STUSB start-up sequence, the NVM is powered off.
        
		Buffer[0]=0;  /* NVM internal controller reset */
		I2C_Write_USB_PD(Port,FTP_CTRL_0,Buffer,1);
        
		Buffer[0]=FTP_CUST_PWR | FTP_CUST_RST_N; /* Set PWR and RST_N bits */
		I2C_Write_USB_PD(Port,FTP_CTRL_0,Buffer,1);
	}
}

void CUST_ReadSector(uint8_t Port,char SectorNum, unsigned char *SectorData)
{
	unsigned char Buffer[2];

	Buffer[0]= FTP_CUST_PWR | FTP_CUST_RST_N; /* Set PWR and RST_N bits */
	I2C_Write_USB_PD(Port,FTP_CTRL_0,Buffer,1);

	Buffer[0]= (READ & FTP_CUST_OPCODE);
	I2C_Write_USB_PD(Port,FTP_CTRL_1,Buffer,1);/* Set Read Sectors Opcode */

	Buffer[0]= (SectorNum & FTP_CUST_SECT) |FTP_CUST_PWR |FTP_CUST_RST_N | FTP_CUST_REQ;
	I2C_Write_USB_PD(Port,FTP_CTRL_0,Buffer,1);  /* Load Read Sectors Opcode */
	do 
	{
	I2C_Read_USB_PD(Port,FTP_CTRL_0,Buffer,1); /* Wait for execution */
	}
	while(Buffer[0] & FTP_CUST_REQ);
	
	I2C_Read_USB_PD(Port,RW_BUFFER,&SectorData[0],8); /* Sectors Data are available in RW-BUFFER @ 0x53 */

	Buffer[0] = 0 ;  /* NVM internal controller reset */
	I2C_Write_USB_PD(Port,FTP_CTRL_0,Buffer,1);

		
}

void CUST_WriteSector(uint8_t Port,char SectorNum, unsigned char *SectorData)
{
	unsigned char Buffer[2];

//Write the 64-bit data to be written in the sector
	I2C_Write_USB_PD(Port,RW_BUFFER,SectorData,8);

	Buffer[0]=FTP_CUST_PWR | FTP_CUST_RST_N; /*Set PWR and RST_N bits*/
	I2C_Write_USB_PD(Port,FTP_CTRL_0,Buffer,1);

    //NVM Program Load Register to write with the 64-bit data to be written in sector
	Buffer[0]= (WRITE_PL & FTP_CUST_OPCODE); /*Set Write to PL Opcode*/
	I2C_Write_USB_PD(Port,FTP_CTRL_1,Buffer,1);

	Buffer[0]=FTP_CUST_PWR |FTP_CUST_RST_N | FTP_CUST_REQ;  /* Load Write to PL Sectors Opcode */  
	I2C_Write_USB_PD(Port,FTP_CTRL_0,Buffer,1);

	do 
	{
	I2C_Read_USB_PD(Port,FTP_CTRL_0,Buffer,1); /* Wait for execution */
	}		 
	while(Buffer[0] & FTP_CUST_REQ) ;  //FTP_CUST_REQ clear by NVM controller
	

	Buffer[0]= (PROG_SECTOR & FTP_CUST_OPCODE);
	I2C_Write_USB_PD(Port,FTP_CTRL_1,Buffer,1);/*Set Prog Sectors Opcode*/

	Buffer[0]=(SectorNum & FTP_CUST_SECT) |FTP_CUST_PWR |FTP_CUST_RST_N | FTP_CUST_REQ;
	I2C_Write_USB_PD(Port,FTP_CTRL_0,Buffer,1); /* Load Prog Sectors Opcode */  

	do 
	{
	I2C_Read_USB_PD(Port,FTP_CTRL_0,Buffer,1); /* Wait for execution */
	}
	while(Buffer[0] & FTP_CUST_REQ); //FTP_CUST_REQ clear by NVM controller

}

void CUST_ExitTestMode(uint8_t Port)
{
	unsigned char Buffer[2];

	Buffer[0]= FTP_CUST_RST_N; Buffer[1]=0x00;  /* clear registers */
	I2C_Write_USB_PD(Port,FTP_CTRL_0,Buffer,2);
	Buffer[0]=0x00; 
	I2C_Write_USB_PD(Port,FTP_CUST_PASSWORD_REG,Buffer,1);  /* Clear Password */

}



