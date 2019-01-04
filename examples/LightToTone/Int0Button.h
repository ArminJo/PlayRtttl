/*
 * Int0Button.h
 *
 *  Created on: 23.11.2018
 *      Author: Armin
 */

#include <Arduino.h>

#ifdef MEASURE_TIMING // needs pin 12
#include "digitalWriteFast.h"
#endif

#ifndef INT0BUTTON_H_
#define INT0BUTTON_H_

#ifndef BUTTON_0_DEBOUNCING_MILLIS
#define BUTTON_0_DEBOUNCING_MILLIS 50 // only needed for button release. 35 millis measured.
#endif

volatile bool sButton0State; // negative logic: true means button pin is LOW
volatile bool sButton0StateHasChanged;
volatile bool sButton0ToggleState = false; // toggle on press, not on release
void (*sButtonPressCallback)(bool aButton0ToggleState) = NULL; // if not null, is called on every button press with sButton0ToggleState as parameter
volatile long sButton0LastChangeMillis; // for debouncing

/*
 * Sets pin 2 mode to INPUT_PULLUP and enables INT0 Interrupt on any logical change.
 */
void initINT0InterruptForButton0AtPin2() {
#ifdef MEASURE_TIMING
    pinModeFast(12, OUTPUT);
#endif

    pinMode(2, INPUT_PULLUP);

// interrupt on any logical change
    EICRA |= (1 << ISC00);
// clear interrupt bit
    EIFR |= 1 << INTF0;
// enable interrupt on next change
    EIMSK |= 1 << INT0;
    sButton0State = false; // negative logic for sButton0State! true means button pin is LOW
}

void initINT0InterruptForButton0AtPin2(void (*aButtonPressCallback)(bool aButton0ToggleState)) {
    sButtonPressCallback = aButtonPressCallback;
    initINT0InterruptForButton0AtPin2();
}

volatile uint8_t sBouncings;
// ISR for PIN PD2
ISR(INT0_vect) {
    uint8_t tActualButtonState = PIND & _BV(2); //    uint8_t tState = digitalReadFast(2);
    tActualButtonState = !tActualButtonState; // negative logic for sButton0State! true means button pin is LOW
#ifdef MEASURE_TIMING
            digitalWriteFast(12, HIGH);
#endif
    long tMillis = millis();
    long tDeltaMillis = tMillis - sButton0LastChangeMillis;
    if (tDeltaMillis <= BUTTON_0_DEBOUNCING_MILLIS) {
#ifdef TRACE
        Serial.println("Button signal ringing");
#endif
        // signal is ringing
    } else {
        sButton0LastChangeMillis = tMillis;
        if (tActualButtonState == sButton0State) {
#ifdef TRACE
            Serial.println("Spike detected");
#endif
            /*
             * ActualButtonState == OldButtonState. We had an interrupt, but nothing seems to have changed -> spike?
             * Disable debouncing for next transition, which will end up here, since it is the spike ending.
             * But this again disables debouncing, so for the over next transition we have a fresh start.
             */
            sButton0LastChangeMillis = 0;
        } else {
            // change detected
#ifdef TRACE
            Serial.print("Change detected. State=");
            Serial.println(tActualButtonState);
#endif
            sButton0State = tActualButtonState;
            sButton0StateHasChanged = true;
            if (tActualButtonState) {
                sButton0ToggleState = !sButton0ToggleState;
                if (sButtonPressCallback != NULL) {
                    // call callback function
                    sButtonPressCallback(sButton0ToggleState);
                    // check button again since it may change back while processing callback function
                    tActualButtonState = PIND & _BV(2);
                    tActualButtonState = !tActualButtonState;
                    if (sButton0State != tActualButtonState) {
                        // button changed back
#ifdef TRACE
                        Serial.println("Change back during callback processing detected.");
#endif
                        sButton0State = tActualButtonState;
                        sButton0StateHasChanged = true;
                    }
                }
            }
        }
    }
#ifdef MEASURE_TIMING
    digitalWriteFast(12, LOW);
#endif
}

#endif /* INT0BUTTON_H_ */
