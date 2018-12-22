/*
 * LightToTone.cpp
 *
 * Plays a pitch based on the light intensity.
 * As long as light intensity is below a threshold a random melody is played.
 * If a TEMT6000 module is attached, this value takes precedence over the LDR value.
 *
 *  Copyright (C) 2018  Armin Joachimsmeyer
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
#include "Int0Button.h"

#define VERSION_EXAMPLE "1.1"

const int TONE_PIN = 11;

const int LDR_PIN = A4; // LDR connected to Ground and 4700 Ohm connected to VCC (depends on the LDR type).
#define LDR_DARK_VALUE 40 // since LDR Voltage can hardly drop to zero

const int TEMT_6000_PIN = A3;
bool isTEMTConnected = false;

#define LIGHT_THRESHOLD 4 // If the analog reading is below this value, a random melody is played

static int sMinimum = 1024, sMaximum;

void setup() {
    Serial.begin(115200);
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));

    pinMode(LDR_PIN, INPUT);
    initINT0InterruptForButton0AtPin2();

    /*
     * Check for TEMT_6000 connected - 10kOhm to GND
     */
    pinMode(TEMT_6000_PIN, INPUT_PULLUP); // 100kOhm to VCC
    delay(2);
    int tTEMTValue = analogRead(TEMT_6000_PIN);
    pinMode(TEMT_6000_PIN, INPUT);
    Serial.print("TEMT test value=");
    Serial.println(tTEMTValue);
    if (tTEMTValue < 950) {
        // assume TEMT6000 connected
        isTEMTConnected = true;
        Serial.println("Assume TEMT6000 connected");
    }
    sMinimum = analogRead(LDR_PIN);
    sMaximum = sMinimum + LIGHT_THRESHOLD + 2;
    randomSeed(analogRead(LDR_PIN));
}

void loop() {

    int tLightValue = analogRead(LDR_PIN);

    if (tLightValue < sMinimum || tLightValue > sMaximum) {
        if (tLightValue < sMinimum) {
            sMinimum = tLightValue;
        }
        if (tLightValue > sMaximum) {
            sMaximum = tLightValue;
        }
        Serial.print("Minimum=");
        Serial.print(sMinimum);
        Serial.print(" Maximum=");
        Serial.println(sMaximum);
    }

    Serial.print("LDR=");
    Serial.print(tLightValue);

    tLightValue = sMaximum - tLightValue;
    Serial.print(" / ");
    Serial.print(tLightValue);
    if (isTEMTConnected) {
        tLightValue = analogRead(TEMT_6000_PIN);
        Serial.print(" TEMT=");
        Serial.print(tLightValue);
    }
    Serial.println();
    if (sButton0ToggleState) {
        /*
         * Play pentatonic notes
         */
        uint8_t tIndex = map(tLightValue, 0, sMaximum - sMinimum, 0, ARRAY_SIZE_NOTE_C5_TO_C7_PENTATONIC - 1);
        Serial.print("Index=");
        Serial.println(tIndex);
        uint16_t tFrequency = NoteC5ToC7Pentatonic[tIndex];
        tone(TONE_PIN, tFrequency);
        delay(200); // add an additional delay to make is easier to play a melody
    } else {
        if (tLightValue < LIGHT_THRESHOLD) {
            /*
             * Play random melody
             */
            startPlayRandomRtttlFromArrayPGM(TONE_PIN, RTTTLMelodies, ARRAY_SIZE_MELODIES);
            int tThresholdCount = 0;
            while (checkForRtttlToneUpdate()) {
                delay(10);
                if (isTEMTConnected) {
                    tLightValue = analogRead(TEMT_6000_PIN);
                } else {
                    tLightValue = sMaximum - analogRead(LDR_PIN);
                }
                if (tLightValue > LIGHT_THRESHOLD) {
                    tThresholdCount++;
                    if (tThresholdCount > 10) {
                        stopPlayRtttl();
                        break; // not really needed here, since the while condition will also change because of stopPlayRtttl.
                    }
                } else {
                    tThresholdCount = 0;
                }
            }
            delay(500);
        } else {
            /*
             * Play tone
             */
            tone(TONE_PIN, tLightValue);
            delay(50);
        }
    }
}

