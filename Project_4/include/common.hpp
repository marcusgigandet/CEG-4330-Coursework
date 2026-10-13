#pragma once

#include <Arduino.h>

#ifdef ESP32
constexpr uint8_t IN_PIN = 4; // connect to PWM_PIN with a jumper wire
#else
#	define IRAM_ATTR
constexpr uint8_t IN_PIN = 2; // connect to PWM_PIN with a jumper wire
#endif

// Vars

constexpr uint32_t BAUDRATE = 115200;

extern uint8_t  duty;
extern uint16_t freq;

// ISR
extern volatile uint32_t lowSumUS;
extern volatile uint32_t highSumUS;
extern volatile uint32_t edgeCount;

// Functions

void changeDutyCycle(long value);
void changeFrequency(long value);
void getSerialInput();
void displayIncomingPWM();

void loop();

// ISR
void IRAM_ATTR pwmISR();
