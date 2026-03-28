#include "Arduino.h"

#define PIN_A8 PA8
#define PIN_A9 PA9

#define A8_SET  (GPIOA->BSRR = (1<<8))
#define A8_CLR  (GPIOA->BSRR = (1<<(8+16)))
#define A9_SET  (GPIOA->BSRR = (1<<9))
#define A9_CLR  (GPIOA->BSRR = (1<<(9+16)))

void setup() {
  pinMode(PIN_A8, OUTPUT);
  pinMode(PIN_A9, OUTPUT);
}

void loop() {
  A8_SET;
  delayMicroseconds(4);
  A8_CLR;

  delayMicroseconds(1);

  A9_SET;
  delayMicroseconds(4);
  A9_CLR;

  delayMicroseconds(1);
}
