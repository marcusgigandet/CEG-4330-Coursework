#include <Arduino.h>
#include <driver/gpio.h>
#include <esp_sleep.h>

#define IR_PIN 12
#define LED_PIN 2

#define TIMER_FREQUENCY 10'000

#define TIMEOUT_DURATION_MICROS 5'000'000

#define START_BIT_DURATION_MS 13'500
#define ZERO_TRANSMITTED_DURATION_MS 1'125
#define ONE_TRANSMITTED_DURATION_MS 2'250
#define BUTTON_HELD_DURATION_MS 11'250 // Unused for this project
#define ERROR_MARGIN_PERCENT 10

#define CHECK_ERROR_MARGIN(x, time)                                                                \
	(x <= (time + (time / ERROR_MARGIN_PERCENT)) && x >= (time - (time / ERROR_MARGIN_PERCENT)))

#define LEFT_ARROW_BUTTON 0x0
#define RIGHT_ARROW_BUTTON 0x1
#define OK_BUTTON 0x2

enum class ir_message_state
{
	StartBit,
	InProgress,
	Complete,
	Idle,
};

volatile uint32_t		  lastTimeMicros;
volatile uint32_t		  irMessage{};
volatile ir_message_state irMessageState{ir_message_state::Idle};
uint64_t				  alarmValue = TIMER_FREQUENCY / 5;

hw_timer_t* timer{};

void IRAM_ATTR onTimerISR();
void IRAM_ATTR onIRFallingEdgeISR();

void setupIR();
void setupTimer();

void setup()
{
	// Configure the LED
	pinMode(LED_PIN, OUTPUT);

	esp_sleep_enable_gpio_wakeup();

	setupIR();
	setupTimer();
}

void loop()
{
	switch (irMessageState)
	{
		// Log new data
	case ir_message_state::Complete:
		// Mark data as read
		irMessageState = ir_message_state::Idle;

		// Log message
		Serial.printf("Bytes sent by IR: 0x%04X\n", irMessage);

		switch (irMessage)
		{
		case OK_BUTTON:
			// Toggle the LED
			digitalWrite(LED_PIN, !digitalRead(LED_PIN));
			break;

		case LEFT_ARROW_BUTTON:
			// Half the frequency
			alarmValue *= 2;
			break;

		case RIGHT_ARROW_BUTTON:
			// Double the frequency
			alarmValue /= 2;
			break;
		default:
			break;
		}

		// Check if alarm value is less than 1 Hz
		if (alarmValue > TIMER_FREQUENCY)
		{
			alarmValue = TIMER_FREQUENCY;
		}

		// Update alarm
		timerAlarm(timer, alarmValue, true, 0);

		break;

	case ir_message_state::Idle:
		// Check if the idle time has exceeded the timeout duration
		if (micros() - lastTimeMicros > TIMEOUT_DURATION_MICROS)
		{
			// Display sleep message
			Serial.println("Timeout duration exceed, entering low-power mode...");
			Serial.flush();

			// Disable interrupts before entering sleep state
			detachInterrupt(digitalPinToInterrupt(IR_PIN));

			// Disable LED
			digitalWrite(LED_PIN, LOW);

			// Enable the IR_PIN to wake microcontroller from sleep
			gpio_wakeup_enable((gpio_num_t)IR_PIN, GPIO_INTR_LOW_LEVEL);

			// Begin sleep state
			esp_light_sleep_start();

			// Display wakeup message
			Serial.println("Exited sleeping state, continuing to run...");
			setupIR();
		}
		break;

	default:
		break;
	}
}

void onTimerISR()
{
	// Toggle the LED
	digitalWrite(LED_PIN, 0 == digitalRead(LED_PIN));
}

void onIRFallingEdgeISR()
{
	static uint8_t bitIndex{};
	uint32_t	   timeSinceLastTimeMicros = micros() - lastTimeMicros;

	// Update time
	lastTimeMicros = micros();

	switch (irMessageState)
	{
	case ir_message_state::Idle:
		irMessage	   = 0;
		bitIndex	   = 0;
		irMessageState = ir_message_state::StartBit;
		break;

	case ir_message_state::StartBit:
		// Update message state
		if (CHECK_ERROR_MARGIN(START_BIT_DURATION_MS, timeSinceLastTimeMicros))
		{
			irMessageState = ir_message_state::InProgress;
		}
		else
		{
			irMessageState = ir_message_state::Idle;
		}
		break;

	// Check for 0 or 1 bit
	case ir_message_state::InProgress:
		// Set bit to 0
		if (CHECK_ERROR_MARGIN(ZERO_TRANSMITTED_DURATION_MS, timeSinceLastTimeMicros))
		{
			irMessage &= ~(1 << bitIndex);
		}

		// Set bit to 1
		else if (CHECK_ERROR_MARGIN(ONE_TRANSMITTED_DURATION_MS, timeSinceLastTimeMicros))
		{
			irMessage |= (1 << bitIndex);
		}

		bitIndex++;

		// Message is complete
		if (bitIndex == 32)
		{
			irMessageState = ir_message_state::Complete;
		}
		break;

	default:
		break;
	}
}

void setupIR()
{
	// Configure high for no signal
	pinMode(IR_PIN, INPUT_PULLUP);
	attachInterrupt(IR_PIN, onIRFallingEdgeISR, FALLING);
}

void setupTimer()
{
	timer = timerBegin(TIMER_FREQUENCY);
	timerAttachInterrupt(timer, onTimerISR);
	timerAlarm(timer, alarmValue, true, 0);
}
