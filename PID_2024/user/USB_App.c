#include <USB_App.h>
#include <PID.h>
#include "usart.h"
#include <Flash_Func.h>

USBD_HandleTypeDef 				  *pdev1;
USBD_CUSTOM_HID_HandleTypeDef     *hhid1;

dataHID_typedef usbPID;

/* Function to check address to rdwr data */
uint32_t check_Address_Flash(dataHID_typedef *hid)
{
    uint8_t mode_save = *(uint8_t *)(hid->dataIn + 37);
    uint32_t address_save;
    switch(mode_save)
    {
    case None_PID:
        address_save = SAVE_PID_Speed_Addr;
        break;
    case PID_Speed:
        address_save = SAVE_PID_Speed_Addr;
        break;
    case PID_Position:
        address_save = SAVE_PID_Position_Addr;
        break;
    default:
        address_save = SAVE_PID_Speed_Addr;
        break;
    }
    return address_save;
}

/* Send data to PC */
static void HID_param_init(dataHID_typedef *hid)
{
//    static uint8_t ID[12] = {'D','F','E','E','.','P','I','D','V','1','.','1'};
    hid->isConnected = 0;
    hid->timeOut = 0;
    hid->header[0] = 'N';
    hid->header[1] = 'A';

    hid->buffer[0] = &hid->header[0];
    hid->buffer[1] = &hid->header[1];

    hid->buffer[2] = (uint8_t *)&PID.ID;

    hid->buffer[3] = (uint8_t *)&PID.mode;

    hid->buffer[4] = (uint8_t *)&PID.Dir;

    hid->buffer[5] = (uint8_t *)&PID.V_set;
    hid->buffer[6] = (uint8_t *)&PID.V_set + 1;
    hid->buffer[7] = (uint8_t *)&PID.V_set + 2;
    hid->buffer[8] = (uint8_t *)&PID.V_set + 3;

    hid->buffer[9]  = (uint8_t *)&PID.Speed;
    hid->buffer[10] = (uint8_t *)&PID.Speed + 1;
    hid->buffer[11] = (uint8_t *)&PID.Speed + 2;
    hid->buffer[12] = (uint8_t *)&PID.Speed + 3;

    hid->buffer[13] = (uint8_t *)&PID.V_now;

    hid->buffer[14] = (uint8_t *)&PID.ptlCtrl;

    hid->buffer[15] = (uint8_t *)&PID.PPR;
    hid->buffer[16] = (uint8_t *)&PID.PPR + 1;

    hid->buffer[17] = (uint8_t *)&PID.RPM;
    hid->buffer[18] = (uint8_t *)&PID.RPM + 1;

    hid->buffer[19] = (uint8_t *)&PID.Kp_def;
    hid->buffer[20] = (uint8_t *)&PID.Kp_def + 1;
    hid->buffer[21] = (uint8_t *)&PID.Kp_def + 2;
    hid->buffer[22] = (uint8_t *)&PID.Kp_def + 3;

    hid->buffer[23] = (uint8_t *)&PID.Ki_def;
    hid->buffer[24] = (uint8_t *)&PID.Ki_def + 1;
    hid->buffer[25] = (uint8_t *)&PID.Ki_def + 2;
    hid->buffer[26] = (uint8_t *)&PID.Ki_def + 3;

    hid->buffer[27] = (uint8_t *)&PID.Kd_def;
    hid->buffer[28] = (uint8_t *)&PID.Kd_def + 1;
    hid->buffer[29] = (uint8_t *)&PID.Kd_def + 2;
    hid->buffer[30] = (uint8_t *)&PID.Kd_def + 3;

    hid->buffer[31] = (uint8_t*)&PID.Motor_Duty;
    hid->buffer[32] = (uint8_t*)&PID.Motor_Duty + 1;

    hid->buffer[33] = (uint8_t*)&PID.E_Sped;
    hid->buffer[34] = (uint8_t*)&PID.E_Sped + 1;
    hid->buffer[35] = (uint8_t*)&PID.E_Sped + 2;
    hid->buffer[36] = (uint8_t*)&PID.E_Sped + 3;

    hid->buffer[37] = (uint8_t*)&PID.EnC_set;
    hid->buffer[38] = (uint8_t*)&PID.EnC_set + 1;
    hid->buffer[39] = (uint8_t*)&PID.EnC_set + 2;
    hid->buffer[40] = (uint8_t*)&PID.EnC_set + 3;

    hid->buffer[41] = (uint8_t*)&PID.EnC_now;
    hid->buffer[42] = (uint8_t*)&PID.EnC_now + 1;
    hid->buffer[43] = (uint8_t*)&PID.EnC_now + 2;
    hid->buffer[44] = (uint8_t*)&PID.EnC_now + 3;

    hid->buffer[45] = (uint8_t*)&PID.targetPos;
    hid->buffer[46] = (uint8_t*)&PID.targetPos + 1;
    hid->buffer[47] = (uint8_t*)&PID.targetPos + 2;
    hid->buffer[48] = (uint8_t*)&PID.targetPos + 3;

    hid->buffer[49] = (uint8_t*)&PID.Time_step;
    hid->buffer[50] = (uint8_t*)&PID.Time_step + 1;

    hid->buffer[51] = (uint8_t*)&PID.scalefactor;

    hid->buffer[52] = (uint8_t*)&PID.E_Pos;
    hid->buffer[53] = (uint8_t*)&PID.E_Pos + 1;
    hid->buffer[54] = (uint8_t*)&PID.E_Pos + 2;
    hid->buffer[55] = (uint8_t*)&PID.E_Pos + 3;

    hid->buffer[56] = (uint8_t *)&PID.delta_EnC;
    hid->buffer[57] = (uint8_t *)&PID.delta_EnC + 1;
    hid->buffer[58] = (uint8_t *)&PID.curr_En;
    hid->buffer[59] = (uint8_t *)&PID.curr_En + 1;
    hid->buffer[60] = (uint8_t *)&PID.rev_UART.baudSelected;
    hid->buffer[61] = 0;
    hid->buffer[62] = 0;
    hid->buffer[63] = 0;
}
static void HID_WD(dataHID_typedef *hid)
{
    // Get data from pointer array to Dataout
    for(uint8_t i = 0; i < DATA_SIZE; i++)
        hid->dataOut[i] = (hid->buffer[i] != NULL) ? *hid->buffer[i] : 0;
    USBD_CUSTOM_HID_SendReport(pdev1,hid->dataOut, DATA_SIZE);
}
/* Recieve data from PC */
static void HID_RD(dataHID_typedef *hid)
{
    // Copy data from USB buffer to DataIn
    memcpy(hid->dataIn + 1,hhid1->Report_buf,DATA_SIZE);
}
/* Process data read from PC */
static void HID_Process_Data(dataHID_typedef *hid)
{
    hid->requestCMD       = *(uint8_t *)(hid->dataIn + 4);
    hid->isUSBControl     = *(uint8_t *)(hid->dataIn + 40);
    // if User control by USB from PC
    if(hid->isUSBControl == USB_C)
    {
        //Disable UART
        if ((USART1->CR1 & USART_CR1_UE) != 0)
            __HAL_UART_DISABLE(&huart1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4,1);

        if(PID.mode == PID_Speed || PID.mode == None_PID)
        {
            PID.Dir         = *(uint8_t *)(hid->dataIn + 9);
            PID.Value_Set   =  (PID.Dir == 0) ?
                               *(int32_t *)(hid->dataIn + 5) : -(*(int32_t *)(hid->dataIn + 5));
        }
        else if(PID.mode == PID_Position)
        {
            PID.Time_step   = *(uint16_t *)(hid->dataIn + 49);
            PID.scalefactor = *(uint8_t *)(hid->dataIn + 51);
            PID.targetPos   = *(int32_t *)(hid->dataIn + 45);
        }
    }
    else
    {
        //Enable UART
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4,0);
        if ((USART1->CR1 & USART_CR1_UE) == 0)
            __HAL_UART_ENABLE(&huart1);
    }
}

static void HID_Request_Function(dataHID_typedef *hid)
{
    // Request save new config from PC
    if(hid->requestCMD == Save_config)
    {
        Flash_data.address_flash = check_Address_Flash(hid);
        Flash_data.f_write_param(&Flash_data);
        Flash_data.f_read_param(&Flash_data);
        PID.f_update_config(&PID);
    }
    // Request get config from Flash
    else if(hid->requestCMD == Get_infor)
    {
        Flash_data.address_flash = check_Address_Flash(hid);
        Flash_data.f_read_param(&Flash_data);

        uint8_t mode_save = *(uint8_t *)(hid->dataIn + 37);
        if(mode_save == PID.mode)
        {
            hid->f_data_write(hid);
        }
        else
        {
            // Get data from pointer array to Dataout
            for(uint8_t i = 0; i < DATA_SIZE; i++)
                hid->dataOut[i] = (hid->buffer[i] != NULL) ? *hid->buffer[i] : 0;

            hid->dataOut[3] = *(uint8_t *)Flash_data.data_rd;
            hid->dataOut[2] = *(uint8_t *)Flash_data.data_rd + 1;
            hid->dataOut[15] = *(uint8_t *)Flash_data.data_rd + 2;
            hid->dataOut[16] = *(uint8_t *)Flash_data.data_rd + 3;
            hid->dataOut[17] = *(uint8_t *)Flash_data.data_rd + 4;
            hid->dataOut[18] = *(uint8_t *)Flash_data.data_rd + 5;
            hid->dataOut[19] = *(uint8_t *)Flash_data.data_rd + 10;
            hid->dataOut[20] = *(uint8_t *)Flash_data.data_rd + 11;
            hid->dataOut[21] = *(uint8_t *)Flash_data.data_rd + 12;
            hid->dataOut[22] = *(uint8_t *)Flash_data.data_rd + 13;
            hid->dataOut[23] = *(uint8_t *)Flash_data.data_rd + 14;
            hid->dataOut[24] = *(uint8_t *)Flash_data.data_rd + 15;
            hid->dataOut[25] = *(uint8_t *)Flash_data.data_rd + 16;
            hid->dataOut[26] = *(uint8_t *)Flash_data.data_rd + 17;
            hid->dataOut[27] = *(uint8_t *)Flash_data.data_rd + 18;
            hid->dataOut[28] = *(uint8_t *)Flash_data.data_rd + 19;
            hid->dataOut[29] = *(uint8_t *)Flash_data.data_rd + 20;
            hid->dataOut[30] = *(uint8_t *)Flash_data.data_rd + 21;
            hid->dataOut[14] = *(uint8_t *)Flash_data.data_rd + 22;
            hid->dataOut[51] = *(uint8_t *)Flash_data.data_rd + 23;
            hid->dataOut[60] = *(uint8_t *)Flash_data.data_rd + 24;
            USBD_CUSTOM_HID_SendReport(pdev1,hid->dataOut, DATA_SIZE);
        }
    }
    hid->requestCMD = 0;
}

void HID_Data_Init(dataHID_typedef *hid)
{
    hid->f_param_init = HID_param_init;
    hid->f_data_write = HID_WD;
    hid->f_data_read  = HID_RD;
    hid->f_process_data = HID_Process_Data;
    hid->f_request_func = HID_Request_Function;
}