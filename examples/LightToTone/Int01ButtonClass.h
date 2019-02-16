/*
 * Int01ButtonClass.h
 *
 *  Created on: 23.11.2018
 *      Author: Armin
 */

#ifndef INT01BUTTONCLASS_H_
#define INT01BUTTONCLASS_H_

#include <Arduino.h>

//#define MEASURE_TIMING
//#define LED_FEEDBACK_FOR_DEBOUNCE_TEST
//#define TRACE

/*
 * These defines are here to enable saving of 150 bytes FLASH if only one button is needed
 */
//#define USE_BUTTON_0
//#define USE_BUTTON_1
#if not (defined(USE_BUTTON_0) || defined(USE_BUTTON_1))
#error USE_BUTTON_0 and USE_BUTTON_1 are not defined, please define them or remove the #include "Int01Buttons.h"
#endif

#if defined(MEASURE_TIMING) || defined (LED_FEEDBACK_FOR_DEBOUNCE_TEST)
#include "digitalWriteFast.h"
#endif

#if defined(MEASURE_TIMING)
#ifndef BUTTON_TEST_TIMING_PIN
#define BUTTON_TEST_TIMING_PIN 12  // use pin 12
#endif
#endif

#if defined (LED_FEEDBACK_FOR_DEBOUNCE_TEST)
#ifndef BUTTON_TEST_FEEDBACK_LED_PIN
#define BUTTON_TEST_FEEDBACK_LED_PIN LED_BUILDIN  // use pin 13
#endif
#endif



#ifndef BUTTON_DEBOUNCING_MILLIS
#define BUTTON_DEBOUNCING_MILLIS 50 // 35 millis measured for my button :-).
#endif

class Int01Button {

public:

    static Int01Button * sPointerToButton0ForISR;
    static Int01Button * sPointerToButton1ForISR;

    volatile bool ButtonStateIsActive;                  // negative logic: true / active means button pin is LOW
    volatile bool ButtonStateHasJustChanged;    // Flag to enable action only once
    /*
     * Duration is set at button release or from an outside loop which polls the button state in order to check for button press timeouts,
     * since we get no interrupt until button is released.
     */
    volatile uint16_t ButtonPressDurationMillis; // Duration of active state.
    volatile long ButtonLastChangeMillis;       // for debouncing
    volatile long ButtonReleaseMillis;          // for double press recognition

    volatile bool ButtonToggleState;    // toggle is on press, not on release
    volatile bool isButton0AtPin2;
    void (*ButtonPressCallback)(bool aButtonToggleState) = NULL; // if not null, is called on every button press with ButtonToggleState as parameter

    Int01Button(bool aIsButton0AtPin2) {
        isButton0AtPin2 = aIsButton0AtPin2;
        init();
    }

    Int01Button(bool aIsButton0AtPin2, void (*aButtonPressCallback)(bool aButtonToggleState)) {
        isButton0AtPin2 = aIsButton0AtPin2;
        ButtonPressCallback = aButtonPressCallback;
        init();
    }

    /*
     * Sets pin 2 mode to INPUT_PULLUP and enables INT0 Interrupt on any logical change.
     */
    void init() {
#if defined(MEASURE_TIMING)
        pinModeFast(BUTTON_TEST_TIMING_PIN, OUTPUT);
#endif

#if defined (LED_FEEDBACK_FOR_DEBOUNCE_TEST)
        pinModeFast(BUTTON_TEST_FEEDBACK_LED_PIN, OUTPUT);
#endif
#if defined(USE_BUTTON_0) && not defined(USE_BUTTON_1)
        DDRD &= ~(_BV(2)); // pinModeFast(2, INPUT_PULLUP);
        PORTD |= _BV(2);
        // interrupt on any logical change
        EICRA |= (1 << ISC00);
        // clear interrupt bit
        EIFR |= 1 << INTF0;
        // enable interrupt on next change
        EIMSK |= 1 << INT0;
        sPointerToButton0ForISR = this;
#endif
#if defined(USE_BUTTON_1) && not defined(USE_BUTTON_0)
        DDRD &= ~(_BV(3));
        PORTD |= _BV(3);
        // interrupt on any logical change
        EICRA |= (1 << ISC10);
        // clear interrupt bit
        EIFR |= 1 << INTF1;
        // enable interrupt on next change
        EIMSK |= 1 << INT1;
        sPointerToButton1ForISR = this;
#endif

#if defined(USE_BUTTON_0) && defined(USE_BUTTON_1)
        if (isButton0AtPin2) {
            DDRD &= ~(_BV(2)); // pinModeFast(2, INPUT_PULLUP);
            PORTD |= _BV(2);
            // interrupt on any logical change
            EICRA |= (1 << ISC00);
            // clear interrupt bit
            EIFR |= 1 << INTF0;
            // enable interrupt on next change
            EIMSK |= 1 << INT0;
            sPointerToButton0ForISR = this;
        } else {
            DDRD &= ~(_BV(3));
            PORTD |= _BV(3);
            // interrupt on any logical change
            EICRA |= (1 << ISC10);
            // clear interrupt bit
            EIFR |= 1 << INTF1;
            // enable interrupt on next change
            EIMSK |= 1 << INT1;
            sPointerToButton1ForISR = this;

        }
#endif
        ButtonStateIsActive = false; // negative logic for ButtonStateIsActive! true means button pin is LOW
        ButtonToggleState = false;
    }

    /*
     * Used for long button press recognition.
     * Updates the ButtonPressDurationMillis by polling, since this cannot be done by interrupt.
     */
    uint16_t updateButtonPressDuration() {
        uint8_t tActualButtonStateIsActive;
#if defined(USE_BUTTON_0) && not defined(USE_BUTTON_1)
        tActualButtonStateIsActive = PIND & _BV(2);  //  = digitalReadFast(2);
#endif

#if defined(USE_BUTTON_1) && not defined(USE_BUTTON_0)
        tActualButtonStateIsActive = PIND & _BV(3);  //  = digitalReadFast(3);
#endif

#if defined(USE_BUTTON_0) && defined(USE_BUTTON_1)
        if (isButton0AtPin2) {
            tActualButtonStateIsActive = PIND & _BV(2);  //  = digitalReadFast(2);
        } else {
            tActualButtonStateIsActive = PIND & _BV(3);  //  = digitalReadFast(3);
        }
#endif
        if (tActualButtonStateIsActive) {
            // Button still active -> update ButtonPressDurationMillis
            ButtonPressDurationMillis = millis() - ButtonLastChangeMillis;
        }
        return ButtonPressDurationMillis;
    }
};

#if defined(USE_BUTTON_0)
Int01Button * Int01Button::sPointerToButton0ForISR;
#endif
#if defined(USE_BUTTON_1)
Int01Button * Int01Button::sPointerToButton1ForISR;
#endif

void handleINT01Interrupts(Int01Button * aButtonControlPtr) {
    // Read button value
    bool tActualButtonStateIsActive;

#if defined(USE_BUTTON_0) && not defined(USE_BUTTON_1)
    tActualButtonStateIsActive = PIND & _BV(2);  //  = digitalReadFast(2);
#endif

#if defined(USE_BUTTON_1) && not defined(USE_BUTTON_0)
    tActualButtonStateIsActive = PIND & _BV(3);  //  = digitalReadFast(3);
#endif

#if defined(USE_BUTTON_0) && defined(USE_BUTTON_1)
    if (aButtonControlPtr->isButton0AtPin2) {
        tActualButtonStateIsActive = PIND & _BV(2);  //  = digitalReadFast(2);
    } else {
        tActualButtonStateIsActive = PIND & _BV(3);  //  = digitalReadFast(3);
    }
#endif

    tActualButtonStateIsActive = !tActualButtonStateIsActive; // negative logic for ButtonStateIsActive! true means button pin is LOW

    long tMillis = millis();
    long tDeltaMillis = tMillis - aButtonControlPtr->ButtonLastChangeMillis;
    // Check for bouncing
    if (tDeltaMillis <= BUTTON_DEBOUNCING_MILLIS) {
#ifdef TRACE
        Serial.println("Button bouncing");
#endif
        /*
         * Button signal is ringing - do nothing, ignore and wait for next interrupt
         */
    } else {
        if (tActualButtonStateIsActive == aButtonControlPtr->ButtonStateIsActive) {
#ifdef TRACE
            Serial.println("Spike detected");
#endif
            /*
             * ActualButtonStateIsActive == OldButtonStateIsActive. We had an interrupt, but nothing seems to have changed -> spike
             * Do nothing, ignore and wait for next interrupt
             */
        } else {
            /*
             * Valid change detected
             */
            aButtonControlPtr->ButtonLastChangeMillis = tMillis;
            aButtonControlPtr->ButtonPressDurationMillis = tDeltaMillis;
#ifdef TRACE
            Serial.print("Change detected. State=");
            Serial.println(tActualButtonStateIsActive);
#endif
            aButtonControlPtr->ButtonStateIsActive = tActualButtonStateIsActive;
            aButtonControlPtr->ButtonStateHasJustChanged = true;
            if (tActualButtonStateIsActive) {
                /*
                 * Action on button press, no action on release
                 */
#ifdef LED_FEEDBACK_FOR_DEBOUNCE_TEST
                digitalWriteFast(BUTTON_TEST_FEEDBACK_LED_PIN, HIGH);
#endif
                aButtonControlPtr->ButtonToggleState = !aButtonControlPtr->ButtonToggleState;
                if (aButtonControlPtr->ButtonPressCallback != NULL) {
                    // call callback function
                    aButtonControlPtr->ButtonPressCallback(aButtonControlPtr->ButtonToggleState);
                    // check button again since it may change back while processing callback function
                    tActualButtonStateIsActive = PIND & _BV(2);
                    tActualButtonStateIsActive = !tActualButtonStateIsActive;
                    if (aButtonControlPtr->ButtonStateIsActive != tActualButtonStateIsActive) {
                        // button now released maintain status
#ifdef TRACE
                        Serial.println("Button release during callback processing detected.");
#endif
                        aButtonControlPtr->ButtonStateIsActive = tActualButtonStateIsActive;
                        aButtonControlPtr->ButtonStateHasJustChanged = true;
                    }
                }
            } else {
                /*
                 * Button release
                 */
                aButtonControlPtr->ButtonReleaseMillis = tMillis;
#ifdef LED_FEEDBACK_FOR_DEBOUNCE_TEST
                digitalWriteFast(BUTTON_TEST_FEEDBACK_LED_PIN, LOW);
#endif
            }
        }
    }

}

/*
 * This functions are weak and can be replaced by your own one
 */
#if defined(USE_BUTTON_0)
void __attribute__ ((weak)) handleINT0Interrupt() {
    handleINT01Interrupts(Int01Button::sPointerToButton0ForISR);
}
#endif

#if defined(USE_BUTTON_1)
void __attribute__ ((weak)) handleINT1Interrupt() {
    handleINT01Interrupts(Int01Button::sPointerToButton1ForISR);
}
#endif

/*
 * Handles spikes / very short pulses, i.e. when we read the input is has changed back again.
 * Debouncing is done by using millis() and ButtonLastChangeMillis.
 * Toggle state on button press not on release
 */
volatile uint8_t sBouncings;
// ISR for PIN PD2
// Cannot make the vector itself weak, since the vector table is already filled by weak vectors resulting in ignoring my weak one:-(
//ISR(INT0_vect, __attribute__ ((weak))) {
#if defined(USE_BUTTON_0)
ISR(INT0_vect) {
#ifdef MEASURE_TIMING
    digitalWriteFast(BUTTON_TEST_TIMING_PIN, HIGH);
#endif
    handleINT0Interrupt();
#ifdef MEASURE_TIMING
    digitalWriteFast(BUTTON_TEST_TIMING_PIN, LOW);
#endif
}
#endif

#if defined(USE_BUTTON_1)
ISR(INT1_vect) {
#ifdef MEASURE_TIMING
    digitalWriteFast(BUTTON_TEST_TIMING_PIN, HIGH);
#endif
    handleINT1Interrupt();
#ifdef MEASURE_TIMING
    digitalWriteFast(BUTTON_TEST_TIMING_PIN, LOW);
#endif
}
#endif

#endif /* INT01BUTTONCLASS_H_ */
