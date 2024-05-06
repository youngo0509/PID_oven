/*
 * pid.c
 *
 *  Created on: Nov 4, 2023
 *      Author: oy050
 */
#include "pid.h"
float pTerm, dTerm, iTerm;


float UpdatePID(SPid * pid, float error)
{
pid->integratMax=100;  // Maximum allowable integrator state
pid->integratMin=-5;
// calculate the proportional term
pTerm = pid->propGain * error;
// calculate the integral state with appropriate limiting
pid->integratState += error;
// Limit the integrator state if necessary
if (pid->integratState > pid->integratMax)
{
pid->integratState = pid->integratMax;
}
else if (pid->integratState < pid->integratMin)
{
pid->integratState = pid->integratMin;
}
// calculate the integral term
iTerm = pid->integratGain * pid->integratState;
// calculate the derivative
dTerm = pid->derGain * (pid->derState - error);
pid->derState = error;
return pTerm + dTerm + iTerm;
}
