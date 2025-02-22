#include "flash.h"

uint8_t lengthPage;

void deleteBuffer(char* data)
{
    uint8_t len = strlen(data);
    for(uint8_t i = 0; i < len; i++)
    {
        data[i] = 0;
    }
}

void Flash_Lock()
{
    HAL_FLASH_Lock();
}

void Flash_Unlock()
{
    HAL_FLASH_Unlock();
}

void Flash_Write_Float(uint32_t address, float f)
{
    Flash_Unlock();
    uint8_t data[4];
    *(float*)data = f;
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, *(uint32_t*)data);
    Flash_Lock();
}

float Flash_Read_Float(uint32_t address)
{
    uint32_t data = *(__IO uint32_t*)(address);
    return *(float*)(&data);
}

void Flash_Erase(uint32_t addr)
{
    Flash_Unlock();
    while((FLASH->SR&FLASH_SR_BSY));
    FLASH->CR |= FLASH_CR_PER; //Page Erase Set
    FLASH->AR = addr; //Page Address
    FLASH->CR |= FLASH_CR_STRT; //Start Page Erase
    while((FLASH->SR&FLASH_SR_BSY));
    FLASH->CR &= ~FLASH_SR_BSY;
    FLASH->CR &= ~FLASH_CR_PER; //Page Erase Clear
}

void Flash_Write_Int(uint32_t addr, int data)
{
    Flash_Unlock();
    FLASH->CR |= FLASH_CR_PG;				/*!< Programming */
    while((FLASH->SR&FLASH_SR_BSY));
    *(__IO uint16_t*)addr = data;
    while((FLASH->SR&FLASH_SR_BSY));
    FLASH->CR &= ~FLASH_CR_PG;
    Flash_Lock();
}

uint16_t Flash_Read_Int(uint32_t addr)
{
    uint16_t* val = (uint16_t *)addr;
    return *val;
}

void Flash_Write_Char(uint32_t addr, char* data)
{
    Flash_Unlock();
    int i;
    FLASH->CR |= FLASH_CR_PG;
    int var = 0;
    lengthPage = strlen(data);
    for(i=0; i<lengthPage; i+=1)
    {
        while((FLASH->SR&FLASH_SR_BSY));
        var = (int)data[i];
        *(__IO uint16_t*)(addr + i*2) = var;
    }
    while((FLASH->SR&FLASH_SR_BSY)) {};
    FLASH->CR &= ~FLASH_CR_PG;
    FLASH->CR |= FLASH_CR_LOCK;
}

void Flash_ReadChar(char* dataOut, uint32_t addr1, uint32_t addr2)
{
    int check = 0;
    deleteBuffer(dataOut);
    if((unsigned char)Flash_Read_Int(addr2+(uint32_t)2) == 255)
    {
        check = (unsigned char)Flash_Read_Int(addr2)-48;
    }
    else
    {
        check = ((unsigned char)Flash_Read_Int(addr2)-48)*10 + (unsigned char)Flash_Read_Int(addr2+2)-48;
    }
    for(int i = 0; i < check; i++)
    {
        dataOut[i] = Flash_Read_Int(addr1 + (uint32_t)(i*2));
    }
}
void Flash_Write_Array(uint32_t address, uint16_t *arr, uint16_t len)
{
    HAL_FLASH_Unlock();
    uint16_t *pt = (uint16_t*)arr;
    for(uint16_t i=0; i<(len+1)/2; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address + 2*i, *pt);
        pt++;
    }
    HAL_FLASH_Lock();
}
void Flash_Read_Array(uint32_t address, uint16_t *arr, uint16_t len)
{
    for(uint16_t i=0; i<(len+1)/2; i++)
    {
        *arr = *(__IO uint16_t*)(address + 2*i);
        *arr++;
    }
}
void Flash_Write_Uint8_t(uint32_t FlashAddress, uint8_t* Data, uint16_t Len)
{
    uint32_t word = 0;

    // Unlock the Flash to enable the flash control register access
    HAL_FLASH_Unlock();

    // Clear all error flags
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);

    for (uint16_t i = 0; i < Len; i += 2) {
        // Combine two bytes into one word (32-bit)
        word = Data[i] | (Data[i + 1] << 8);

        // Write word to flash
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FlashAddress, word);

        FlashAddress += 2;
    }

    // Lock the Flash to disable the flash control register access
    HAL_FLASH_Lock();
}
void Flash_Read_Uint8_t(uint32_t FlashAddress, uint8_t* Data, uint16_t Len) {
    uint16_t* flash_ptr = (uint16_t*)FlashAddress;
    uint16_t word;

    for (uint16_t i = 0; i < Len; i += 2) {
        // Ð?c t?ng t? (16-bit) t? Flash
        word = *flash_ptr++;

        // Chia t? thành hai byte
        Data[i] = (uint8_t)(word & 0xFF);       // Byte th?p
        Data[i + 1] = (uint8_t)((word >> 8) & 0xFF); // Byte cao
    }
}



void Flash_ProgramPage(char* dataIn, uint32_t addr1, uint32_t addr2)
{
    //FLASH_Unlock
    Flash_Unlock();
    //Flash_Erase Page
    Flash_Erase(addr1);
    //FLASH_Program HalfWord
    Flash_Write_Char(addr1,dataIn);
    HAL_Delay(100);
    char tempbuf[5] = {0};
    //sprintf(tempbuf,"%d",lengthPage);
    //FLASH_Unlock
    Flash_Unlock();
    //Flash_Erase Page
    Flash_Erase(addr2);
    //FLASH_Program HalfWord
    Flash_Write_Char(addr2,tempbuf);
}
