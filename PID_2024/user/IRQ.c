#include <PID.h>
#include <IRQ.h>
#include <crc8_crc16.h>
#include "stdbool.h"
#include <string.h>
#include <USB_App.h>
#include <Flash_Func.h>

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart1;

extern PID_typedef PID;
extern dataHID_typedef usbPID;
extern USBD_HandleTypeDef 				  *pdev1;
extern USBD_CUSTOM_HID_HandleTypeDef      *hhid1;

volatile uint16_t cntPID = 0;

void Update_Encoder(PID_typedef *pid)
{
    pid->curr_En = TIM2->CNT;

    if(pid->curr_En == pid->prev_En)
    {
        pid->delta_EnC = 0;
    }
    else if(pid->curr_En > pid->prev_En)
    {
        if(TIM2->CR1 & TIM_CR1_DIR)/*__HAL_TIM_IS_TIM_COUNTING_DOWN(htim)*/
        {
            pid->delta_EnC = -(int16_t)(pid->prev_En + (TIM2->ARR + 1 - pid->curr_En));
        }
        else
        {
            pid->delta_EnC = (int16_t)(pid->curr_En - pid->prev_En);
        }
    }
    else
    {
        if(TIM2->CR1 & TIM_CR1_DIR)
        {
            pid->delta_EnC = (int16_t)(pid->curr_En - pid->prev_En);
        }
        else
        {
            pid->delta_EnC = (int16_t)(pid->curr_En + (TIM2->ARR + 1 - pid->prev_En));
        }
    }
    pid->EnC_now += ((int32_t)(pid->delta_EnC));
    pid->Speed = (float)(((float)pid->delta_EnC)/pid->pulsetoRound);
    pid->prev_En = pid->curr_En;
}
void SysTick_Handler(void)
{
    /* USER CODE BEGIN SysTick_IRQn 0 */
    /* USER CODE END SysTick_IRQn 0 */
    HAL_IncTick();
    /* USER CODE BEGIN SysTick_IRQn 1 */

    /* USER CODE END SysTick_IRQn 1 */
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM4)
    {
        Update_Encoder(&PID);
        PID.f_cal_pid(&PID);
        if(pdev1->dev_state == 0x03) //Connected USB
        {
            if(usbPID.isConnected == 1)
            {        
                usbPID.f_data_read(&usbPID);//Recive data from PC
                usbPID.f_process_data(&usbPID);//Process data from PC
                usbPID.f_request_func(&usbPID);//Check CMD request
                usbPID.f_data_write(&usbPID);//Send data to PC
            }
        }
        else
        {
            usbPID.isConnected = 0;
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4,0);
        }
        if(PID.rev_UART.rxDone == 1)
        {
            PID.f_update_setpoint(&PID);
            PID.rev_UART.rxDone = 0;
        }
    }
    if(htim->Instance == TIM3)
    {
        if(PID.ptlCtrl != PWM_mode && PID.mode == PID_Position)
        {
            if(PID.Time_step == 0)
            {
                PID.Value_Set = (int32_t)(PID.targetPos * PID.scalefactor);
            }

            if(PID.modePos == Normal)
            {
                cntPID++;
                if(cntPID >= PID.Time_step)
                {
                    if( PID.Value_Set > (int32_t)(PID.targetPos * PID.scalefactor))
                        PID.Value_Set--;
                    else if(PID.Value_Set < (int32_t)(PID.targetPos * PID.scalefactor))
                        PID.Value_Set++;
                    else
                        PID.Value_Set = (int32_t)(PID.targetPos * PID.scalefactor);

                    cntPID = 0;
                }
            }
            else if(PID.modePos == ReStart)
            {
                PID.EnC_now = 0;
                PID.Value_Set = PID.targetPos = 0;
                TIM2->CNT = 0;
                PID.prev_En = PID.curr_En = 0;
                PID.modePos = Normal;
            }
            else
                cntPID = 0;
        }
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM3)
    {
        PID.Dir = GPIOA->IDR & GPIO_PIN_7;
        (PID.Dir == 0) ? PID.Value_Set++ : PID.Value_Set--;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

}
//static void reverse(uint8_t* start, uint8_t* end) {
//    while (start < end - 1) {
//        uint8_t temp = *start;
//        *start = *(end - 1);
//        *(end - 1) = temp;
//        start++;
//        end--;
//    }
//}
//static void sort_data(uint8_t* data, int length,uint8_t isHeader) {
//    uint8_t* header = data;
//    uint8_t* end = data + length;

//    while (header < end && *header != isHeader) {
//        header++;
//    }

//    if (header != data && header != end) {
//        reverse(data, header);
//        reverse(header, end);
//        reverse(data, end);
//    }
//}

//Header(1 byte) + Data(2 byte) + crc8
//Speed: 0xFEE2 0XFF 0xFFFF 0x0000 0x00 CRC16
//Header(2 byte) + Data(1 byte(ID) + 2 byte(Speed) + 2 byte (Pulse)) + 1 byte Mode + crc16
//static void revData_process(PID_typedef *pid)
//{
//    if(pid->mode == PID_Position)
//    {
//        sort_data(PID.rev_UART.rxData,PID.mode,0xEE);
//        bool data_verify = verify_CRC16_check_sum(pid->rev_UART.rxData,pid->mode);
//        if(data_verify == false)
//            return;
//        if(pid->rev_UART.rxData[0] == 0xEE)
//        {
//            pid->rev_UART.isCorrectData = ((pid->rev_UART.rxData[1] & 0x7f) == pid->ID) ? true : false;
//            if(pid->rev_UART.isCorrectData == false)
//                return;
//            //Normal Control
//            pid->Dir = (pid->rev_UART.rxData[1] >> 7);
//            pid->Time_step = (pid->rev_UART.rxData[2] << 8) | pid->rev_UART.rxData[3];
//            pid->targetPos = (int32_t)(pid->rev_UART.rxData[4] << 8) | pid->rev_UART.rxData[5];
//            pid->targetPos = (pid->Dir == 0) ? pid->targetPos : -pid->targetPos;
//            pid->modePos = pid->rev_UART.rxData[6];
//        }
//        return;
//    }
//    else
//    {
//        sort_data(PID.rev_UART.rxData,PID.mode,0xFE);
//        if(pid->rev_UART.rxData[0] != 0xFE)
//            return;

//        if(verify_CRC8_check_sum(pid->rev_UART.rxData,pid->mode) == false)
//            return;

//        pid->rev_UART.isCorrectData = ((pid->rev_UART.rxData[1] & 0x7f) == pid->ID) ? true : false;

//        if(pid->rev_UART.isCorrectData == false)
//            return;

//        pid->Dir = (pid->rev_UART.rxData[1] >> 7);
//        pid->Value_Set = (int32_t)pid->rev_UART.rxData[2];
//        return;
//    }
//}

static void revData_process(PID_typedef *pid)
{
    pid->rev_UART.isheader[2] =  pid->rev_UART.isheader[1];
    pid->rev_UART.isheader[1] =  pid->rev_UART.isheader[0];
    pid->rev_UART.isheader[0] = pid->rev_UART.Rxbuffer[0];

    if(pid->rev_UART.isheader[2] == 0xFE && pid->rev_UART.isheader[1] == 0xE2
            && pid->rev_UART.isheader[0] == pid->ID)// ID check
    {
        pid->rev_UART.rxData[0] = 0xFE;
        pid->rev_UART.rxData[1] = 0xE2;
        pid->rev_UART.rxData[2] = pid->ID;
        pid->rev_UART.isCorrectData = 1;
        pid->rev_UART.rxCnt = 2;
        pid->rev_UART.rxDone = 0;
    }
    else if(pid->rev_UART.isCorrectData == 1)
    {
        pid->rev_UART.rxData[++pid->rev_UART.rxCnt] = pid->rev_UART.Rxbuffer[0];
        if(pid->rev_UART.rxCnt >= frameSize)
        {
            pid->rev_UART.rxCnt = 0;
            pid->rev_UART.isCorrectData = 0;
            pid->rev_UART.isheader[2] = 0xFF;
            pid->rev_UART.isheader[1] = 0xFF;
            pid->rev_UART.isheader[0] = 0xFF;
            pid->rev_UART.rxDone = 1;
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        revData_process(&PID);
        HAL_UART_Receive_IT(&huart1,PID.rev_UART.Rxbuffer,1);
    }
}
