#include "common.hpp"
#include <Arduino.h>
#include <avr/io.h>

constexpr uint8_t PWM_PIN = 9; // OC1A, the Timer1 output pin

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

	// IN_PIN
	pinMode(IN_PIN, INPUT);
	attachInterrupt(digitalPinToInterrupt(IN_PIN), pwmISR, CHANGE);
}

void changeDutyCycle(long value)
{
	// Ignore invalid numbers
	if (value < 0 || value > 255)
	{
		return;
	}

	// Set duty
	OCR1A = value * ICR1 / 255;

	Serial.print("Set duty: ");
	Serial.println(value);
}

void changeFrequency(long value)
{
	// Ignore invalid numbers
	if (value < 100 || value > 10000)
	{
		return;
	}

	ICR1   = 1000000 / value; // period (us)

	Serial.print("Set frequency: ");
	Serial.println(value);
}
