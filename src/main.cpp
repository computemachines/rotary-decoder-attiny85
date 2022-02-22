#define __DEBUG__

#include "avr/interrupt.h"
#include "avr/io.h"
#include "util/delay.h"

#include "common.h"

volatile uint16_t lastEncoded = 0;
volatile uint16_t position = 0;
volatile int8_t direction = 0;

int main()
{
  DDRB = _BV(PB0) | _BV(PB1) | _BV(PB2); // Set PB0..2 output, others input
  PORTB = _BV(PB3) | _BV(PB4);           // Enable Port B pullup resistor on PB3, PB4
  GIMSK = _BV(PCIE);                     // Enable Pin Change Interrupt, in General Interrupt Mask
  PCMSK = _BV(PCINT4) | _BV(PCINT3);     // Enable Pin Change Interrupt 4, in Pin Change Interrupt Mask
  sei();                                 // Enable global interrupt flag in status register

  while (true){}
}

#define FORWARD_PIN PB0
#define REVERSE_PIN PB1
#define ERROR_PIN PB2

volatile int8_t quadrature_transitions[4][4] = {
  {0, 1, -1, 0},
  {-1, 0, 0, 1},
  {1, 0, 0, -1},
  {0, -1, 1, 0}
};

volatile uint8_t quadrature_errors[4][4] = {
  {0, 0, 0, 1},
  {0, 0, 1, 0},
  {0, 1, 0, 0},
  {1, 0, 0, 0}
};

ISR(PCINT0_vect)
{
  uint8_t newEncoded = (PINB >> PB3) & 0b11;

  direction = quadrature_transitions[lastEncoded][newEncoded];
  switch (direction)
  {
  case 1:
    PULSE(FORWARD_PIN);
    break;
  
  case -1:
    PULSE(REVERSE_PIN);
    break;

  default:
    break;
  }

  if (quadrature_errors[lastEncoded][newEncoded]) {
    SET_IO(ERROR_PIN);
  }

  lastEncoded = newEncoded;
}
