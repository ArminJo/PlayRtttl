/*
 * RandomMelody.cpp
 *
 * Plays a melody from FLASH.
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

char StarWarsInRam[] =
        "StarWars:d=32,o=5,b=45,l=2,s=N:p,f#,f#,f#,8b.,8f#.6,e6,d#6,c#6,8b.6,16f#.6,e6,d#6,c#6,8b.6,16f#.6,e6,d#6,e6,8c#6";

void setup() {
    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__)
    while (!Serial)
        ; //delay for Leonardo
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));

    /*
     * Play one melody
     */
    playRtttlBlocking(TONE_PIN, StarWarsInRam);
    delay(5000);
}

void loop() {
    /*
     * And all the others, but use non blocking functions
     */
    for (uint8_t i = 1; i < ARRAY_SIZE_MELODIES_SMALL; ++i) {
#if defined(__AVR__)
        const char* tSongPtr = (char*) pgm_read_word(&RTTTLMelodiesSmall[i]);
        startPlayRtttlPGM(TONE_PIN, tSongPtr);
#else
        char* tSongPtr = (char*) RTTTLMelodiesSmall[i];
        startPlayRtttl(TONE_PIN, tSongPtr);
#endif
        while (updatePlayRtttl()) {
            /*
             * your own code here...
             */
            delay(1);
        }
        delay(2000);
    }
    delay(20000);
}

