/*
 * LightToTone.cpp
 *
 * Plays a pitch based on the light intensity.
 * As long as light intensity is below a threshold a random melody is played.
 * If a TEMT6000 module is attached, this value takes precedence over the LDR value.
 *
 * More RTTTL songs can be found under http://www.picaxe.com/RTTTL-Ringtones-for-Tune-Command/
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

#define USE_BUTTON_0
#include "EasyButtonAtInt01.h"

EasyButton Button0AtPin2(true);
//#define DEBUG

#define VERSION_EXAMPLE "1.1"

const int TONE_PIN = 11;

const int LDR_PIN = A4; // LDR connected to Ground and 4700 Ohm connected to VCC (depends on the LDR type).
#define TEMT_6000_DARK_VALUE 10

const int TEMT_6000_PIN = A3;
#define TEMT_6000_DARK_VALUE 10
bool isTEMTConnected = false;

#define LIGHT_THRESHOLD 14 // If the TEMT_6000 reading is below this value, a random melody is played

static int sMinimum = 1024, sMaximum;
static int sLDRMaximum = 0; // only for LDR

// Forward declarations
void maintainMinAndMax(int aLightValue);
int readLDRValue();
int readLightValue();

void setup() {
    Serial.begin(115200);
    while (!Serial)
        ; //delay for Leonardo
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));

    pinMode(LDR_PIN, INPUT);

    /*
     * Check if TEMT_6000 is connected - 10kOhm to GND
     * do not cover the sensor otherwise the detection will fail
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
    } else {
        Serial.println("Assume no TEMT6000 connected");
    }
    Serial.println();

    /*
     * Initialize maximum and minimum values
     */
    int tLightValue = readLightValue();
    sLDRMaximum += LIGHT_THRESHOLD + 2;
    randomSeed(analogRead(LDR_PIN));
    Serial.println();
    delay(500);
}

void loop() {

    int tLightValue = readLightValue();

    if (Button0AtPin2.ButtonToggleState) {

        /*
         * Play pentatonic notes
         */
        uint8_t tIndex = map(tLightValue, sMinimum, sMaximum, 0, ARRAY_SIZE_NOTE_C5_TO_C7_PENTATONIC - 1);
#ifdef DEBUG
        Serial.print("Index=");
        Serial.println(tIndex);
#endif
        uint16_t tFrequency = NoteC5ToC7Pentatonic[tIndex];
        tone(TONE_PIN, tFrequency);
        delay(200); // add an additional delay to make is easier to play a melody
    } else {
        if (tLightValue < LIGHT_THRESHOLD) {

            /*
             * Play random melody
             * More RTTTL songs can be found under http://www.picaxe.com/RTTTL-Ringtones-for-Tune-Command/
             *
             */
            startPlayRandomRtttlFromArrayPGM(TONE_PIN, RTTTLMelodies, ARRAY_SIZE_MELODIES);
            int tThresholdCount = 0;
            while (updatePlayRtttl()) {
                delay(10);
                /*
                 * Read new light value to decide if intensity is still low
                 */
                tLightValue = readLightValue();

                if (tLightValue > (LIGHT_THRESHOLD * 4)) {
                    // wait for 10 consecutive times of intensity above threshold to avoid spikes
                    tThresholdCount++;
                    if (tThresholdCount > 10) {
                        // stop playing melody
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
            tone(TONE_PIN, tLightValue * 4);
            delay(50);
        }
    }
}

void maintainMinAndMax(int aLightValue) {
    if (aLightValue < sMinimum || aLightValue > sMaximum) {
        if (aLightValue < sMinimum) {
            sMinimum = aLightValue;
        }
        if (aLightValue > sMaximum) {
            sMaximum = aLightValue;
        }
        Serial.print("Minimum=");
        Serial.print(sMinimum);
        Serial.print(" Maximum=");
        Serial.println(sMaximum);
    }
}

int readLDRValue() {

    /*
     * Read LDR value and maintain maximum and minimum values for it.
     */
    int tLightValue = analogRead(LDR_PIN);
#ifdef DEBUG
    Serial.print("LDR raw=");
    Serial.print(tLightValue);
#endif
    if (tLightValue > sLDRMaximum) {
        sLDRMaximum = tLightValue;
    }

    /*
     * Convert LDR value to be comparable to the TEMT_6000
     */
    tLightValue = (sLDRMaximum - tLightValue) + TEMT_6000_DARK_VALUE;
#ifdef DEBUG
    Serial.print(" converted=");
    Serial.print(tLightValue);
#endif
    return tLightValue;
}

int readLightValue() {
    int tLightValue;

    /*
     * Read TEMT_6000
     */
    if (isTEMTConnected) {
        tLightValue = analogRead(TEMT_6000_PIN);
#ifdef DEBUG
        Serial.print("TEMT=");
        Serial.print(tLightValue);
        Serial.print(" - ");
        // just for serial output
        readLDRValue();
#endif
    } else {
        tLightValue = readLDRValue();
    }
#ifdef DEBUG
    Serial.println();
#endif

    maintainMinAndMax(tLightValue);
    return tLightValue;
}

