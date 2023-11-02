#pragma once

#include <Arduino.h>
#include <stdint.h>
#include "constants.h"

static int randomNumberGenerator(uint8_t *dest, unsigned size)
{
  // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of
  // random noise). This can take a long time to generate random data if the result of analogRead(0)
  // doesn't change very frequently.
  while (size)
  {
    uint8_t val = 0;
    for (unsigned i = 0; i < 8; ++i)
    {
      int init = analogRead(0);
      int count = 0;
      while (analogRead(0) == init)
        ++count;

      if (count == 0)
        val = (val << 1) | (init & 0x01);
      else
        val = (val << 1) | (count & 0x01);
    }
    *dest = val;
    ++dest;
    --size;
  }
  // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
  return 1;
}

static uint8_t char2int(char input)
{
  if (input >= '0' && input <= '9')
    return input - '0';
  if (input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if (input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  return 0;
}

static void hex2bin(const char *src, uint8_t *target)
{
  while (*src && src[1])
  {
    *(target++) = char2int(*src) * 16 + char2int(src[1]);
    src += 2;
  }
}
