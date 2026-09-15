#include <Arduino.h>
#include <cmath>
#include <cstddef>

namespace
{
	constexpr uint32_t BAUD_RATE{9600};

	constexpr uint8_t LED_PIN{2};
	constexpr uint8_t BUTTON_PIN{12};
	constexpr uint8_t SPEAKER_PIN{18};

	constexpr uint16_t DEBOUNCE_DELAY_MS{50};
	constexpr uint16_t TONE_DURATION_MS{100};

	constexpr uint8_t ROWS{4};
	constexpr uint8_t COLS{4};
	constexpr uint8_t rowPins[ROWS]{13, 14, 27, 26};
	constexpr uint8_t colPins[COLS]{25, 33, 32, 4};

	constexpr char keypad[ROWS][COLS]{
		{'1', '2', '3', 'A'},
		{'4', '5', '6', 'B'},
		{'7', '8', '9', 'C'},
		{'*', '0', '#', 'D'}};

	constexpr uint8_t N_TABLE[ROWS][COLS]{
		40,
		41,
		42,
		0,
		43,
		44,
		45,
		0,
		46,
		47,
		48,
		0,
		49,
		50,
		51,
		0,
	};

	struct DebounceData
	{
		uint8_t lastRawState{LOW};
		uint8_t debouncedState{LOW};

		uint32_t startTime{};
		uint32_t lastReleaseTime{};

		uint32_t pressCount{};
	};

	DebounceData debounceData{};
} // namespace

void setup()
{
	Serial.begin(BAUD_RATE);
	Serial.println("Starting program...");

	pinMode(SPEAKER_PIN, OUTPUT);
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, HIGH);

	pinMode(BUTTON_PIN, INPUT_PULLDOWN);

	for (uint32_t row{}; row < ROWS; ++row)
	{
		pinMode(rowPins[row], INPUT_PULLDOWN);
	}

	for (uint32_t col{}; col < COLS; ++col)
	{
		pinMode(colPins[col], OUTPUT);
		digitalWrite(colPins[col], HIGH);
	}

	// Initialize the base states
	debounceData.lastRawState	= digitalRead(BUTTON_PIN);
	debounceData.debouncedState = debounceData.lastRawState;
}

void handleButtonPress()
{
	const uint8_t  rawButtonState{digitalRead(BUTTON_PIN)};
	const uint32_t currentTime{millis()};

	// Raw input changed, so restart debounce timer.
	if (rawButtonState != debounceData.lastRawState)
	{
		debounceData.lastRawState = rawButtonState;
	}

	// Check for valid duration
	if ((currentTime - debounceData.startTime) >= DEBOUNCE_DELAY_MS)
	{
		// Only process it if the stable state differs from
		// our current debounced state.
		if (rawButtonState != debounceData.debouncedState)
		{
			debounceData.debouncedState = rawButtonState;

			// Rising edge: button pressed.
			if (rawButtonState == HIGH)
			{
				debounceData.startTime = currentTime;
				++debounceData.pressCount;
			}

			// Falling edge: button released.
			else
			{

				Serial.printf(
					"Press count: %u - Press time: %f s\n",
					debounceData.pressCount,
					(currentTime - debounceData.startTime) / 1000.0);
			}
		}
	}
}

char getKeypadPress()
{
	for (size_t c{}; c < COLS; ++c)
	{
		// Select the current column for reading
		digitalWrite(colPins[c], HIGH);

		for (size_t r{}; r < ROWS; ++r)
		{
			// Check if the current element is pressed
			if (HIGH == digitalRead(rowPins[r]))
			{
				// Reset pin state
				pinMode(colPins[c], LOW);
				Serial.print(keypad[r][c]);

				return N_TABLE[r][c];
			}
		}

		// Reset pin state
		digitalWrite(colPins[c], LOW);
	}

	// Base case where no keys are pressed
	return NULL;
}

void handleTone(char key)
{
	const uint32_t frequency{440 * static_cast<uint32_t>(pow(2, ((key - 49)) / 12.0))};

	if (0 < frequency)
	{
		tone(SPEAKER_PIN, frequency, TONE_DURATION_MS);
	}
}

void loop()
{
	handleButtonPress();
	// char key = getKeypadPress();
	// handleTone(key);
}
