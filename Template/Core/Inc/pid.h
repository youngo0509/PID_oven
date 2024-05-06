/*
 * pid.h
 *
 * Created on: Nov 4, 2023
 * Author: oy050
 */

#ifndef INC_PID_H_
#define INC_PID_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

// Define the SPid structure here
typedef struct
{
    float derState;    // Last position input
    float integratState; // Integrator state
    float integratMax;  // Maximum allowable integrator state
    float integratMin;  // Minimum allowable integrator state
    float integratGain; // integral gain
    float propGain;     // proportional gain
    float derGain;      // derivative gain
} SPid;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */
float UpdatePID(SPid *pid, float error);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /* INC_PID_H_ */
