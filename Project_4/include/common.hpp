#pragma once

#include <cstdint>

// Vars

constexpr uint32_t BAUDRATE = 115200;

uint8_t	 duty = 127;
uint16_t freq = 5000;

// ISR
volatile uint32_t lowSumUS	= 0;
volatile uint32_t highSumUS = 0;
volatile uint32_t edgeCount = 0;

// Functions

void changeDutyCycle(long value);
void changeFrequency(long value);
void getSerialInput();
void displayIncomingPWM();

void loop();