#include <Arduino.h>

// Global variables
constexpr uint8_t  SENSOR_PIN{13};
constexpr uint32_t DEBOUNCE_WAIT_US{1'000};
constexpr uint32_t US{1'000'000};

// Interrupt-related variables
volatile bool	  objectDetected = false;
volatile uint32_t totalTime		 = 0;
volatile uint32_t objectCount	 = 0;

/**
 * @brief Call on each rising edge of the sensor.
 *
 * This is used to dermine if an object is detected.
 */
void IRAM_ATTR onRisingISR()
{
	static uint32_t lastTime{0};
	uint32_t		currentTime{micros()};

	// Prevent accidental interrupts from incrementing count
	if (currentTime - lastTime < DEBOUNCE_WAIT_US)
	{
		return;
	}

	// Update the total time calculation
	totalTime = currentTime - lastTime;

	// Update the last time.
	lastTime = currentTime;

	// Increment and update the flag
	objectCount++;
	objectDetected = true;
}

void setup()
{
	// Serial
	Serial.begin(9600);

	// Sensor
	pinMode(SENSOR_PIN, INPUT_PULLDOWN);
	attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), onRisingISR, RISING);
}

void loop()
{
	// Log frequency
	if (objectDetected)
	{
		// Object count
		Serial.printf("Products detected: %u", objectCount);

		// Frequency
		Serial.printf("Frequency per sec: %.2f\n", (float)US / (float)totalTime);

		// Update state
		objectDetected = false;
	}
}
