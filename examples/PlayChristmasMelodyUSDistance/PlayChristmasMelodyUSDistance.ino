/*
 * PlayChristmasMelodyUSDistance.cpp
 *
 * Plays a random Christmas melody if US Sensor value is a defined range.
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

#include "HCSR04.h"
#include "BlinkLed.h"
#include <PlayRtttl.h>

#define VERSION_EXAMPLE "1.0"

#define PIN_SPEAKER         3

#define PIN_TRIGGER_OUT     4
#define PIN_ECHO_IN         5

#define PIN_GREEN_LED       6
#define PIN_YELLOW_LED      7
#define PIN_RED_LED         8

BlinkLed RedLed(PIN_RED_LED);
BlinkLed YellowLed(PIN_YELLOW_LED);
BlinkLed GreenLed(PIN_GREEN_LED);

void playRandomSongAndBlink();

//The setup function is called once at startup of the sketch
void setup() {
    // Start serial output
    Serial.begin(115200);
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));

    initUSDistancePins(PIN_TRIGGER_OUT, PIN_ECHO_IN);

    randomSeed(getUSDistance());
    /*
     * Play first song
     */
    playRandomSongAndBlink();
    YellowLed.off(); // switch it manually off here
}

void loop() {
    unsigned int tCentimeter = getUSDistanceAsCentiMeter();
    Serial.print("Distance=");
    Serial.println(tCentimeter);
    delay(100);
    if (tCentimeter > 40 && tCentimeter < 120) {
        playRandomSongAndBlink();

        // wait for distance to be out of range for 4 consecutive readings
        uint8_t tCounter = 0;
        tCentimeter = getUSDistanceAsCentiMeter();
        while (tCounter < 4) {
            Serial.print("Distance=");
            Serial.print(tCentimeter);
            if (tCentimeter < 40 || tCentimeter > 120) {
                tCounter++;
                Serial.print(" counter=");
                Serial.print(tCounter);
                Serial.println();
            } else {
                tCounter = 0; // reset to start condition
                Serial.print(" still in range. Wait for ");
                Serial.print(4 - tCounter);
                Serial.println(" distances out of range.");
            }
            YellowLed.update();
            tCentimeter = getUSDistanceAsCentiMeter();
            delay(200);
        }
    }
    YellowLed.off();
}

/*
 * Leaves yellow led blinking
 */
void playRandomSongAndBlink() {
    char StringBuffer[16];
    Serial.println();
    Serial.print("Now playing: ");
    startPlayRandomRtttlFromArrayPGM(PIN_SPEAKER, RTTTLChristmasMelodies, ARRAY_SIZE_CHRISTMAS_SONGS, StringBuffer,
            sizeof(StringBuffer));
    Serial.println(StringBuffer);

    /*
     * Start LEDs blinking
     */
    RedLed.startWithOnOffTime(300, 600);
    YellowLed.startWithOnOffTime(800, 400);
    GreenLed.startWithOnOffTime(1000, 1500);

// wait for the song to end
    while (updatePlayRtttl()) {
        RedLed.update();
        YellowLed.update();
        GreenLed.update();
        delay(1);
    }
// switch off only 2 leds, the third will blink until the "thing in the right distance" is gone
    RedLed.off();
    GreenLed.off();
    delay(2000);
}
