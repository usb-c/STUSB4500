/*Identification of STUSB */
#define DEVICE_ID 0x2F
#define EXTENTION_10  (uint8_t )0x80
#define ID_Reg (uint8_t )0x1C 
#define CUT (uint8_t )3<<2
#define CUT_A (uint8_t )4 << 2 
#define CUT_gen1S (uint8_t )8 << 2 

#define DEV_CUT (uint8_t )0x03

/*NVM FLasher Registers Definition */

#define FTP_CUST_PASSWORD_REG	0x95
#define FTP_CUST_PASSWORD		0x47
#define FTP_CTRL_0              0x96
	#define FTP_CUST_PWR	0x80 
	#define FTP_CUST_RST_N	0x40
	#define FTP_CUST_REQ	0x10
	#define FTP_CUST_SECT 0x07
#define FTP_CTRL_1              0x97
	#define FTP_CUST_SER_MASK 0xF8
	#define FTP_CUST_OPCODE_MASK 0x07
#define RW_BUFFER 0x53
	

/* FTP_CUST_OPCODE field values */
#define READ            0x00 //Read memory array
#define WRITE_PL        0x01 //Shift In Data on Program Load (PL) Register
#define WRITE_SER       0x02 //Shift In Data on Sector Erase (SER) Register 
#define READ_PL	        0x03 //Shift Out Data on Program Load (PL) Register
#define READ_SER        0x04 //Shift Out Data on sector Erase (SER) Register
#define ERASE_SECTOR    0x05 //Erase memory array
#define PROG_SECTOR     0x06 //Program 256b word into EEPROM
#define SOFT_PROG_SECTOR 0x07 //Soft Program array

/* FTP_CUST_SER field values */
#define	SECTOR_0	0x01
#define	SECTOR_1	0x02
#define	SECTOR_2	0x04
#define	SECTOR_3	0x08
#define	SECTOR_4	0x10








