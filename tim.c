#include "tim.h"

#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>


void
Delay_us (uint16_t nTime)
{
  DELAY_US_TIM->CNT = 0;

  while (DELAY_US_TIM->CNT < nTime)
    ;
}

void
Delay_ms (uint16_t nTime)
{
  DELAY_MS_TIM->CNT = 0;

  while (DELAY_MS_TIM->CNT < nTime)
    ;
}

void
DTIM_Initialize ()
{
  RCC_APB1PeriphClockCmd (DELAY_US_TIM_APB, ENABLE);

  TIM_TimeBaseInitTypeDef TIM;
  TIM_TimeBaseStructInit (&TIM);
  TIM.TIM_Prescaler = (uint16_t) (SystemCoreClock / 1000000) - 1;
  TIM.TIM_Period = UINT16_MAX;
  TIM.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit (DELAY_US_TIM, &TIM);

  TIM_Cmd (DELAY_US_TIM, ENABLE);

  RCC_APB1PeriphClockCmd (DELAY_MS_TIM_APB, ENABLE);

  TIM_TimeBaseStructInit (&TIM);
  TIM.TIM_Prescaler = (uint16_t) (SystemCoreClock / 1000) - 1;
  TIM.TIM_Period = UINT16_MAX;
  TIM.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit (DELAY_MS_TIM, &TIM);

  TIM_Cmd (DELAY_MS_TIM, ENABLE);

}
