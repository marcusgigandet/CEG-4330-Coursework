#include "common.hpp"
#include <Arduino.h>

constexpr uint8_t PWM_PIN = 9; // OC1A, the Timer1 output pin
constexpr uint8_t IN_PIN  = 2; // connect to PWM_PIN with a jumper wire

ISR(pwmISR)
{

}

void setup()
{
	// Setup serial monitor
	Serial.begin(115200);

	// Setup Timer1 for PWM: phase correct, prescaler 8.
	// The timer counts 0 -> ICR1 -> 0 at 2 MHz, so one period is ICR1 microseconds.
	TCCR1A = _BV(COM1A1) | _BV(WGM11);		   // PWM output on OC1A
	TCCR1B = _BV(WGM13) | _BV(CS11);		   // top = ICR1, prescaler 8
	ICR1   = 1000000 / freq;				   // period (us)
	OCR1A  = (unsigned long)duty * ICR1 / 255; // high time (us)

	// Setup GPIO and interrupt
	pinMode(PWM_PIN, OUTPUT);
}

void changeDutyCycle(long value)
{
	// Ignore invalid numbers
	// Ignore invalid numbers
	if (value > 255)
	{
		return;
	}

	Serial.printf("Set duty: %ld", value);
}

void changeFrequency(long value)
{
	// Ignore invalid numbers
	if (value < 100 || value > 10000)
	{
		return;
	}

	Serial.printf("Set frequency: %ld", value);
}
