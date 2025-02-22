#include <PID.h>
#include <IRQ.h>
#include <crc8_crc16.h>
#include "stdbool.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart1;

extern PID_typedef PID;

extern volatile int32_t set_speed;
extern volatile int32_t set_Pos;
extern volatile uint8_t start_PID;
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
            pid->delta_EnC = -(int16_t)(pid->prev_En + (TIM2->ARR - pid->curr_En));
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
            pid->delta_EnC = (int16_t)(pid->curr_En + (TIM2->ARR - pid->prev_En));
        }
    }
    pid->EnC_now += ((int32_t)(pid->delta_EnC));
    pid->Speed = ((float)pid->delta_EnC)/(pid->PPR * 4 * pid->Delta_T);
    pid->prev_En = pid->curr_En;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM4)
    {
        //Update_Encoder(&PID);
         PID.enablePID = 1;
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

}
//Header(1 byte) + Data(2 byte) + crc8(1 byte)
static void revData_process()
{
    if(PID.pid_rev.rxData[0] != 0xFE)
        return;

    if(verify_CRC8_check_sum(PID.pid_rev.rxData,len(PID.pid_rev.rxData)) == false)
        return;

    PID.pid_rev.isCorrectData = ((PID.pid_rev.rxData[1] & 0x7f) == PID.ID) ? true : false;

    if(PID.pid_rev.isCorrectData == false)
        return;

    PID.Dir = (PID.pid_rev.rxData[1] >> 7);
    set_speed = (int32_t)PID.pid_rev.rxData[2];
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        revData_process();
        HAL_UART_Receive_IT(huart,PID.pid_rev.rxData,len(PID.pid_rev.rxData));
    }
}
