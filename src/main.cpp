#include "Wire.h"
#include <Arduino.h>
#include <LTR390.h>
#include <Tiny4kOLED_Wire.h>
#include <avr/sleep.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C

typedef enum {
  WDT_SLEEP_16MS = 0,
  WDT_SLEEP_32MS,
  WDT_SLEEP_64MS,
  WDT_SLEEP_125MS,
  WDT_SLEEP_250MS,
  WDT_SLEEP_500MS,
  WDT_SLEEP_1S,
  WDT_SLEEP_2S,
} WDT_sleep_t;

void BOD_off(void) {
  // The following two lines must be executed atomically
  MCUCR |= (1 << BODS) | (1 << BODSE); // Turn off brown-out detection
  MCUCR &= ~(1 << BODSE);              // Must be done right after setting BODS
}

// From datasheet.
void WDT_off(void) {
  __asm__ __volatile__("wdr");
  /* Clear WDRF in MCUSR */
  MCUSR = 0x00;
  /* Write logical one to WDCE and WDE */
  WDTCR |= _BV(WDCE) | _BV(WDE);
  /* Turn off WDT */
  WDTCR = 0x00;
}

void ADC_off(void) { ADCSRA &= ~_BV(ADEN); }
void ADC_on(void) { ADCSRA |= _BV(ADEN); }

// We're gonna to wake up from a watchdog timer interrupt.
ISR(WDT_vect) {
  WDT_off(); // Disable WDT on wake.
}

void deep_sleep(WDT_sleep_t duration) {
  BOD_off(); // NOTE: Enables back upon waking up.
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sei();
  {
    WDTCR |= duration & 0b111; // Set prescaler value.
    WDTCR |= _BV(WDIE);        // Set interrupt mode.
    WDTCR |= _BV(WDE);         // Enable the timer.
  }
  sleep_mode();
}

long get_vcc() {
  ADC_on();
  // Read 1.1V reference against AVcc.
  // Set the reference to Vcc and the measurement to the internal 1.1V reference.
  ADMUX = _BV(MUX3) | _BV(MUX2);
  delay(2);            // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC))
    ; // measuring
  ADC_off();

  uint8_t low = ADCL;  // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high << 8) | low;
  return 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
}

static LTR390 uv;

void setup() {
  oled.begin();
  oled.clear();
  oled.setFontX2Smooth(FONT6X8CAPS);
  oled.on();

  if (!uv.begin()) {
    oled.println("UV FAILURE");
    while (1)
      deep_sleep(WDT_SLEEP_2S);
  }

  oled.switchRenderFrame();
  oled.clear();

  long voltage = get_vcc();
  if (voltage > 2900) {
    oled.println("GOOD");
  } else if (voltage > 2800) {
    oled.println("PRETTY BAD");
  } else {
    oled.println("CHANGE");
  }
  oled.println("BATTERY");
  oled.switchFrame();
}

static uint32_t UVI;
static uint32_t LUX;

void loop() {
  // The recommended config to get the most accurate UVI readings.
  uv.setResolution(LTR390_RESOLUTION_20BIT);
  uv.setGain(LTR390_GAIN_18);
  uv.setRate(4); // 500 ms
  uv.setUVSMode();
  deep_sleep(WDT_SLEEP_500MS);
  UVI = uv.getUVIndex();
  // Fast measuring time with decent accuracy.
  uv.setResolution(LTR390_RESOLUTION_18BIT);
  uv.setGain(LTR390_GAIN_1);
  uv.setRate(2); // 100 ms
  uv.setALSMode();
  deep_sleep(WDT_SLEEP_125MS);
  LUX = uv.getLUX();

  // Show the data.
  oled.clear();
  oled.print("UVI: ");
  oled.println(UVI);
  oled.print("LUX: ");
  if (LUX > 99999) // No way to fit on the screen.
    oled.println("MAX");
  else
    oled.println(LUX);
  oled.switchFrame();
}
