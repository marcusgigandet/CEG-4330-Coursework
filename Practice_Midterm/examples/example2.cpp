#include <Arduino.h>
#define BUTTON_PIN 4
#define LED_PIN 2

void setup()
{
  // Start the serial monitor
  Serial.begin(9600);
  
  // Setup button and LED pins
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
}

// Used for debouncing
uint32_t lastLow = 0;
uint32_t lastHigh = 0;

// Track when a start or release has occurred
uint32_t startPress = 0;
uint32_t releasePress = 0;

// Track number of presses and releases
uint8_t actions = 0;
uint32_t durations[256] = {0};

// Used if we are playing
bool play = false;
uint8_t playIndex = 0;

void loop()
{
  // Get the current time and button state
  uint32_t now = micros();
  bool wasPressed = digitalRead(BUTTON_PIN);

  // Record the timings of when high and when low
  if (wasPressed) lastHigh = now;
  else lastLow = now;

  // Pressed with debouncing
  if (wasPressed && lastLow > 1000)
  {
    digitalWrite(LED_PIN, HIGH);
    play = false;
    // Record now as the time started pressing
    startPress = now;
    // If we already pressed in the past, record the previous period of being released
    if (actions > 0)
    {
      durations[actions] = startPress - releasePress;
      actions++;

      // Avoid going over the end of the array
      if (actions >= 255) actions = 254;
    }
  }

  // Released with debouncing
  if (!wasPressed && lastHigh > 1000)
  {
    // Record now as the time started releasing
    releasePress = now;
    durations[actions] = releasePress - startPress;
    durations[actions + 1] = 0;

    // Avoid going over the end of the array
    actions++;
    if (actions >= 255) actions = 254;
  }

  // If we have not pressed the button in 5 seconds, play
  if (lastHigh > 5000000 && !play)
  {
    play = true;
    playIndex = 0;
    digitalWrite(LED_PIN, LOW);
  }

  // As long as we're not at the end
  while(play && playIndex < 256 && durations[playIndex] != 0)
  {
    // Wait, toggle the LED, then move to the next duration
    delay(durations[playIndex] / 1000);
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    playIndex++;
  }

  // If we exceeded our array or we don't have any more durations, reset
  if (playIndex >= 256) playIndex = 0;
  if (durations[playIndex] == 0) playIndex = 0;
}