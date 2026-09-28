#include <Arduino.h>

#define HUMIDITY_PIN 34

#define ADC_MAX 4095
#define FRAME_COUNT 5

uint32_t dataFrame[FRAME_COUNT] = {};
uint8_t frameIndex = 0;

void setup()
{
    Serial.begin(9600);
    pinMode(HUMIDITY_PIN, ANALOG);
    
    // Initialize array
    for (uint8_t i = 0; i < FRAME_COUNT; ++i)
    {
        dataFrame[i] = analogRead(HUMIDITY_PIN);
    }
}

uint32_t filterRawData(uint32_t newData)
{
    dataFrame[frameIndex] = newData;
    frameIndex = (frameIndex + 1) % FRAME_COUNT;

    uint32_t sum = 0;

    for (uint8_t i = 0; i < FRAME_COUNT; ++i)
    {
        sum += dataFrame[i];
    }

    return sum / FRAME_COUNT;
}

void loop()
{
    const auto data = analogRead(HUMIDITY_PIN);

    Serial.println(100 * (float)filterRawData(data) / ADC_MAX);

    delay(1000);
}
