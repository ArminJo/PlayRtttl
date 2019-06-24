/*
 * RandomMelody.cpp
 *
 * Plays a random melody. If you here the same melody twice and miss some melodies, than you get an idea of pseudo random.
 *
 * More RTTTL songs can be found under http://www.picaxe.com/RTTTL-Ringtones-for-Tune-Command/
 *
 *  Copyright (C) 2019  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of PlayRttl https://github.com/ArminJo/PlayRttl.
 *
 *  PlayRttl is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#include <Arduino.h>

#include <PlayRtttl.h>

#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#include "TinySerialOut.h"
#endif

#define VERSION_EXAMPLE "1.0"

const int TONE_PIN = 11;
const int BUTTON_PIN = 2;

void setup() {
    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__)
    while (!Serial)
        ; //delay for Leonardo
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));
    pinMode(LED_BUILTIN, OUTPUT);

    // get "true" random
    randomSeed(analogRead(0));

    // enable button press detection
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    setDefaultStyle(RTTTL_STYLE_CONTINUOUS);
}

void toggleLED() {
    static int tCount = 0; // to enable little delays but slow blink
    if (++tCount == 10) {
        tCount = 0;
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
    delay(10);
}

void loop() {
    /*
     * Play random melody
     * If you here the same melody twice and miss some melodies, than you get an idea of pseudo random.
     */
#if defined(__AVR__)
#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
    // use smaller array to fit into FLASH
    startPlayRandomRtttlFromArrayPGM(TONE_PIN, RTTTLMelodiesTiny, ARRAY_SIZE_MELODIES_TINY);
#else
    startPlayRandomRtttlFromArrayPGMAndPrintName(TONE_PIN, RTTTLMelodies, ARRAY_SIZE_MELODIES, &Serial);
#endif
#else
    startPlayRandomRtttlFromArrayAndPrintName(TONE_PIN, RTTTLMelodies, ARRAY_SIZE_MELODIES, &Serial);
#endif
// for Christmas
//    startPlayRandomRtttlFromArrayPGMAndPrintName(TONE_PIN, RTTTLChristmasMelodies, ARRAY_SIZE_CHRISTMAS_SONGS, &Serial);

    while (updatePlayRtttl()) {
        /*
         * Blink LED
         */
        toggleLED();
        /*
         * Check if button is pressed.
         * If yes stop melody wait and start with next loop
         */
        if (digitalRead(BUTTON_PIN) == LOW) {
            // stop playing melody
            stopPlayRtttl();
            break;
        }
    }
    // wait after playing
    delay(1000);
}

