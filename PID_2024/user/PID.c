#include <PID.h>
#include <math.h>
#include "tim.h"
#include <CRC8_CRC16.h>
#include "stdbool.h"
#include <Flash_Func.h>

#define myABS(x) ((x) < 0 ? -(x) : (x))
#define maxOuput    970
//#define Speed_scale 15000 // 15000 = 60(s) * 250
//#define Duty_scale  3.98f // 995:250
#define Speed_scale 60000 // 60000 = 60(s) * 1000

#define LimitMax(input, max)           \
        {                              \
            if(input > max)            \
                input = max;           \
            else if (input < -max)     \
                input = -max;          \
        }                              \

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart1;
PID_typedef PID;

static void PID_param_init(PID_typedef *pid)
{
//    pid->ptlCtrl = UARTCOM;

    pid->enablePID = 0;

    pid->modePos = Normal;

    pid->Motor_Duty = 0;

    pid->scalefactor = (pid->scalefactor == 0) ? 1 : pid->scalefactor;

    /* POSITION : SPEED */
    pid->Delta_T = (pid->mode == PID_Position) ? 0.001f : (pid->PPR >= 100) ? 0.005f : 0.01f;
    pid->pulsetoRound = (float)(pid->PPR * 4 * pid->Delta_T);
    pid->Dir 	= 0;
    pid->V_set 	= 0;
    pid->V_now  = 0;
    pid->Speed = 0.0f;
    pid->delta_EnC = 0;

    pid->Time_step = 0;
    pid->targetPos = 0;

    pid->P_Pos = pid->P_Sped = 0.0f;
    pid->I_Pos = pid->I_Sped = 0.0f;
    pid->D_Pos = pid->D_Sped = 0.0f;

    pid->E_Pos  = pid->E_Sped  = 0.0f;
    pid->E1_Pos = pid->E1_Sped = 0.0f;
    pid->E2_Pos = pid->E2_Sped = 0.0f;

    pid->Output_Pos     = pid->Output_Sped = 0.0f;
    pid->sumOutput_Pos = pid->lastOutput_Sped = 0.0f;

    pid->curr_En = 0;
    pid->prev_En = 0;

    pid->EnC_now = 0;
    pid->EnC_set = 0;

    if(pid->mode == PID_Position)
    {
        pid->RPM = 0;
        pid->PPR = 1;
        pid->Kp_Pos = pid->Kp_def;
        pid->Ki_Pos = pid->Ki_def;// * pid->Delta_T;
        pid->Kd_Pos = pid->Kd_def;// / pid->Delta_T;

        pid->Kp_Sped = 0;
        pid->Ki_Sped = 0;
        pid->Kd_Sped = 0;
    }
    else if(pid->mode == PID_Speed)
    {
        pid->Kp_Pos = (pid->PPR >= 100) ? 1.5f:15.0f;
//        pid->Ki_Pos = 0.00001f;
//        pid->Kd_Pos = 0.00003f;
        pid->Ki_Pos = (pid->PPR >= 100) ? 0.0001f:0.0001f;
        pid->Kd_Pos = (pid->PPR >= 100) ? 0.0005f:0.005f;

//        pid->Kp_def = (pid->PPR >= 100) ? 50.0f:10.0f;
//        pid->Ki_def = (pid->PPR >= 100) ? 2000.0f:400.0f;
//        pid->Kd_def = (pid->PPR >= 100) ? 0.01f:0.001f;
        pid->Kp_def = (pid->PPR >= 100) ? 5.0f:((pid->PPR <= 13) ? 5.0f : 3.0f);
        pid->Ki_def = (pid->PPR >= 100) ? 0.005f:0.05f;
        pid->Kd_def = (pid->PPR >= 100) ? 0.08f:((pid->PPR <= 13) ? 0.08f : 0.03f);
        pid->Kp_Sped = pid->Kp_def;
        pid->Ki_Sped = pid->Ki_def * pid->Delta_T;
        pid->Kd_Sped = pid->Kd_def / pid->Delta_T;
    }

    pid->rev_UART.isCorrectData = 0;
//    pid->rev_UART.rxData[0] = 0;
//    pid->rev_UART.rxData[1] = 0;
//    pid->rev_UART.rxData[2] = 0;
//    pid->rev_UART.rxData[3] = 0;

    (pid->ptlCtrl == PWM_mode) ?
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1):
    (TIM3->ARR = 100-1,HAL_TIM_Base_Start_IT(&htim3),HAL_UART_Receive_IT(&huart1,PID.rev_UART.rxData,PID.mode));

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start(&htim2,TIM_CHANNEL_ALL);
    PID_Motor_Ctrl(PID.Motor_Duty);
    MX_TIM4_Init();
    HAL_TIM_Base_Start_IT(&htim4);
}

//static void LocK_process(PID_typedef *pid, uint8_t lock_lv)
//{
//    if(abs(pid->V_now) >= 200)
//        pid->Kp_Sped = pid->Kp_def + 1.0f, pid->Ki_Sped = pid->Ki_def + 3.0f;
//    else if(abs(pid->V_now) < 200 && abs(pid->V_now) >= 150)
//        pid->Kp_Sped = pid->Kp_def + 0.5f, pid->Ki_Sped = pid->Ki_def + 2.0f;
//    else if(abs(pid->V_now) < 150 && abs(pid->V_now) >= 100)
//        pid->Kp_Sped = pid->Kp_def + 0.5f, pid->Ki_Sped = pid->Ki_def + 1.5f;
//    else if(abs(pid->V_now) < 100 && abs(pid->V_now) >= 30)
//        pid->Kp_Sped = pid->Kp_def - 0.5f, pid->Ki_Sped = (pid->PPR >= 100) ? pid->Ki_def - 2.0f : pid->Ki_def - 3.0f;
//    else if(abs(pid->V_now) < 30 && abs(pid->V_now) >= 15)
//        pid->Kp_Sped = pid->Kp_def - 1.0f, pid->Ki_Sped = (pid->PPR >= 100) ? pid->Ki_def - 3.0f : pid->Ki_def - 4.0f;
//    else
//    {
//        pid->Ki_Sped = pid->Ki_def - (double)lock_lv;
//        pid->Kp_Sped = pid->Kp_def - (double)lock_lv;
//        if(pid->PPR >= 100)
//        {
//            pid->Kp_Pos = (1.6f - (double)lock_lv/10);
//        }
//        else
//            pid->Kp_Pos = (16.0f - (double)lock_lv/10);
//    }
//}


static void PID_set(PID_typedef *pid, float Kp, float Ki, float Kd)
{
    if(pid->mode == PID_Position)
    {
        pid->Kp_Pos = Kp;
        pid->Ki_Pos = Ki;//*pid->Delta_T;
        pid->Kd_Pos = Kd;///pid->Delta_T;
    }
    else
    {
        pid->Kp_Sped = Kp;
        pid->Ki_Sped = Ki*pid->Delta_T;
        pid->Kd_Sped = Kd/pid->Delta_T;
    }
}

static void PID_Calulate(PID_typedef *pid)
{

    if(pid->mode == PID_Speed)
    {
        static uint8_t first_Lock = 0;
        if(pid->Value_Set != 0U)
        {
            // RPS to 0-1000
            pid->V_now = fabs((pid->Speed * Speed_scale) / pid->RPM);// 0 - 1000

            if(pid->Value_Set > 1000) pid->Value_Set = 1000;
            else if(pid->Value_Set < -1000) pid->Value_Set = -1000;
            pid->V_set = (float)(pid->Value_Set * pid->RPM)/Speed_scale;

            // if speed is less 3 round/second
            if(myABS(pid->V_set) < 3U)
            {
                pid->V_set = 0U;
//                LocK_process(pid,*Value_Set);
                if(first_Lock == 0 && fabs(pid->Speed) == 0.0f)
                {
                    pid->Delta_T = 0.001f;
                    TIM4->ARR = 1000-1;
                    //Reset Data
                    pid->P_Sped = pid->I_Sped = pid->D_Sped = 0.0f;
                    pid->Output_Sped = 0.0f;
                    pid->lastOutput_Sped = pid->Output_Sped;
                    // None Lock
                    pid->Motor_Duty = (int16_t)pid->Output_Pos;
                    PID_Motor_Ctrl(pid->Motor_Duty);
                    first_Lock = 1;

                    TIM2->CNT = 0;
                    pid->delta_EnC = 0;
                    pid->curr_En = 0;
                    pid->prev_En = 0;
                    pid->EnC_now = 0;
                    pid->EnC_set = 0;
                }
            }
            else
            {
                if(first_Lock == 1)
                {
                    if(pid->PPR >= 100)
                    {
                        pid->Delta_T = 0.005f;
                        TIM4->ARR = ARR_SET-1;
                    }
                    else
                    {
                        pid->Delta_T = 0.01f;
                        TIM4->ARR = 10000-1;
                    }
                    pid->P_Pos = pid->I_Pos = pid->D_Pos = 0.0f;
                    pid->Output_Pos = 0.0f;
                    // None Lock
                    pid->Motor_Duty = (int16_t)pid->Output_Sped;
                    PID_Motor_Ctrl(pid->Motor_Duty);
                    first_Lock = 0;
                }
            }

            if(first_Lock == 0)
            {
                // Get Erorr
                pid->E_Sped = (pid->V_set - pid->Speed);
                
                if(pid->E_Sped == pid->E1_Sped)
                {
                    pid->E_Sped = 0;
                }
                
                if(myABS(pid->E_Sped) < 3)
                {
                    if(pid->PPR == 17 || pid->PPR == 11)
                        PID_set(pid,pid->Kp_def - 1.5,0,0.001f);
                    else
                        PID_set(pid,pid->Kp_def - 1.5,0,pid->Kd_def);
                }
                else
                {
                    PID_set(pid,pid->Kp_def,pid->Ki_def,pid->Kd_def);
                }
                
                pid->P_Sped = pid->Kp_Sped * pid->E_Sped;
                pid->I_Sped += pid->Ki_Sped * pid->E_Sped * 0.5f;
                pid->D_Sped = pid->Kd_Sped * (pid->E_Sped - pid->E1_Sped);

                LimitMax(pid->I_Sped,maxOuput);

                // Calculate Ouput
                pid->Output_Sped = pid->lastOutput_Sped
                                   + pid->P_Sped + pid->I_Sped + pid->D_Sped;

                //Litmit Ouput
                LimitMax(pid->Output_Sped,maxOuput);

                // Update feedback
                pid->lastOutput_Sped = pid->Output_Sped;

                if(pid->E1_Sped != pid->E_Sped && pid->E_Sped != 0)
                {
                   pid->E1_Sped = pid->E_Sped;
                }
                
                // Output
                pid->Motor_Duty = (int16_t)pid->Output_Sped;

                PID_Motor_Ctrl(pid->Motor_Duty);
            }
            else if(first_Lock == 1)
            {
                pid->E_Sped = pid->V_set - pid->Speed;
                pid->E_Pos = (pid->EnC_set - pid->EnC_now);

                /* Apdaptive Config */
                if(myABS(pid->E_Pos) < 20)
                {
                    if(pid->PPR >= 100)
                        pid->Kp_Pos = (float)(0.6f - (float)myABS(pid->Value_Set/10));
                    else
                        pid->Kp_Pos = (float)(6.0f - myABS(pid->Value_Set));
                    pid->Ki_Pos = 0;//(pid->PPR >= 100) ? 0.0001f:0.0001f;
                    pid->Kd_Pos = 0;//(pid->PPR >= 100) ? 0.0005f:0.005f;
                }
                else
                {
                    if(pid->PPR >= 100)
                        pid->Kp_Pos = (float)(1.6f - (float)myABS(pid->Value_Set/10));
                    else
                        pid->Kp_Pos = (float)(16.0f - myABS(pid->Value_Set));
                    pid->Ki_Pos = (pid->PPR >= 100) ? 0.0001f:0.0001f;
                    pid->Kd_Pos = (pid->PPR >= 100) ? 0.0005f:0.005f;
                }
                                
                pid->P_Pos = pid->Kp_Pos * pid->E_Pos;

                pid->I_Pos += pid->Ki_Pos * pid->E_Pos * pid->Delta_T * 0.5f;
                LimitMax(pid->I_Pos,maxOuput);

                pid->D_Pos = -(pid->Kd_Pos * (pid->EnC_now - pid->lastEnC) / pid->Delta_T);

                pid->Output_Pos = pid->P_Pos + pid->I_Pos + pid->D_Pos;

                LimitMax(pid->Output_Pos,(maxOuput+5)/1.2);

                pid->lastEnC = pid->EnC_now;
                pid->E1_Pos = pid->E_Pos;
                pid->Motor_Duty = (int16_t)round(pid->Output_Pos);
                PID_Motor_Ctrl(pid->Motor_Duty);
            }
        }
        else
        {
            //Reset Data
            pid->V_set = 0U;
            pid->EnC_now = pid->EnC_set;
            pid->E_Pos = 0;
            pid->E1_Pos = pid->E2_Pos = pid->E_Pos;
            pid->E_Sped = 0;
            pid->E2_Sped = pid->E1_Sped = pid->E_Sped;
            pid->P_Sped = pid->I_Sped = pid->D_Sped = 0.0f;
            pid->Output_Sped = 0.0f;
            pid->lastOutput_Sped = pid->Output_Sped;
            // None Lock
            pid->Motor_Duty = (int16_t)pid->Output_Sped;
            PID_Motor_Ctrl(pid->Motor_Duty);
        }
    }
    else if(pid->mode == PID_Position)
    {
//        if(pid->lastTimeStep != pid->Time_step)
//        {
//            if(pid->Time_step < 15)
//            {
//                pid->Kp_Pos = pid->Kp_def;
//                pid->Kd_Pos = pid->Kd_def/pid->Delta_T;
//            }
//            else if(pid->Time_step >= 15)
//            {
//                pid->Kp_Pos = pid->Kp_def/(pid->Time_step*0.1f);
//                pid->Kp_Pos = (pid->Kp_Pos <= pid->Kp_def/4) ? (pid->Kp_def/4): pid->Kp_Pos;
//                pid->Kd_Pos = pid->Kd_def/((pid->Time_step*0.1)*pid->Delta_T);
//            }
//            pid->lastTimeStep = pid->Time_step;
//        }
        
//        if(pid->targetPos == pid->Value_Set)
//             pid->Kp_Pos = pid->Kp_def;
        
        pid->EnC_set = pid->Value_Set * 4;

        pid->E_Pos = (pid->EnC_set - pid->EnC_now);      
        
        pid->P_Pos = pid->Kp_Pos * pid->E_Pos;

        pid->I_Pos += pid->Ki_Pos * pid->E_Pos * 0.5;
        LimitMax(pid->I_Pos,maxOuput);

        pid->D_Pos = -(pid->Kd_Pos * (pid->EnC_now - pid->lastEnC));

        pid->Output_Pos = pid->P_Pos + pid->I_Pos + pid->D_Pos;

        LimitMax(pid->Output_Pos,maxOuput);

        pid->lastEnC = pid->EnC_now;
        pid->E1_Pos = pid->E_Pos;
        pid->Motor_Duty = (int16_t)pid->Output_Pos;
        PID_Motor_Ctrl(pid->Motor_Duty);
    }
    else if(pid->mode == None_PID)
    {
        //Reset Data
        pid->EnC_now = pid->EnC_set;
        pid->E_Pos = 0;
        pid->E1_Pos = pid->E2_Pos = pid->E_Pos;
        pid->P_Sped = pid->I_Sped = pid->D_Sped = 0.0f;
        pid->Output_Sped = 0.0f;
        pid->lastOutput_Sped = pid->Output_Sped;

        pid->Motor_Duty = (int16_t)pid->Value_Set;
        LimitMax(pid->Motor_Duty, maxOuput);
        PID_Motor_Ctrl(pid->Motor_Duty);
    }
}
//ByteL -> ByteH
static void PID_UpdateSetpoint(PID_typedef *pid)
{
    if((bool)verify_CRC16_check_sum(pid->rev_UART.rxData,frameSize) == false)
        return;
    if(pid->mode != PID_Position)
    {
        pid->Value_Set = *(int16_t *)(pid->rev_UART.rxData + 3);
    }
    else
    {
        pid->Time_step  = *(uint16_t *)(pid->rev_UART.rxData + 3);
        pid->targetPos  = *(int16_t *)(pid->rev_UART.rxData + 5);
        pid->modePos    = *(uint8_t *)(pid->rev_UART.rxData + 7);
    }
}

static void PID_Update_Config(PID_typedef *pid)
{
    pid->mode    = *(uint8_t*)Flash_data.data_rd;
    pid->ID      = *(uint8_t*)(Flash_data.data_rd + 1);

    if(pid->mode == 0xFF || pid->ID == 0xFF)
    {
        pid->mode = None_PID;
        pid->ID   = 0;
        return;
    }

//    pid->Delta_T = (pid->mode == PID_Position) ? 0.001f : (pid->PPR >= 100) ? 0.005f : 0.01f;
    if(pid->mode == PID_Position)
    {
        pid->Delta_T = 0.001f;
        TIM4->ARR = 1000 - 1;
        TIM3->ARR = 100-1;
    }
    else
    {
        if(pid->PPR >= 100)
        {
            pid->Delta_T = 0.005f;
            TIM4->ARR = ARR_SET - 1;
        }
        else
        {
            pid->Delta_T = 0.01f;
            TIM4->ARR = ARR_SET*2 - 1;
        }

    }
    pid->PPR     = *(uint16_t *)(Flash_data.data_rd + 2);
    pid->RPM     = *(uint16_t *)(Flash_data.data_rd + 4);
    pid->ptlCtrl = *(uint8_t *)(Flash_data.data_rd + 22);
    pid->rev_UART.baudSelected = (*(uint8_t*)(Flash_data.data_rd + 24) > 6) ? 6 : *(uint8_t*)(Flash_data.data_rd + 24);
    if(pid->mode == PID_Position)
    {
        pid->Kp_def  = *(float *)(Flash_data.data_rd + 10);
        pid->Ki_def  = *(float *)(Flash_data.data_rd + 14);
        pid->Kd_def  = *(float *)(Flash_data.data_rd + 18);
        pid->scalefactor = *(uint8_t*)(Flash_data.data_rd + 23);

        pid->RPM = 0;
        pid->PPR = 1;
        PID_set(pid,pid->Kp_def,pid->Ki_def,pid->Kd_def);

        pid->Kp_Sped = 0;
        pid->Ki_Sped = 0;
        pid->Kd_Sped = 0;
    }
    else if(pid->mode == PID_Speed)
    {
        pid->Kp_Pos = (pid->PPR >= 100) ? 1.5f:15.0f;
        pid->Ki_Pos = (pid->PPR >= 100) ? 0.0001f:0.0001f;
        pid->Kd_Pos = (pid->PPR >= 100) ? 0.0005f:0.005f;

        pid->Kp_def = (pid->PPR >= 100) ? 5.0f:((pid->PPR <= 13) ? 5.0f : 3.0f);
        pid->Ki_def = (pid->PPR >= 100) ? 0.005f:0.05f;
        pid->Kd_def = (pid->PPR >= 100) ? 0.08f:((pid->PPR <= 13) ? 0.08f : 0.03f);

        PID_set(pid,pid->Kp_def,pid->Ki_def,pid->Kd_def);
    }
    pid->pulsetoRound = (float)(pid->PPR * 4 * pid->Delta_T);
}

void PID_Init(PID_typedef *pid)
{
    pid->f_param_init = PID_param_init;
    pid->f_pid_set    = PID_set;
    pid->f_cal_pid	  = PID_Calulate;
    pid->f_update_setpoint = PID_UpdateSetpoint;
    pid->f_update_config = PID_Update_Config;
}
