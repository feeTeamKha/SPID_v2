#include <Flash_Func.h>
#include <USB_App.h>

Flashdef Flash_data;


/* Flash define */
//#define DATA_START_ADDRESS 		 	((uint32_t)0x0801FC00)	//Page 127
//#define LENGTH_START_ADDRESS 		((uint32_t)0x0801F810)	//Page 126

#define BIT0	0x01
#define BIT1	0x02
#define BIT2	0x04
#define BIT4	0x10
#define BIT6	0x40

/********************* Flash WriteSector **********************************/
int  writeSector(uint32_t Address,void * values, uint16_t size)
{
    uint16_t *AddressPtr;
    uint16_t *valuePtr;

    AddressPtr 	= (uint16_t *) Address;
    valuePtr		= (uint16_t *) values;
    size 				= size / 2;  // incoming value is expressed in bytes, not 16 bit words

    while(size)
    {
        // unlock the flash
        // Key 1 : 0x45670123
        // Key 2 : 0xCDEF89AB
        FLASH->KEYR = 0x45670123;
        FLASH->KEYR = 0xCDEF89AB;
        FLASH->CR &= ~BIT1; // ensure PER is low
        FLASH->CR |= BIT0;  // set the PG bit
        *(AddressPtr) = *(valuePtr);
        while(FLASH->SR & BIT0); // wait while busy
        if (FLASH->SR & BIT2)	return -1; // flash not erased to begin with
        if (FLASH->SR & BIT4)	return -2; // write protect error
        AddressPtr++;
        valuePtr++;
        size--;
    }
    return 0;
}

/********************* Flash eraseSector **********************************/
void eraseSector(uint32_t SectorStartAddress)
{
    FLASH->KEYR = 0x45670123;
    FLASH->KEYR = 0xCDEF89AB;
    FLASH->CR &= ~BIT0;  // Ensure PG bit is low
    FLASH->CR |= BIT1; // set the PER bit
    FLASH->AR = SectorStartAddress;
    FLASH->CR |= BIT6; // set the start bit
    while(FLASH->SR & BIT0); // wait while busy
}
/********************* Flash readSector **********************************/
void readSector(uint32_t SectorStartAddress, void * values, uint16_t size)
{
    uint16_t 			*AddressPtr;
    uint16_t 			*valuePtr;
    AddressPtr 	= (uint16_t *) SectorStartAddress;
    valuePtr		= (uint16_t *) values;
    size 				=  size/2; // incoming value is expressed in bytes, not 16 bit words

    while(size)
    {
        *((uint16_t *)valuePtr)=*((uint16_t *)AddressPtr);
        valuePtr++;
        AddressPtr++;
        size--;
    }
}

static void Init_param_PID(Flashdef *fl)
{
    fl->data_buffer[0] = (uint8_t *)&usbPID.dataIn[37];//&PID.mode;
    fl->data_buffer[1] = (uint8_t *)&usbPID.dataIn[43];//&PID.ID;
    fl->data_buffer[2] = (uint8_t *)&usbPID.dataIn[14];//(uint8_t*)&PID.PPR;
    fl->data_buffer[3] = (uint8_t *)&usbPID.dataIn[15];//(uint8_t*)&PID.PPR + 1;
    fl->data_buffer[4] = (uint8_t *)&usbPID.dataIn[16];//(uint8_t*)&PID.RPM;
    fl->data_buffer[5] = (uint8_t *)&usbPID.dataIn[17];//(uint8_t*)&PID.RPM + 1;
//    fl->data_buffer[6] = (uint8_t *)&usbPID.dataIn[3];//(uint8_t*)&PID.Delta_T;
//    fl->data_buffer[7] = (uint8_t *)&usbPID.dataIn[3];//(uint8_t*)&PID.Delta_T + 1;
//    fl->data_buffer[8] = (uint8_t *)&usbPID.dataIn[3];//uint8_t*)&PID.Delta_T + 2;
//    fl->data_buffer[9] = (uint8_t *)&usbPID.dataIn[3];//(uint8_t*)&PID.Delta_T + 3;
    fl->data_buffer[10] = (uint8_t *)&usbPID.dataIn[18];//(uint8_t*)&PID.Kp_def;
    fl->data_buffer[11] = (uint8_t *)&usbPID.dataIn[19];//(uint8_t*)&PID.Kp_def + 1;
    fl->data_buffer[12] = (uint8_t *)&usbPID.dataIn[20];//(uint8_t*)&PID.Kp_def + 2;
    fl->data_buffer[13] = (uint8_t *)&usbPID.dataIn[21];//(uint8_t*)&PID.Kp_def + 3;
    fl->data_buffer[14] = (uint8_t *)&usbPID.dataIn[22];//(uint8_t*)&PID.Ki_def;
    fl->data_buffer[15] = (uint8_t *)&usbPID.dataIn[23];//(uint8_t*)&PID.Ki_def + 1;
    fl->data_buffer[16] = (uint8_t *)&usbPID.dataIn[24];//(uint8_t*)&PID.Ki_def + 2;
    fl->data_buffer[17] = (uint8_t *)&usbPID.dataIn[25];//(uint8_t*)&PID.Ki_def + 3;
    fl->data_buffer[18] = (uint8_t *)&usbPID.dataIn[27];//(uint8_t*)&PID.Kd_def;
    fl->data_buffer[19] = (uint8_t *)&usbPID.dataIn[28];//(uint8_t*)&PID.Kd_def + 1;
    fl->data_buffer[20] = (uint8_t *)&usbPID.dataIn[29];//(uint8_t*)&PID.Kd_def + 2;
    fl->data_buffer[21] = (uint8_t *)&usbPID.dataIn[30];//(uint8_t*)&PID.Kd_def + 3;
    fl->data_buffer[22] = (uint8_t *)&usbPID.dataIn[38];//(uint8_t*)&PID.ptlCtrl;
    fl->data_buffer[23] = (uint8_t *)&usbPID.dataIn[51];//(uint8_t*)&PID.scalefactor;
    fl->data_buffer[24] = (uint8_t *)&usbPID.dataIn[39];//(uint8_t*)&PID.rev_UART.baudSelected;
}

static void Write_param_PID(Flashdef *fl)
{
    fl->data_wr[0] = *fl->data_buffer[0];//mode
    fl->data_wr[1] = *fl->data_buffer[1];//ID
    eraseSector(SAVE_PID_Init);
    writeSector(SAVE_PID_Init,fl->data_wr,2);

    for(uint8_t i = 2; i < DATA_SIZE_FL - 2; i++)
        fl->data_wr[i] = *fl->data_buffer[i];

    eraseSector(fl->address_flash);
    writeSector(fl->address_flash,(fl->data_wr + 2),DATA_SIZE_FL-2);
}
static void Read_param_PID(Flashdef *fl)
{
    readSector(SAVE_PID_Init,fl->data_rd,2);
    static uint8_t fist_read = 0;
    if(fist_read == 0)
    {
        switch(*(uint8_t*)(fl->data_rd))
        {
        case PID_Speed:
            fl->address_flash = SAVE_PID_Speed_Addr;
            break;
        case PID_Position:
            fl->address_flash = SAVE_PID_Position_Addr;
            break;
        default:
            fl->address_flash = SAVE_PID_Speed_Addr;
            break;
        }
        fist_read = 1;
    }
    readSector(fl->address_flash,(uint16_t*)(fl->data_rd + 2),DATA_SIZE_FL-2);
}

static void Change_mode_PID(Flashdef *fl, PID_typedef *pid)
{
    uint32_t addr_save = (PID.mode == PID_Position) ?
                         SAVE_PID_Position_Addr : SAVE_PID_Speed_Addr;

    readSector(addr_save,(uint16_t*)(fl->data_rd + 2),DATA_SIZE_FL-2);

    PID.PPR     = *(uint16_t *)(fl->data_rd + 2);
    PID.RPM     = *(uint16_t *)(fl->data_rd + 4);
    if(PID.mode == PID_Position)
    {
        PID.Kp_def  = *(float *)(fl->data_rd + 10);
        PID.Ki_def  = *(float *)(fl->data_rd + 14);
        PID.Kd_def  = *(float *)(fl->data_rd + 18);
    }
}

void Flash_data_init(Flashdef *fl)
{
    fl->f_init_param  = Init_param_PID;
    fl->f_write_param = Write_param_PID;
    fl->f_read_param  = Read_param_PID;
    fl->f_change_mode = Change_mode_PID;
}