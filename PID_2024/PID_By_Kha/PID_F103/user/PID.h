#ifndef _PID_SPEED_H_
#define _PID_SPEED_H_

#include "stm32f1xx.h"

#define PID_Motor_Ctrl(Duty)    \
{                               \
    if(Duty >= 0)               \
    {                           \
        TIM1->CCR1 = 0;         \
        TIM1->CCR2 = Duty;      \
    }                           \
    else                        \
    {                           \
        TIM1->CCR2 = 0;         \
        TIM1->CCR1 = -Duty;     \
    }                           \
}                               \

typedef struct _data_Rev{
    uint8_t rxData[4];
    uint8_t isCorrectData;
}PID_data;

typedef enum {
	PID_Speed, PID_Position, None_PID,
} PID_mode;

typedef struct _Pid{
	PID_mode mode;
    
    uint8_t ID;

	uint8_t Dir; 	// Chiều Quay
    
    volatile uint8_t enablePID;
    
    uint8_t scalefactor;
    
    volatile int16_t delta_EnC;
    volatile uint16_t curr_En; // Số encoder đo được
    volatile uint16_t prev_En;
    
	int32_t V_set;	// Tốc độ đặt 0 - 255
    double V_now;
	volatile double Speed;
    
    double Delta_T; // Thời gian lấy mẫu 

    double prev_I_Pos, prev_I_Sped;
    
    int32_t EnC_set;
    volatile int32_t EnC_now;
    volatile int32_t lastEnC;
        
    uint16_t Sped_Pos;
    
	uint16_t RPM;   // Số vòng quay/phút
	uint16_t PPR;		// số xung encoder/vòng
    int16_t Motor_Duty;

    double E_Pos, E_Sped;
    double E1_Pos, E1_Sped;
    double E2_Pos, E2_Sped;

    double Kp_def, Ki_def, Kd_def;
    
    double Kp_Pos, Ki_Pos, Kd_Pos;
    double Kp_Sped, Ki_Sped, Kd_Sped;
    double P_Pos, I_Pos, D_Pos;
    double P_Sped, I_Sped, D_Sped;
    
    PID_data pid_rev;
    
    double Output_Pos, sumOutput_Pos; // Kết quả tính toán PID
    double Output_Sped, lastOutput_Sped; // Kết quả tính toán PID
	void (*f_param_init)(struct _Pid *pid, double Kp,double Ki, double Kd);
	void (*f_pid_set)(struct _Pid *pid, double Kp, double Ki, double Kd);
	void (*f_cal_pid)(struct _Pid *pid,volatile int32_t* Value_Set);

} PID_typedef;



void PID_Init(PID_typedef *pid, uint8_t Mode);

#endif /* _PID_SPEED_H_ */
