#include <PID.h>
#include <math.h>

#define maxOuput    995
#define maxI_pos    1000
#define Speed_scale 15000 // 15000 = 60(s) * 250
#define Duty_scale  3.98f // 995:250

#define LimitMax(input, max)           \
        {                              \
            if(input > max)            \
                input = max;           \
            else if (input < -max)     \
                input = -max;          \
        }                              \

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim4;
extern volatile int32_t set_speed;


static void PID_param_init(PID_typedef *pid, double Kp,double Ki, double Kd)
{
    pid->ID = 1;

    pid->enablePID = 0;
    
    pid->scalefactor = 1;
    /* POSITION : SPEED */
    pid->Delta_T = (pid->mode == PID_Position) ? 0.005f : 0.005f;
//    pid->Delta_T = 1;
    pid->Dir 	= 0;
    pid->V_set 	= 0;
    pid->V_now  = 0;
    pid->Speed = 0.0f;
    pid->delta_EnC = 0;

    pid->P_Pos = pid->P_Sped = 0.0f;
    pid->I_Pos = pid->I_Sped = 0.0f;
    pid->D_Pos = pid->D_Sped = 0.0f;
    pid->prev_I_Pos = pid->prev_I_Sped = 0.0f;

    pid->E_Pos  = pid->E_Sped  = 0.0f;
    pid->E1_Pos = pid->E1_Sped = 0.0f;
    pid->E2_Pos = pid->E2_Sped = 0.0f;

    pid->Output_Pos     = pid->Output_Sped = 0.0f;
    pid->sumOutput_Pos = pid->lastOutput_Sped = 0.0f;

    pid->curr_En = 0;
    pid->prev_En = 0;

    pid->EnC_set = 0;
    pid->EnC_now = 0;
    pid->lastEnC = 0;

    pid->Kp_def = Kp;
    pid->Ki_def = Ki;
    pid->Kd_def = Kd;

    pid->Kp_Pos = pid->Kp_Sped = pid->Kp_def;
    pid->Ki_Pos = pid->Ki_Sped = pid->Ki_def;
    pid->Kd_Pos = pid->Kd_Sped = pid->Kd_def;

    if(pid->mode == PID_Position)
    {
        pid->Kp_Pos = pid->Kp_def;
        pid->Ki_Pos = pid->Ki_def * pid->Delta_T;
        pid->Kd_Pos = pid->Kd_def / pid->Delta_T;

//        set_speed = 10;
        pid->Kp_Sped = 0.01f;
        pid->Ki_Sped = 0.005f;
        pid->Kd_Sped = 0.0f;//0.0001f;
    }
    else if(pid->mode == PID_Speed)
    {
//        pid->Kp_Pos = 0.5f;
//        pid->Kp_Pos = 5.0f;
        pid->Ki_Pos = 0.00001f ;
//        pid->Ki_Pos = 0.01f;
        pid->Kd_Pos = 0.00003f;
//        pid->Kd_Pos = 0.03f;

        pid->Kp_Sped = pid->Kp_def;
        pid->Ki_Sped = pid->Ki_def;
        pid->Kd_Sped = pid->Kd_def;
    }

    pid->RPM = 17500;
    pid->PPR 	 = (pid->mode == PID_Position) ? 1 : 500;

    pid->Motor_Duty = 0;

    pid->Sped_Pos = 999;

    pid->pid_rev.isCorrectData = 0;
    pid->pid_rev.rxData[0] = 0;
    pid->pid_rev.rxData[1] = 0;
    pid->pid_rev.rxData[2] = 0;
    pid->pid_rev.rxData[3] = 0;
}

static void LocK_process(PID_typedef *pid, uint8_t lock_lv)
{
    if(fabs(pid->V_now) >= 200)
        pid->Kp_Sped = pid->Kp_def + 7.0f, pid->Ki_Sped = pid->Ki_def + 3.0f;
    else if(fabs(pid->V_now) < 200 && fabs(pid->V_now) >= 150)
        pid->Kp_Sped = pid->Kp_def + 6.0f, pid->Ki_Sped = pid->Ki_def + 2.0f;
    else if(fabs(pid->V_now) < 150 && fabs(pid->V_now) >= 100)
        pid->Kp_Sped = pid->Kp_def + 5.0f, pid->Ki_Sped = pid->Ki_def + 1.5f;
    else if(fabs(pid->V_now) < 100 && fabs(pid->V_now) >= 30)
        pid->Kp_Sped = pid->Kp_def - 4.0f, pid->Ki_Sped = (pid->PPR >= 100) ? pid->Ki_def - 2.0f : pid->Ki_def - 3.0f;
    else if(fabs(pid->V_now) < 30 && fabs(pid->V_now) >= 15)
        pid->Kp_Sped = pid->Kp_def - 3.0f, pid->Ki_Sped = (pid->PPR >= 100) ? pid->Ki_def - 3.0f : pid->Ki_def - 4.0f;
    else
    {
        pid->Ki_Sped = pid->Ki_def - (double)lock_lv;
        pid->Kp_Sped = pid->Kp_def - (double)lock_lv;
        if(pid->PPR >= 100)
        {
            pid->Kp_Pos = (1.6f - (double)lock_lv/10);
        }
        else
            pid->Kp_Pos = (16.0f - (double)lock_lv/10);
    }
}


static void PID_set(PID_typedef *pid, double Kp, double Ki, double Kd)
{
    if(pid->mode == PID_Position)
    {
        pid->Kp_Pos = Kp;
        pid->Ki_Pos = Ki*pid->Delta_T;
        pid->Kd_Pos = Kd/pid->Delta_T;
    }
    else
    {
        pid->Kp_Sped = Kp;
        pid->Ki_Sped = Ki*pid->Delta_T;
        pid->Kd_Sped = Kd/pid->Delta_T;
    }
}

static void PID_Calulate(PID_typedef *pid, volatile int32_t* Value_Set)
{

    if(pid->mode == PID_Speed)
    {
        static uint8_t first_Lock = 0;
        
        if(*Value_Set != 0U)
        {
            // RPS to 0-250
            pid->V_now = (pid->Speed * Speed_scale) / pid->RPM;

            // if speed sets from 1 to 3 than lock mode
            if(*Value_Set < 4U)
            {
                pid->V_set = 0U;
                LocK_process(pid,*Value_Set);
                if(first_Lock == 0 && fabs(pid->Speed) == 0)
                {
                    first_Lock = 1; 
                    pid->Delta_T = 0.00005;
                    htim4.Init.Period = 100-1;
                    
                    HAL_TIM_Base_Init(&htim4);
                    pid->EnC_set = 0;
                    //Reset Data
                    pid->P_Sped = pid->I_Sped = pid->D_Sped = 0.0f;
                    pid->Output_Sped = 0.0f;
                    pid->lastOutput_Sped = pid->Output_Sped;
                    // None Lock
                    pid->Motor_Duty = (int16_t)pid->Output_Pos;
                    PID_Motor_Ctrl(pid->Motor_Duty);
                }
            }
            else
            {
                if(first_Lock == 1)
                {
                    pid->Delta_T = 0.005;
                    htim4.Init.Period = 10000-1;
                    HAL_TIM_Base_Init(&htim4);

                    pid->P_Pos = pid->I_Pos = pid->D_Pos = 0.0f;
                    pid->Output_Pos = 0.0f;
                    // None Lock
                    pid->Motor_Duty = (int16_t)pid->Output_Sped;
                    PID_Motor_Ctrl(pid->Motor_Duty);
                    first_Lock = 0;
                }
                if(*Value_Set > 250) *Value_Set = 250;
                // 0-250 to RPS
                pid->V_set = (*Value_Set * pid->RPM)/Speed_scale;
                pid->V_set = (pid->Dir == 0U)  ? pid->V_set : -pid->V_set;
                if(fabs(pid->E_Sped) < 2)
                {
                    if(pid->V_set >=5 && pid->V_set < 10)
                        pid->Ki_Sped = pid->Ki_def;
                    else if(pid->V_set >= 10 && pid->V_set < 50)
                        pid->Ki_Sped = pid->Ki_def-0.5;
                    else if(pid->V_set >= 50 && pid->V_set < 100)
                        pid->Ki_Sped = pid->Ki_def-0.6;
                    else if(pid->V_set >= 100 && pid->V_set < 150)
                        pid->Ki_Sped = pid->Ki_def-0.7;
                    else if(pid->V_set >= 150 && pid->V_set < 200)
                        pid->Ki_Sped = pid->Ki_def-0.8;
                    else
                        pid->Ki_Sped = pid->Ki_def-0.9;
                }
                else if(fabs(pid->E_Sped) >= 2 && fabs(pid->E_Sped) < 10)
                    pid->Ki_Sped = (pid->PPR >= 100) ? pid->Ki_def-11 : pid->Ki_def-4;
                else if(fabs(pid->E_Sped) >= 10 && fabs(pid->E_Sped) < 30)
                    pid->Ki_Sped = (pid->PPR >= 100) ? pid->Ki_def-10 : pid->Ki_def-3.5;
                else if(fabs(pid->E_Sped) >= 30 && fabs(pid->E_Sped) < 50)
                    pid->Ki_Sped = (pid->PPR >= 100) ? pid->Ki_def-9 : pid->Ki_def-3;
                else if(fabs(pid->E_Sped) >= 50 && fabs(pid->E_Sped) < 100)
                    pid->Ki_Sped = (pid->PPR >= 100) ? pid->Ki_def-8 : pid->Ki_def-2.5;
                else if(fabs(pid->E_Sped) >= 100 && fabs(pid->E_Sped) < 200)
                    pid->Ki_Sped = (pid->PPR >= 100) ? pid->Ki_def-7 : pid->Ki_def-2.0;
                else
                    pid->Ki_Sped = pid->Ki_def;

            }

            if(first_Lock == 0)
            {
                // Get Erorr
                pid->E_Sped = pid->V_set - pid->Speed;
                              
                pid->P_Sped = pid->E_Sped - pid->E1_Sped;
                pid->I_Sped = pid->Ki_Sped * pid->E_Sped * pid->Delta_T;
//                LimitMax(pid->I_Sped,maxI_pos);
                pid->D_Sped = (pid->E_Sped - 2.0f*pid->E1_Sped + pid->E2_Sped)/pid->Delta_T;

                // Calculate Ouput
                pid->Output_Sped = pid->lastOutput_Sped +
                                   (pid->Kp_Sped * pid->P_Sped) + pid->I_Sped +
                                   (pid->Kd_Sped * pid->D_Sped);

                //Litmit Ouput
                LimitMax(pid->Output_Sped,maxOuput);

                // Update feedback
                pid->lastOutput_Sped = pid->Output_Sped;
                pid->E2_Sped = pid->E1_Sped;
                pid->E1_Sped = pid->E_Sped;
                
                // Output
                pid->Motor_Duty = (int16_t)round(pid->Output_Sped);

                PID_Motor_Ctrl(pid->Motor_Duty);
                pid->EnC_now = 0;
            }
            else if(first_Lock == 1)
            {
                pid->E_Pos = (pid->EnC_set - pid->EnC_now);

                pid->P_Pos = pid->E_Pos;
//                pid->I_Pos += (0.5 * pid->Delta_T * (pid->E_Pos + pid->E1_Pos));
                pid->I_Pos += (pid->Ki_Pos * pid->E_Pos) * pid->Delta_T;
                pid->D_Pos = (pid->E_Pos - pid->E1_Pos) / pid->Delta_T;

                LimitMax(pid->I_Pos,maxI_pos);

                pid->Output_Pos = (pid->Kp_Pos * pid->P_Pos) +
                                  pid->I_Pos +
                                  (pid->Kd_Pos * pid->D_Pos);

                LimitMax(pid->Output_Pos,(maxOuput+1)/2);

                pid->E1_Pos = pid->E_Pos;

                // Output
                pid->Motor_Duty = (int16_t)round(pid->Output_Pos);

                PID_Motor_Ctrl(pid->Motor_Duty);
                
            }
        }
        else
        {
            //Reset Data
            pid->E_Pos = 0;
            pid->E1_Pos = pid->E2_Pos = pid->E_Pos;
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
        pid->EnC_set = (pid->Dir == 1)  ? (*Value_Set * 4) : -(*Value_Set * 4);

        pid->E_Pos = (pid->EnC_set - pid->EnC_now);

        pid->P_Pos = pid->E_Pos;
//        pid->I_Pos += (0.5 * pid->Delta_T * (pid->E_Pos + pid->E1_Pos));
//        pid->I_Pos += pid->Ki_Pos * pid->E_Pos;
//        pid->D_Pos = pid->E_Pos - pid->E1_Pos;
        pid->D_Pos = pid->EnC_now - pid->lastEnC;

        pid->sumOutput_Pos += pid->Ki_Pos * pid->E_Pos;

        pid->sumOutput_Pos -= pid->Kp_Pos *pid->D_Pos;

        LimitMax(pid->sumOutput_Pos,set_speed);

        pid->Output_Pos += pid->sumOutput_Pos - pid->Kd_Pos * pid->D_Pos;

//        pid->Output_Pos = (pid->Kp_Pos * pid->P_Pos) +
//                          (pid->I_Pos) +
//                          (pid->Kd_Pos * pid->D_Pos);

//        LimitMax(pid->Output_Pos,set_speed);


        LimitMax(pid->Output_Pos,set_speed);

        pid->lastEnC = pid->EnC_now;
//        pid->E1_Pos = pid->E_Pos;
        pid->Motor_Duty = (int16_t)round(pid->Output_Pos);
        /*******************************************************************************/

//        pid->V_set = (int32_t)(pid->Output_Pos*4);
//        if(pid->E_Pos == 0)
//        {
//            set_speed = 20;
//            pid->V_set = 0;
//        }
//        else
//        {
//            if(set_speed < 30)
//                pid->V_set = 0;
//        }

//        pid->E_Sped = pid->V_set - pid->Speed;

//        pid->P_Sped = pid->E_Sped - pid->E1_Sped;
//        pid->I_Sped = pid->E_Sped * pid->Delta_T;
//        pid->D_Sped = (pid->E_Sped - 2.0f*pid->E1_Sped + pid->E2_Sped)/pid->Delta_T;

//        pid->Output_Sped = pid->lastOutput_Sped +
//                           (pid->Kp_Sped * pid->P_Sped) +
//                           (pid->Ki_Sped * pid->I_Sped) +
//                           (pid->Kd_Sped * pid->D_Sped);

//        LimitMax(pid->Output_Sped,maxOuput);
//        pid->lastOutput_Sped = pid->Output_Sped;
//        pid->E2_Sped = pid->E1_Sped;
//        pid->E1_Sped = pid->E_Sped;

//        pid->Motor_Duty = (int16_t)round(pid->Output_Sped);

        PID_Motor_Ctrl(pid->Motor_Duty);
    }
    else if(pid->mode == None_PID)
    {
        pid->Motor_Duty = (pid->Dir == 0) ? ((int16_t)*Value_Set * Duty_scale) : -((int16_t)*Value_Set * Duty_scale);
        LimitMax(pid->Motor_Duty, maxOuput);
        PID_Motor_Ctrl(pid->Motor_Duty);
    }
}

void PID_Init(PID_typedef *pid, uint8_t Mode)
{
    pid->f_param_init = PID_param_init;
    pid->f_pid_set    = PID_set;
    pid->f_cal_pid	  = PID_Calulate;
    pid->mode		  = Mode;
}
