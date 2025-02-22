#ifndef _FLASH_FUNC_H_
#define _FLASH_FUNC_H_

#include "stm32f1xx.h"
#include <PID.h>

#define SAVE_PID_Init               ((uint32_t)0x0801F800) //Page126
#define SAVE_PID_Speed_Addr         ((uint32_t)0x0801FC00) //Page127
#define SAVE_PID_Position_Addr      ((uint32_t)0x0801F400) //Page125

#define DATA_SIZE_FL 32
typedef struct _flashDef
{
    uint8_t* data_buffer[DATA_SIZE_FL];
    uint8_t data_wr[DATA_SIZE_FL];
    uint8_t data_rd[DATA_SIZE_FL];
    uint32_t address_flash;
    void(*f_init_param)(struct _flashDef *fl);
    void(*f_write_param)(struct _flashDef *fl);
    void(*f_read_param)(struct _flashDef *fl);
    void(*f_change_mode)(struct _flashDef *fl, PID_typedef *pid);
}Flashdef;
extern Flashdef Flash_data;

void Flash_data_init(Flashdef *fl);
#endif /*_FLASH_FUNC_H_*/