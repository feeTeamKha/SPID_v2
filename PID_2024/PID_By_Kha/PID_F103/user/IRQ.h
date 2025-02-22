#ifndef _IRQ_H_
#define _IRQ_H_

#include "stm32f1xx.h"
#include <PID.h>
#define len(x) (sizeof(x)/sizeof(x[0]))

void Update_Encoder(PID_typedef *pid);
#endif /* _PID_SPEED_H_ */
