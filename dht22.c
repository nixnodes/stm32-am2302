#include "dht22.h"
#include "tim.h"

#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>

static uint8_t bits[40];

static uint8_t hMSB = 0;
static uint8_t hLSB = 0;
static uint8_t tMSB = 0;
static uint8_t tLSB = 0;
static uint8_t parity_rcv = 0;

static GPIO_InitTypeDef PORT;

void
DHT22_Init (void)
{
  RCC_APB2PeriphClockCmd (DHT22_GPIO_CLOCK, ENABLE);
  PORT.GPIO_Pin = DHT22_GPIO_PIN;
  PORT.GPIO_Speed = GPIO_Speed_50MHz;
}

uint32_t
DHT22_GetReadings (void)
{
  uint8_t i;
  uint16_t c;

  // Switch pin to output
  PORT.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init (DHT22_GPIO_PORT, &PORT);

  // Generate start impulse
  DHT22_GPIO_PORT->BRR = DHT22_GPIO_PIN; // Pull down SDA (Bit_SET)
  Delay_ms (2); // Host start signal at least 1ms
  DHT22_GPIO_PORT->BSRR = DHT22_GPIO_PIN; // Release SDA (Bit_RESET)

  // Switch pin to input with Pull-Up
  PORT.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init (DHT22_GPIO_PORT, &PORT);

  // Wait for AM2302 to begin communication (20-40us)
  DELAY_US_TIM->CNT = 0;
  while (((c = DELAY_US_TIM->CNT) < 100)
      && (DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN))
    ;

  DELAY_US_TIM->CNT = 0;

  if (c >= 100)
    return DHT22_RCV_NO_RESPONSE;

  // Check ACK strobe from sensor  
  while (((c = DELAY_US_TIM->CNT) < 100)
      && !(DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN))
    ;

  DELAY_US_TIM->CNT = 0;

  if ((c < 65) || (c > 95))
    return DHT22_RCV_BAD_ACK1;
  
  while (((c = DELAY_US_TIM->CNT) < 100)
      && (DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN))
    ;

  DELAY_US_TIM->CNT = 0;

  if ((c < 65) || (c > 95))
    return DHT22_RCV_BAD_ACK2;

  // ACK strobe received --> receive 40 bits
  i = 0;

  while (i < 40)
    {
      // Measure bit start impulse (T_low = 50us)
      
      while (((c = DELAY_US_TIM->CNT) < 100)
	  && !(DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN))
	;

      if (c > 75)
	{
	  // invalid bit start impulse length
	  bits[i] = 0xff;
	  while (((c = DELAY_US_TIM->CNT) < 100)
	      && (DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN))
	    ;
          DELAY_US_TIM->CNT = 0;
	}
      else
	{
	  // Measure bit impulse length (T_h0 = 25us, T_h1 = 70us)
	  DELAY_US_TIM->CNT = 0;
	  while (((c = DELAY_US_TIM->CNT) < 100)
	      && (DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN))
	    ;
          DELAY_US_TIM->CNT = 0;
	  bits[i] = (c < 100) ? (uint8_t) c : 0xff;
	}

      i++;
    }

  for (i = 0; i < 40; i++)
    if (bits[i] == 0xff)
      return DHT22_RCV_TIMEOUT;

  return DHT22_RCV_OK;
}

uint16_t
DHT22_DecodeReadings (void)
{
  uint8_t parity;
  uint8_t i = 0;

  hMSB = 0;
  for (; i < 8; i++)
    {
      hMSB <<= 1;
      if (bits[i] > 48)
	hMSB |= 1;
    }
  hLSB = 0;
  for (; i < 16; i++)
    {
      hLSB <<= 1;
      if (bits[i] > 48)
	hLSB |= 1;
    }
  tMSB = 0;
  for (; i < 24; i++)
    {
      tMSB <<= 1;
      if (bits[i] > 48)
	tMSB |= 1;
    }
  tLSB = 0;
  for (; i < 32; i++)
    {
      tLSB <<= 1;
      if (bits[i] > 48)
	tLSB |= 1;
    }
  for (; i < 40; i++)
    {
      parity_rcv <<= 1;
      if (bits[i] > 48)
	parity_rcv |= 1;
    }

  parity = hMSB + hLSB + tMSB + tLSB;

  return (parity_rcv << 8) | parity;
}

uint16_t
DHT22_GetHumidity (void)
{
  return (hMSB << 8) + hLSB;
}

uint16_t
DHT22_GetTemperature (void)
{
  return (tMSB << 8) + tLSB;
}

bool
DHT22_Read (volatile dht22_data *out)
{
  out->rcv_response = DHT22_GetReadings ();
  if (out->rcv_response != DHT22_RCV_OK)
    {
      return false;
    }

  uint32_t response = DHT22_DecodeReadings ();

  out->received = response & 0xff;
  out->actual = response >> 8;

  if (out->received != out->actual)
    {
      out->rcv_response = DHT22_BAD_DATA;
      return false;
    }

  out->humidity = (float) DHT22_GetHumidity () / 10.0f;

  uint16_t temperature = DHT22_GetTemperature ();
  out->temperature = ((float) (temperature & 0x7fff)) / 10.0f;

  if (temperature & 0x8000)
    {
      out->temperature = -out->temperature;
    }

  return true;

}
