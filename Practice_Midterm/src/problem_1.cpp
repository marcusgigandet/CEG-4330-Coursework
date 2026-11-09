#include <Arduino.h>

struct BeltState
{
    uint32_t objectCount{};
    uint32_t lastTime{};
    uint32_t totalTime{};
    bool hasNewObject{};
    bool hasNewFrequency{};
};

// Pins
constexpr uint32_t BUTTON_PIN{23};

// Global variables
volatile BeltState state{};

void IRAM_ATTR risingEdgeISR()
{
	uint32_t currentTime{micros()};
    uint32_t totalTime{currentTime - state.lastTime};

    if (totalTime != state.totalTime)
    {
        state.totalTime = totalTime;
        state.hasNewFrequency = true;
    }

    // Update state
    state.lastTime = currentTime;
    state.objectCount += 1;
}

void setup()
{
	// Serial
	Serial.begin(9600);

	// Button
	pinMode(BUTTON_PIN, INPUT_PULLDOWN);
	attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), risingEdgeISR, RISING);
}

void loop()
{
    BeltState localState;
	noInterrupts();
	{
        localState.totalTime = state.totalTime;
        localState.objectCount = state.objectCount;
        localState.hasNewFrequency = state.hasNewFrequency;
        localState.hasNewObject = state.hasNewObject;

        // Reset flags
        state.hasNewFrequency = false;
        state.hasNewObject = false;
	}
	interrupts();

    // Display frequency
    if (localState.hasNewFrequency)
    {
        Serial.printf("Frequency %.2f\n", 1'000'000 / (float)localState.totalTime);
    }

    // Display count
    if (localState.hasNewObject)
    {
        Serial.printf("Count: %u\n", localState.objectCount);
    }
}
