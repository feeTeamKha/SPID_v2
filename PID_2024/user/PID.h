#ifndef _PID_H_
#define _PID_H_

#include "stm32f1xx.h"
#define ARR_SET 5000
#define frameSize 10
#define PID_Motor_Ctrl(Duty)  \
((Duty >= 0) ?   (TIM1->CCR1 = 0,TIM1->CCR2 = Duty) : (TIM1->CCR2 = 0, TIM1->CCR1 = -Duty));
typedef struct _data_Rev{
    uint8_t Rxbuffer[16];
    uint8_t rxData[16];
    uint8_t isCorrectData;
    uint8_t baudSelected;
    uint8_t isheader[3];
    uint8_t rxCnt;
    uint8_t rxDone;
}PID_data;

typedef enum {
	 PID_Speed = 4, None_PID, PID_Position = 9, 
} PID_mode;

typedef enum {
    Normal, ReStart,
}POS_mode;

typedef enum
{
   UARTCOM = 0, USB_C, CAN_BUS, PWM_mode
}PCP;// PID CONTROL PROTOCOL

typedef struct _Pid{
	PID_data rev_UART; // Du lieu dien khien UART
        
    PID_mode mode;     // Che do hoat dong cua PID 
    
    uint8_t ptlCtrl;  // Giao thuc dieu khien: UART, USB, CAN, PWM
    
    uint8_t ID;        // Dia chi board

	uint8_t Dir; 	   // Chiều Quay
    
    volatile uint8_t enablePID; // Cho phep tinh toan gia tri PID
    volatile uint8_t modePos;   // Chuc nang trong che do PID_POSITION
       
    volatile int16_t delta_EnC; // So xung trong thoi gian lay mau
    volatile uint32_t curr_En; //  So encoder hien tai
    volatile uint32_t prev_En; //  So encoder truoc do 
    
    volatile int32_t Value_Set;// Gia tri dieu khien dau vao toc do/vi tri
	volatile float V_set;	   // Toc do dat tu 0 - 250 trong PID Speed
    uint8_t V_now;             // Toc do hien tai tu 0 - 250
	volatile float Speed;      // Toc do hien tai vong/s
    float pulsetoRound;
      
    
    float Delta_T;              // Thoi gian lay mau
   
    int32_t EnC_set;            // Vi tri encoder can den(chi su dung trong tinh toan PID)
    volatile int32_t EnC_now;   // Vi tri encoder hien tai
    volatile int32_t lastEnC;   // Vi tri encoder tro do Delta_T giay
        
    volatile int32_t targetPos;  // Vi tri encoder can den (Gia tri dau vao PID POSITION)
    volatile uint16_t Time_step, lastTimeStep; // Thoi gian giua 2 xung dieu khien
    uint8_t scalefactor;         // Gia tri khueck dai dau vao PID POSITION
    
	uint16_t RPM;                // Gia tri Vong/Phut toi da
	uint16_t PPR;	             // So xung encoder/vong
    int16_t Motor_Duty;          // Do rong xung PWM dieu khien
    
    float E_Pos, E_Sped;        // Sai so hien tai e(k)
    float E1_Pos, E1_Sped;      // Sai so truoc do e(k-1)
    float E2_Pos, E2_Sped;      // Sai so truoc do e(k-2)

    float Kp_def, Ki_def, Kd_def;   // Thong so Kp,Ki,Kd default
    
    float Kp_Pos, Ki_Pos, Kd_Pos;   // Thong so Kp,Ki,Kd PID POSITION
    float Kp_Sped, Ki_Sped, Kd_Sped;// Thong so Kp,Ki,Kd PID Speed 
    float P_Pos, I_Pos, D_Pos;      // Thong so P, I, D PID POSITION
    float P_Sped, I_Sped, D_Sped;   // Thong so P, I, D PID Speed
    
    float Output_Pos, sumOutput_Pos;    // Kết quả tính toán PID POSITION
    float Output_Sped, lastOutput_Sped; // Kết quả tính toán PID Speed
	void (*f_param_init)(struct _Pid *pid); // Ham khoi tao PID
	void (*f_pid_set)(struct _Pid *pid, float Kp, float Ki, float Kd); // Ham set thong so Kp,Ki,Kd
	void (*f_cal_pid)(struct _Pid *pid);   // Ham tinh toan PID
    void (*f_update_setpoint)(struct _Pid *pid);
    void (*f_update_config)(struct _Pid *pid);

} PID_typedef;
extern PID_typedef PID;
void PID_Init(PID_typedef *pid);

#endif /* _PID_H_ */
