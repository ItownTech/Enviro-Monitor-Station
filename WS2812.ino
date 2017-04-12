/*
  Projet ESP8266 Enviro Monitor Station
  Copyright (C) 2017 by Leon

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//#define NOISE_READING_DELAY   100
//#define NOISE_READING_WINDOW    20
//#define NOISE_BUFFER_SIZE       20

#define CLAP_DEBOUNCE_DELAY     150
#define CLAP_TIMEOUT_DELAY      1000
#define CLAP_SENSIBILITY        80
#define CLAP_COUNT_TRIGGER      4
#define CLAP_BUFFER_SIZE        7
#define CLAP_TOLERANCE          1.50

#define MAX_SERIAL_BUFFER       20

#define DEFAULT_EVERY           20
#define DEFAULT_PUSH            1
#define DEFAULT_CLAP            0
#define DEFAULT_THRESHOLD       0

#define RGB_WIPE				        1
#define RGB_RAINBOW				      2
#define RGB_RAINBOW_CYCLE		    3

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, RGB_PIN, NEO_GRB + NEO_KHZ800);

int clapTimings[CLAP_BUFFER_SIZE];
byte clapPointer = 0;

bool clap = DEFAULT_CLAP;
unsigned long every = 1000L * DEFAULT_EVERY;
unsigned int threshold = DEFAULT_THRESHOLD;

// RGB Global Variables
bool rgbExec = true;				      // Should code execute
int colorR, colorG, colorB = 0;		// Component colors.
int rgbEffect = RGB_WIPE;			    // default animation

unsigned int noise_buffer[NOISE_BUFFER_SIZE] = {0};
unsigned int noise_buffer_pointer = 0;

static unsigned long last = 0;

// -----------------------------------------------------------------------------
// MIC
// -----------------------------------------------------------------------------

void clapDecode() {

  // at least 2 claps
  if (clapPointer > 0) {

    byte code = 2;
    if (clapPointer > 1) {
      int length = clapTimings[0] * CLAP_TOLERANCE;
      for (byte i = 1; i < clapPointer; i++) {
        code <<= 1;
        if (clapTimings[i] > length) code += 1;
      }
    }
    //    link.send_P(at_code, code);
  }
  // reset
  clapPointer = 0;
}

void clapRecord(int value) {

  static bool reading = false;
  static unsigned long last_clap;
  static int counts = 0;
  unsigned long current = millis();
  unsigned long span = current - last_clap;

  if (value > CLAP_SENSIBILITY) {
    ++counts;
  } else {
    counts = 0;
  }

  if (counts == CLAP_COUNT_TRIGGER) {

    //Serial.print("Value: "); Serial.println(value);

    // Is it the first clap?
    if (!reading) {

      last_clap = current;
      reading = true;

      // or not
    } else {

      //Serial.print("Span : "); Serial.println(span);

      // timed out
      if (span > CLAP_TIMEOUT_DELAY) {
        clapDecode();
        // reset
        reading = false;
      } else if (span < CLAP_DEBOUNCE_DELAY) {
        // do nothing
        // new clap!
      } else if (clapPointer < CLAP_BUFFER_SIZE) {
        clapTimings[clapPointer] = span;
        last_clap = current;
        clapPointer++;
        // buffer overrun
      } else {
        clapPointer = 0;
        reading = false;
      }
    }
    // check if we have to process it
  } else if (reading) {
    if (span > CLAP_TIMEOUT_DELAY) {
      clapDecode();
      // back to idle
      reading = false;
    }
  }
}

void noiseLoop() {

  static unsigned long last_reading = 0;
  static unsigned int triggered = 0;

  unsigned int sample;
  //unsigned int count = 0;
  //unsigned long sum;
  unsigned int min = ADC_COUNTS;
  unsigned int max = 0;

  // Check MIC every NOISE_READING_DELAY
  // if (millis() - last_reading < NOISE_READING_DELAY) return;
  last_reading = millis();

  while (millis() - last_reading < NOISE_READING_WINDOW) {
    sample = analogRead(MIC_PIN);
    //++count;
    //sum += sample;
    if (sample < min) min = sample;
    if (sample > max) max = sample;
  }

  //++noise_count;
  //unsigned int average = 100 * (sum / count) / ADC_COUNTS;
  //noise_sum += average;

  unsigned int peak = map(max - min, 0, ADC_COUNTS, 0, 100);
  //Serial.println(peak);
  if (clap) clapRecord(peak);

  noise_buffer_sum = noise_buffer_sum + peak - noise_buffer[noise_buffer_pointer];
  noise_buffer[noise_buffer_pointer] = peak;
  noise_buffer_pointer = (noise_buffer_pointer + 1) % NOISE_BUFFER_SIZE;

  //noise_peak += peak;
  //if (max > noise_max) noise_max = max;
  //if (min < noise_min) noise_min = min;

  if (threshold > 0) {
    unsigned int value = noise_buffer_sum / NOISE_BUFFER_SIZE;
    if (value > threshold) {
      if (value > triggered) {
        client.publish(noise_topic, String(peak).c_str(), true);
        triggered = value;
      }
    } else if (triggered > 0) {
      client.publish(noise_topic, String(peak).c_str(), true);
      triggered = 0;
    }
  }
}


// Check to see if WS2812 light ring has any instructions awaiting
void rgbLoop(int rgbEffect = RGB_WIPE, bool restoreColor = true) {
  if (rgbExec == true) {
    switch (rgbEffect) {
      case RGB_WIPE:
        // Do wipe animation here
        colorWipe(strip.Color(colorR, colorG, colorB), 5);
        rgbExec = false;	// Only do this once...
        break;

      case RGB_RAINBOW:
        // Do Rainbow animation here
        rainbow(24);
        if (restoreColor) {
          rgbEffect = RGB_WIPE;
          rgbExec = true;
        }
        else {
          colorWipe(0, 0); // Switch off all LEDS No delay
        }
        break;

      case RGB_RAINBOW_CYCLE:
        rainbowCycle(24);
        if (restoreColor) {
          rgbEffect = RGB_WIPE;
          rgbExec = true;
        }
        else {
          colorWipe(0, 0); // Switch off all LEDS No delay
        }
        break;
    }
  }
}

// -----------------------------------------------------------------------------
// ws2812
// -----------------------------------------------------------------------------

void run_ws2812() {

//  if ((every > 0) && ((millis() - last > every) || (last == 0))) {
//    last = millis();
    // animate to indicate reading has occurred
    rgbExec = true;			  // Set the Exec flag to on
    rgbLoop(RGB_RAINBOW);	// Set the Animation type
//  }
  rgbLoop();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
