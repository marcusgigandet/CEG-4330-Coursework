// Write an ESP32 program using the Arduino libraries to do the following:
//
// Use a GPIO pin and an interrupt to measure rainfall using a tipping-bucket rain gauge.
// Assume the gauge outputs a short active-high pulse on its reed switch line each time the bucket
// tips. Each tip corresponds to 0.2 mm of rainfall.
//
// You must print two quantities to the serial monitor, only if they change:
// - The total accumulated rainfall in millimeters since the system powered on
// - The rainfall rate in millimeters per hour, computed over a sliding 60-second window
//
// Constraints:
// - No floating-point math inside the interrupt service routine;
//   the ISR must only record timestamps or counts
// - The rainfall rate must decay to zero if no tips are received for more than 60 seconds
// - Serial output must not occur inside the ISR

#include <Arduino.h>

constexpr uint8_t RAINGAUGE_PIN = 4; // reed switch pulse input
constexpr double  MM_PER_TIP	= 0.2;

// Written only by the ISR
volatile uint32_t tipCount	= 0;	 // total tips since boot
volatile uint32_t lastTipMs = 0;	 // timestamp of most recent tip
volatile uint32_t prevTipMs = 0;	 // timestamp of the tip before it
volatile bool	  haveTwo	= false; // true once two tips have been seen

void IRAM_ATTR tipIsr()
{
	uint32_t now = millis();
	if (tipCount > 0)
	{
		prevTipMs = lastTipMs;
	}
	if (tipCount >= 1)
	{
		haveTwo = true;
	}
	lastTipMs = now;
	tipCount++;
}

uint32_t lastPrintedCount = 0;
double	 lastPrintedRate  = -1.0;

void setup()
{
	Serial.begin(115200);
	pinMode(RAINGAUGE_PIN, INPUT_PULLDOWN); // active-high pulse
	attachInterrupt(digitalPinToInterrupt(RAINGAUGE_PIN), tipIsr, RISING);
}

void loop()
{
	// ---- Snapshot ISR variables with interrupts masked ----
	noInterrupts();
	uint32_t count = tipCount;
	uint32_t tLast = lastTipMs;
	uint32_t tPrev = prevTipMs;
	bool	 two   = haveTwo;
	interrupts();

	// ---- Total rainfall: print only on change ----
	if (count != lastPrintedCount)
	{
		double totalMM = count * MM_PER_TIP;
		Serial.print("Total rainfall: ");
		Serial.print(totalMM, 1);
		Serial.println(" mm");
		lastPrintedCount = count;
	}

	// ---- Rainfall rate from the interval between the last two tips ----
	// rate = (0.2 mm / interval) scaled to mm/h
	if (two && count != lastPrintedCount)
	{
		uint32_t interval = tLast - tPrev; // millis, wraps safely
		if (interval > 0)
		{
			double rateMMh = MM_PER_TIP * 3600000.0 / interval;
			if (rateMMh != lastPrintedRate)
			{
				Serial.print("Rainfall rate: ");
				Serial.print(rateMMh, 2);
				Serial.println(" mm/h");
				lastPrintedRate = rateMMh;
			}
		}
	}
}
