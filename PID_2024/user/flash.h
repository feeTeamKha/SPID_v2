#include "stm32f1xx_hal.h"
#include "stdint.h"
#include "string.h"

void deleteBuffer(char* data);
void 	Flash_Lock(void);
void 	Flash_Unlock(void);
void 	Flash_Erase(uint32_t addr);
void 	Flash_Write_Int(uint32_t addr, int data);
uint16_t Flash_Read_Int(uint32_t addr);
void 	Flash_Write_Char(uint32_t addr, char* data);
void 	Flash_ReadChar(char* dataOut, uint32_t addr1, uint32_t addr2);
void 	Flash_ProgramPage(char* dataIn, uint32_t addr1, uint32_t addr2);
void    Flash_Write_Float(uint32_t address, float f);
float   Flash_Read_Float(uint32_t address);
void    Flash_Write_Array(uint32_t address, uint16_t *arr, uint16_t len);
void    Flash_Read_Array(uint32_t address, uint16_t *arr, uint16_t len);
void    Flash_Write_Uint8_t(uint32_t FlashAddress, uint8_t* Data, uint16_t Len);
void    Flash_Read_Uint8_t(uint32_t FlashAddress, uint8_t* Data, uint16_t Len);
