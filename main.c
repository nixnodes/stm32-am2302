#include "tim.h"
#include "dht22.h"

#include <stm32f10x.h>

volatile static dht22_data data;

int main()
{
  /*
    ....
  */

  DTIM_Initialize();
  DHT22_Init();

  while (1)
  {
     /* Temporarily disable all interrupts to ensure accurate timing */
    __disable_irq ();

    if ( DHT22_Read (&data) == true )
    {
      /* Use data.temperature and data.humidity */
    } else {
      /* 
        Error - check response code in data.rcv_response, for more info 
        attach debugger and examine impulse times in uint8_t bits[40] (dht22.c) 
      */
    }

    __enable_irq ();

    Delay_ms(4000);
  }

}
