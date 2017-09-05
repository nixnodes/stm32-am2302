#ifndef TIM_H_
#define TIM_H_

#define DELAY_US_TIM_APB	RCC_APB1Periph_TIM6
#define DELAY_US_TIM		TIM6

#define DELAY_MS_TIM_APB	RCC_APB1Periph_TIM7
#define DELAY_MS_TIM		TIM7

#include <stdint.h>

void
Delay_us (uint16_t nTime);
void
Delay_ms (uint16_t nTime);

void
DTIM_Initialize ();

#endif

