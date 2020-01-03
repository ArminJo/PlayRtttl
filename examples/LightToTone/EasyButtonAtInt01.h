/*
 * EasyButtonAtInt01.h
 *
 *  Arduino library for handling push buttons connected between ground and INT0 and / or INT1 pin.
 *  INT0 and INT1 are connected to Pin 2 / 3 on most Arduinos (ATmega328), to PB6 / PA3 on ATtiny167 and on ATtinyX5 we have only INT0 at PB2.
 *  The library is totally based on interrupt.
 *  Debouncing is implemented in a not blocking way! It is merely done by ignoring a button change within the debouncing time.
 *  So button state is instantly available without debouncing delay!
 *
 *  Usage:
 *  #define USE_BUTTON_0
 *  #include "EasyButtonAtInt01.h"
 *  EasyButton Button0AtPin2(true);
 *
 *  Copyright (C) 2018  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of EasyButtonAtInt01 https://github.com/ArminJo/EasyButtonAtInt01.
 *
 *  EasyButtonAtInt01 is free software: you can redistribute it and/or modify
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
 */

/*
 * Version 1.1.0 - 1/2020
 * - Ported to ATtinyX5 and ATiny167.
 * - Support PinChangeInterrupt for ATtinies.
 * - Long press detection support.
 */

#ifndef EASY_BUTTON_AT_INT01_H_
#define EASY_BUTTON_AT_INT01_H_

#include <Arduino.h>

/*
 * Usage:
 * #define USE_BUTTON_0  // Enable code for button at INT0
 * #define USE_BUTTON_1  // Enable code for button at INT1 (PCINT0 for ATtinyX5)
 * #include "EasyButtonAtInt01.h"
 * EasyButton Button0AtPin2(true);  // true  -> Button is connected to INT0
 * EasyButton Button0AtPin3(false, &Button3CallbackHandler); // false -> button is not connected to INT0 => connected to INT1
 * ...
 * digitalWrite(LED_BUILTIN, Button0AtPin2.ButtonToggleState);
 * ...
 *
 */
#define EASY_BUTTON_LONG_PRESS_STILL_POSSIBLE 0
#define EASY_BUTTON_LONG_PRESS_ABORT 1
#define EASY_BUTTON_LONG_PRESS_DETECTED 2

/*
 * You can define your own value if you have buttons which are worse or better than the one I have.
 * Since debouncing is not done with blocking wait, reducing this value makes not much sense.
 * Test your value with just commenting out the line #define LED_FEEDBACK_FOR_DEBOUNCE_TEST below.
 * No other action other than defining the button is needed for this test. Define it e.g. with EasyButton Button0AtPin2(true);
 * Then press the button many times and see if the LED shows the state of the button for every press.
 * In case of the debouncing period too small, the led will sometimes stay active if the button was released.
 */
#ifndef BUTTON_DEBOUNCING_MILLIS
#define BUTTON_DEBOUNCING_MILLIS 50 // 35 millis measured for my button :-).
#endif

/*
 * Use the arduino function attachInterrupt().
 * Recommended if you get the error " multiple definition of `__vector_1'" (or `__vector_2')
 * because another library uses the attachInterrupt() function.
 * For one button it needs additional 160 bytes FLASH, for 2 buttons still 88 bytes.
 */
//#define USE_ATTACH_INTERRUPT
//
/*
 * Activate LED_BUILTIN as long as button is pressed
 * Serves as easy test to see if the debounce of 50 milliseconds is working with the button attached
 */
//#define LED_FEEDBACK_FOR_DEBOUNCE_TEST
#if defined (LED_FEEDBACK_FOR_DEBOUNCE_TEST)
#  if ! defined(BUTTON_TEST_FEEDBACK_LED_PIN)
#    if defined (LED_BUILTIN)
#    define BUTTON_TEST_FEEDBACK_LED_PIN LED_BUILTIN  // if not specified, use built in led - pin 13 on Uno board
#    else
#    error "LED_FEEDBACK_FOR_DEBOUNCE_TEST defined but no BUTTON_TEST_FEEDBACK_LED_PIN or LED_BUILTIN defined"
#    endif
#  endif
#endif

//
//#define MEASURE_INTERRUPT_TIMING
#if defined(MEASURE_INTERRUPT_TIMING) || defined (LED_FEEDBACK_FOR_DEBOUNCE_TEST)
#include "digitalWriteFast.h"
#endif

#if defined(MEASURE_INTERRUPT_TIMING)
#ifndef BUTTON_TEST_TIMING_PIN
#define BUTTON_TEST_TIMING_PIN 6  // use pin 6
//#define BUTTON_TEST_TIMING_PIN 12  // use pin 12
#endif
#endif

//#define TRACE

/*
 * These defines are here to enable saving of 150 bytes FLASH if only one button is needed
 */
//#define USE_BUTTON_0
//#define USE_BUTTON_1
#if not (defined(USE_BUTTON_0) || defined(USE_BUTTON_1))
#error USE_BUTTON_0 and USE_BUTTON_1 are not defined, please define them or remove the #include "EasyButtonAtInt01.h"
#endif

/*
 * Pin and port definitions for Arduinos
 */
#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#define INT0_PIN (2)
#define INT0_DDR_PORT (DDRB)
#define INT0_IN_PORT  (PINB)
#define INT0_OUT_PORT (PORTB)

#if ! defined(INT1_PIN)
#define INT1_PIN (3)
#elif (INT1_PIN != 2) && (INT1_PIN > 5)
#error "INT1_PIN (for PCINT0 interrupt) can only be 0,1,3,4,5"
#endif
#define INT1_DDR_PORT (DDRB)
#define INT1_IN_PORT  (PINB)
#define INT1_OUT_PORT (PORTB)
#elif defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#define INT0_PIN (14)
#define INT0_DDR_PORT (DDRB)
#define INT0_IN_PORT  (PINB)
#define INT0_OUT_PORT (PORTB)

#if ! defined(INT1_PIN)
#define INT1_PIN (3)
#elif (INT1_PIN != 3) && (INT1_PIN > 7)
#error "INT1_PIN (for PCINT0 interrupt) can only be 0 to 2 or 4 to 7"
#endif
#define INT1_DDR_PORT (DDRA)
#define INT1_IN_PORT  (PINA)
#define INT1_OUT_PORT (PORTA)
#else
#define INT0_PIN (2)
#define INT0_DDR_PORT (DDRD)
#define INT0_IN_PORT  (PIND)
#define INT0_OUT_PORT (PORTD)

#if ! defined(INT1_PIN)
#define INT1_PIN (3)
#endif
#define INT1_DDR_PORT (DDRD)
#define INT1_IN_PORT  (PIND)
#define INT1_OUT_PORT (PORTD)
#endif

#define INT1_BIT INT1_PIN

#if defined(USE_BUTTON_1) && ((! defined(ISC10)) || ((defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)) && INT1_PIN != 3)) && ! defined(INTENTIONALLY_USE_PCINT0_FOR_BUTTON1)
#warning "Using PCINT0 interrupt for button 1"
#endif

#if (INT0_PIN >= 8)
#define INT0_BIT (INT0_PIN - 8)
#else
#define INT0_BIT INT0_PIN
#endif

void handleINT0Interrupt();
void handleINT1Interrupt();

class EasyButton {

public:

#if defined(USE_BUTTON_0)
    static EasyButton * sPointerToButton0ForISR;
#endif
#if defined(USE_BUTTON_1)
    static EasyButton * sPointerToButton1ForISR;
#endif

    volatile bool ButtonStateIsActive;          // negative logic: true / active means button pin is LOW
    volatile bool ButtonToggleState;            // Toggle is on press, not on release - initial value is false
    volatile bool ButtonStateHasJustChanged;    // Flag to enable action only once. Only set to true by library
                                                // Can be checked and set to false my main program to enable only one action per button press
    /*
     * Duration is set at button release or from an outside loop which polls the button state in order to check for button press timeouts,
     * since we get no interrupt until button is released.
     */
    volatile uint16_t ButtonPressDurationMillis; // Duration of active state.
    volatile long ButtonLastChangeMillis;       // for debouncing
    volatile long ButtonReleaseMillis;          // for double press recognition

    volatile bool isButtonAtINT0;
    void (*ButtonPressCallback)(bool aButtonToggleState) = NULL; // if not null, is called on every button press with ButtonToggleState as parameter

    EasyButton(bool aIsButtonAtINT0) {
        isButtonAtINT0 = aIsButtonAtINT0;
        init();
    }

    EasyButton(bool aIsButtonAtINT0, void (*aButtonPressCallback)(bool aButtonToggleState)) {
        isButtonAtINT0 = aIsButtonAtINT0;
        ButtonPressCallback = aButtonPressCallback;
        init();
    }

#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#define EICRA  MCUCR
#define EIFR   GIFR
#define EIMSK  GIMSK
#endif
    /*
     * Sets pin 2 mode to INPUT_PULLUP and enables INT0 Interrupt on any logical change.
     */
    void init() {
#if defined(MEASURE_INTERRUPT_TIMING)
        pinModeFast(BUTTON_TEST_TIMING_PIN, OUTPUT);
#endif

#if defined(LED_FEEDBACK_FOR_DEBOUNCE_TEST)
        pinModeFast(BUTTON_TEST_FEEDBACK_LED_PIN, OUTPUT);
#endif
#if defined(USE_BUTTON_0) && not defined(USE_BUTTON_1)
        INT0_DDR_PORT &= ~(_BV(INT0_BIT)); // pinModeFast(2, INPUT_PULLUP);
        INT0_OUT_PORT |= _BV(INT0_BIT);
        sPointerToButton0ForISR = this;
#  if defined(USE_ATTACH_INTERRUPT)
        attachInterrupt(digitalPinToInterrupt(INT0_PIN), &handleINT0Interrupt, CHANGE);
#  else
        EICRA |= (1 << ISC00);  // interrupt on any logical change
        EIFR |= 1 << INTF0;     // clear interrupt bit
        EIMSK |= 1 << INT0;     // enable interrupt on next change
#  endif //USE_ATTACH_INTERRUPT

#elif defined(USE_BUTTON_1) && not defined(USE_BUTTON_0)
        INT1_DDR_PORT &= ~(_BV(INT1_BIT));
        INT1_OUT_PORT |= _BV(INT1_BIT);
        sPointerToButton1ForISR = this;

#  if (! defined(ISC10)) || ((defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)) && INT1_PIN != 3)
#    if defined(PCICR)
        PCICR |= 1 << PCIE0; //PCINT0 enable
        PCMSK0 = digitalPinToBitMask(INT1_PIN);
#    else
        GIMSK |= 1 << PCIE; //PCINT enable
        PCMSK = digitalPinToBitMask(INT1_PIN);
#    endif
#  else
#    if defined(USE_ATTACH_INTERRUPT)
        attachInterrupt(digitalPinToInterrupt(INT1_PIN), &handleINT1Interrupt, CHANGE);
#    else
        EICRA |= (1 << ISC10);  // interrupt on any logical change
        EIFR |= 1 << INTF1;     // clear interrupt bit
        EIMSK |= 1 << INT1;     // enable interrupt on next change
#    endif //USE_ATTACH_INTERRUPT
#  endif // ! defined(ISC10)

#elif defined(USE_BUTTON_0) && defined(USE_BUTTON_1)
        if (isButtonAtINT0) {
            INT0_DDR_PORT &= ~(_BV(INT0_BIT)); // pinModeFast(2, INPUT_PULLUP);
            INT0_OUT_PORT |= _BV(INT0_BIT);
            sPointerToButton0ForISR = this;
#  if defined(USE_ATTACH_INTERRUPT)
            attachInterrupt(digitalPinToInterrupt(INT0_PIN), &handleINT0Interrupt, CHANGE);
#  else
            EICRA |= (1 << ISC00);  // interrupt on any logical change
            EIFR |= 1 << INTF0;     // clear interrupt bit
            EIMSK |= 1 << INT0;     // enable interrupt on next change
#  endif //USE_ATTACH_INTERRUPT
        } else {
            INT1_DDR_PORT &= ~(_BV(INT1_BIT));
            INT1_OUT_PORT |= _BV(INT1_BIT);
            sPointerToButton1ForISR = this;

#  if (! defined(ISC10)) || ((defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)) && INT1_PIN != 3)
#    if defined(PCICR)
            PCICR |= 1 << PCIE0; //PCINT0 enable
            PCMSK0 = digitalPinToBitMask(INT1_PIN);
#    else
            GIMSK |= 1 << PCIE; //PCINT enable
            PCMSK = digitalPinToBitMask(INT1_PIN);
#    endif
#  else
#    if defined(USE_ATTACH_INTERRUPT)
            attachInterrupt(digitalPinToInterrupt(INT1_PIN), &handleINT1Interrupt, CHANGE);
#    else
            EICRA |= (1 << ISC10);  // interrupt on any logical change
            EIFR |= 1 << INTF1;     // clear interrupt bit
            EIMSK |= 1 << INT1;     // enable interrupt on next change
#    endif //USE_ATTACH_INTERRUPT
#  endif // ! defined(ISC10)
        }
#endif
        ButtonStateIsActive = false; // negative logic for ButtonStateIsActive! true means button pin is LOW
        ButtonToggleState = false;
    }

    /*
     * Negative logic for readButtonState() true means button pin is LOW
     */
    bool readButtonState() {
#if defined(USE_BUTTON_0) && not defined(USE_BUTTON_1)
        return !(INT0_IN_PORT & _BV(INT0_BIT));  //  = digitalReadFast(2);
#elif defined(USE_BUTTON_1) && not defined(USE_BUTTON_0)
        return !(INT1_IN_PORT & _BV(INT1_BIT));  //  = digitalReadFast(3);
#elif defined(USE_BUTTON_0) && defined(USE_BUTTON_1)
        if (isButtonAtINT0) {
            return !(INT0_IN_PORT & _BV(INT0_BIT));  //  = digitalReadFast(2);
        } else {
            return !(INT1_IN_PORT & _BV(INT1_BIT));  //  = digitalReadFast(3);
        }
#endif
    }

    /*
     * Update button state if state change was not captured by the ISR
     * @return true if state was changed and updated
     */
    bool updateButtonState() {
        if (readButtonState() != ButtonStateIsActive) {
            handleINT01Interrupts();
#ifdef TRACE
            Serial.println(F("Updated button state, current state was not caught by ISR"));
#endif
            return true;
        }
        return false;
    }

    /*
     * Updates the ButtonPressDurationMillis by polling, since this cannot be done by interrupt.
     */
    uint16_t updateButtonPressDuration() {
        if (readButtonState()) {
            // Button still active -> update ButtonPressDurationMillis
            ButtonPressDurationMillis = millis() - ButtonLastChangeMillis;
        }
        return ButtonPressDurationMillis;
    }

    /*
     * Used for long button press recognition.
     */
    uint8_t checkForLongPress(uint16_t aLongPressThresholdMillis) {
        if (readButtonState()) {
            if (millis() - ButtonLastChangeMillis > aLongPressThresholdMillis) {
                // long press detected
                ButtonPressDurationMillis = millis() - ButtonLastChangeMillis;
                return EASY_BUTTON_LONG_PRESS_DETECTED;
            }
            return EASY_BUTTON_LONG_PRESS_STILL_POSSIBLE;
        }
        return EASY_BUTTON_LONG_PRESS_ABORT;
    }

    /*
     * Checks blocking for long press of button
     * @return true if long press was detected
     */
    bool checkForLongPressBlocking(uint16_t aLongPressThresholdMillis) {
        uint8_t tLongPressCheckResult;
        do {
            tLongPressCheckResult = checkForLongPress(aLongPressThresholdMillis);
            delay(1);
        } while (!tLongPressCheckResult);
        return (tLongPressCheckResult == EASY_BUTTON_LONG_PRESS_DETECTED);
    }

    /*
     * 1. Read button pin level and invert logic level since we have negative logic because of using pullups.
     * 2. Check for bouncing - state change during debounce period. we need millis() to be enabled to run in the background.
     * 3. Check for spikes - interrupts but no level change.
     * 4. Process valid button state change. If Callback requested, call callback routine, get button pin level again and handle release.
     */
    void handleINT01Interrupts() {
        // Read button value
        bool tCurrentButtonStateIsActive;
        /*
         * This is faster than readButtonState();
         */
#if defined(USE_BUTTON_0) && not defined(USE_BUTTON_1)
        tCurrentButtonStateIsActive = INT0_IN_PORT & _BV(INT0_BIT);  //  = digitalReadFast(2);
    #elif defined(USE_BUTTON_1) && not defined(USE_BUTTON_0)
        tCurrentButtonStateIsActive = INT1_IN_PORT & _BV(INT1_BIT);  //  = digitalReadFast(3);
#elif defined(USE_BUTTON_0) && defined(USE_BUTTON_1)
        if (isButtonAtINT0) {
            tCurrentButtonStateIsActive = INT0_IN_PORT & _BV(INT0_BIT);  //  = digitalReadFast(2);
        } else {
            tCurrentButtonStateIsActive = INT1_IN_PORT & _BV(INT1_BIT);  //  = digitalReadFast(3);
        }
    #endif

        tCurrentButtonStateIsActive = !tCurrentButtonStateIsActive; // negative logic for ButtonStateIsActive! true means button pin is LOW
#ifdef TRACE
        Serial.println(tCurrentButtonStateIsActive);
#endif
        long tMillis = millis();
        long tDeltaMillis = tMillis - ButtonLastChangeMillis;
        // Check for bouncing - state change during debounce period
        if (tDeltaMillis <= BUTTON_DEBOUNCING_MILLIS) {
#ifdef TRACE
            Serial.println(F("Button bouncing"));
            //        Serial.print(F("ms="));
            //        Serial.print(tMillis);
            //        Serial.print(F(" D="));
            //        Serial.println(tDeltaMillis);
#endif
            /*
             * Button is bouncing, signal is ringing - do nothing, ignore and wait for next interrupt
             */
        } else {
            /*
             * Here we are after debouncing period
             */
            if (tCurrentButtonStateIsActive == ButtonStateIsActive) {
#ifdef TRACE
                Serial.print(F("Spike detected. State="));
                Serial.println(tCurrentButtonStateIsActive);
#endif
                /*
                 * tCurrentButtonStateIsActive == OldButtonStateIsActive. We had an interrupt, but nothing seems to have changed -> spike
                 * Do nothing, ignore and wait for next interrupt
                 */
            } else {
                /*
                 * Valid change detected
                 */
                ButtonLastChangeMillis = tMillis;
                ButtonPressDurationMillis = tDeltaMillis;
#ifdef TRACE
                Serial.print(F("Change detected. State="));
                Serial.println(tCurrentButtonStateIsActive);
#endif
                ButtonStateIsActive = tCurrentButtonStateIsActive;
                ButtonStateHasJustChanged = true;
                if (tCurrentButtonStateIsActive) {
                    /*
                     * Action on button press, no action on release
                     */
#ifdef LED_FEEDBACK_FOR_DEBOUNCE_TEST
                    digitalWriteFast(BUTTON_TEST_FEEDBACK_LED_PIN, HIGH);
#endif
                    ButtonToggleState = !ButtonToggleState;
                    if (ButtonPressCallback != NULL) {
                        /*
                         * Call callback function.
                         * sei() is needed if callback function needs more time to allow millis() to proceed.
                         * Otherwise we may see bouncing instead of button release and then spike instead of button press
                         */
                        sei();
                        ButtonPressCallback(ButtonToggleState);
                        /*
                         * Check button again since it may changed back while processing callback function
                         */
                        tCurrentButtonStateIsActive = readButtonState();
                        if (ButtonStateIsActive != tCurrentButtonStateIsActive) {
                            // button released now, so maintain status
#ifdef TRACE
                            Serial.println(F("Button release during callback processing detected."));
#endif
                            ButtonStateIsActive = tCurrentButtonStateIsActive;
                            ButtonStateHasJustChanged = true;
                        }
                    }
                } else {
                    /*
                     * Button release
                     */
                    ButtonReleaseMillis = tMillis;
#ifdef LED_FEEDBACK_FOR_DEBOUNCE_TEST
                    digitalWriteFast(BUTTON_TEST_FEEDBACK_LED_PIN, LOW);
#endif
                }
            }
        }
    }
};

#if defined(USE_BUTTON_0)
EasyButton * EasyButton::sPointerToButton0ForISR;
#endif
#if defined(USE_BUTTON_1)
EasyButton * EasyButton::sPointerToButton1ForISR;
#endif

/*
 * This functions are weak and can be replaced by your own code
 */
#if defined(USE_BUTTON_0)
void __attribute__ ((weak)) handleINT0Interrupt() {
    EasyButton::sPointerToButton0ForISR->handleINT01Interrupts();
}
#endif

#if defined(USE_BUTTON_1)
void __attribute__ ((weak)) handleINT1Interrupt() {
    EasyButton::sPointerToButton1ForISR->handleINT01Interrupts();
}
#endif

#if not defined(USE_ATTACH_INTERRUPT)
// ISR for PIN PD2
// Cannot make the vector itself weak, since the vector table is already filled by weak vectors resulting in ignoring my weak one:-(
//ISR(INT0_vect, __attribute__ ((weak))) {
#  if defined(USE_BUTTON_0)
ISR(INT0_vect) {
#    ifdef MEASURE_INTERRUPT_TIMING
    digitalWriteFast(BUTTON_TEST_TIMING_PIN, HIGH);
#    endif
    handleINT0Interrupt();
#    ifdef MEASURE_INTERRUPT_TIMING
    digitalWriteFast(BUTTON_TEST_TIMING_PIN, LOW);
#    endif
}
#  endif

#  if defined(USE_BUTTON_1)
#    if (! defined(ISC10)) || ((defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)) && INT1_PIN != 3)
// on ATtinyX5 we do not have a INT1_vect but we can use the PCINT0_vect
ISR(PCINT0_vect)
#    else
ISR(INT1_vect)
#    endif
{
#    ifdef MEASURE_INTERRUPT_TIMING
    digitalWriteFast(BUTTON_TEST_TIMING_PIN, HIGH);
#    endif
    handleINT1Interrupt();
#    ifdef MEASURE_INTERRUPT_TIMING
    digitalWriteFast(BUTTON_TEST_TIMING_PIN, LOW);
#    endif
}
#  endif
#endif // not defined(USE_ATTACH_INTERRUPT)

#endif /* EASY_BUTTON_AT_INT01_H_ */

#pragma once
