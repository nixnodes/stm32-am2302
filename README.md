# stm32-am2302
Based on https://github.com/LonelyWolf/stm32/tree/master/am2302

While the original version relies on SysTick interrupt handler to measure impulse times, this one uses general purpose timers, thus ensuring accurate and reliable readings on a wider range of STM32 devices.
