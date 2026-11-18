/*
 * PlayRtttl.hpp
 *
 * Plays RTTTL melodies / ringtones from FLASH or RAM.
 * Includes a non blocking version and a name output function.
 *
 *  Copyright (C) 2018-2026  Armin Joachimsmeyer
 *
 *     Based on the RTTTL.pde example code written by Brett Hagman
 *     http://www.roguerobotics.com/
 *     bhagman@roguerobotics.com
 *
 *     The example melodies may have copyrights you have to respect.
 *
 *  This file is part of PlayRtttl https://github.com/ArminJo/PlayRtttl.
 *
 *  PlayRtttl is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/gpl-3.0.html>.
 */

#ifndef _PLAY_RTTTL_HPP
#define _PLAY_RTTTL_HPP

#include "PlayRtttl.h"

#if defined(TRACE)
#define LOCAL_TRACE
// Propagate trace level
#  if !defined(DEBUG)
#define DEBUG
#  endif
#else
//#define LOCAL_TRACE // Activate it to see the note played on Serial output
#endif

#if defined(DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // This enables debug output only for this file
#endif

#if defined(LOCAL_DEBUG) || defined(LOCAL_TRACE)
Print *const sPointerToSerial = &Serial;  // requires 0 bytes Flash because it is constant
#endif //DEBUG

struct playRtttlState sPlayRtttlState;

/*
 * The frequencies of notes of the highest octave.
 * Used to compute all other frequencies.
 */
const uint16_t Notes[] PROGMEM = { NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7,
NOTE_AS7, NOTE_B7 };

#define RTTTL_IS_DIGIT(n) ((n) >= '0' && (n) <= '9') // saves over 100 bytes program space if used instead of isdigit(n)

#if !defined(USE_NO_RTX_EXTENSIONS)
uint8_t sDefaultStyleDivisorValue = RTTTL_STYLE_DEFAULT; // Natural (16)
#endif

/*
 * Blocking versions
 */
void playRtttlBlocking(uint8_t aTonePin, const char *aRTTTLArrayPtr) {
    startPlayRtttl(aTonePin, aRTTTLArrayPtr, nullptr);
    while (updatePlayRtttl()) {
        delay(1); // this in turn calls yield();
    }
}

/*
 * Version for RTTTL Data in RAM. Ie. you must call updatePlayRtttl() in your loop.
 * Since we do not need all the pgm_read_byte() calls this version is more simple and maybe better to understand.
 */
void startPlayRtttl(uint8_t aTonePin, const char *aRTTTLArrayPtr, void (*aOnComplete)()) {
    sPlayRtttlState.Flags.IsPGMMemory = false;
    sPlayRtttlState.OnComplete = aOnComplete;
    sPlayRtttlState.TonePin = aTonePin;
    int tNumber;
    /*
     * Skip name and :
     */
#if defined(LOCAL_DEBUG)
    sPointerToSerial->print(F("Title="));
#endif
    while (*aRTTTLArrayPtr != ':') { // do not check for end of string here, a string without any note makes no sense!
        /*
         * Read title
         */
#if defined(LOCAL_DEBUG)
        sPointerToSerial->print(*aRTTTLArrayPtr);
#endif
        aRTTTLArrayPtr++;
    }

    sPlayRtttlState.DefaultDuration = DEFAULT_DURATION;
    sPlayRtttlState.DefaultOctave = DEFAULT_OCTAVE;
    sPlayRtttlState.TimeForWholeNoteMillis = (60 * 1000L * 4) / DEFAULT_BPM;
#if !defined(USE_NO_RTX_EXTENSIONS)
    sPlayRtttlState.NumberOfRepetitions = 0;
    sPlayRtttlState.StyleDivisorValue = sDefaultStyleDivisorValue;
#endif

#if !defined(USE_NO_RTX_EXTENSIONS)
#if defined(LOCAL_DEBUG)
    char tStyleChar = RTX_STYLE_DEFAULT;
#else
    char tStyleChar;
#endif
#endif
    int tBPM = 0;

    do {
        /*
         * Skip separator (comma or colon)
         */
        aRTTTLArrayPtr++;

        /*
         * Read song info with format: d=N(N),o=N,b=NNN:
         */
        if (*aRTTTLArrayPtr == 'd') {
            /*
             * get default duration
             */
            aRTTTLArrayPtr++;
            aRTTTLArrayPtr++;              // skip "d="
            tNumber = 0;
            while (RTTTL_IS_DIGIT(*aRTTTLArrayPtr)) {
                tNumber = (tNumber * 10) + (*aRTTTLArrayPtr++ - '0');
            }
            if (tNumber == 0) {
                tNumber = DEFAULT_DURATION;
            }
            sPlayRtttlState.DefaultDuration = tNumber;
            // aRTTTLArrayPtr now points to ‘,’ or ':'
        }

        /*
         * get default octave
         */
        else if (*aRTTTLArrayPtr == 'o') {
            aRTTTLArrayPtr++;
            aRTTTLArrayPtr++;              // skip "o="
            tNumber = *aRTTTLArrayPtr++ - '0';
            if (tNumber < 0 || tNumber > 9) { // not a digit
                tNumber = DEFAULT_OCTAVE;
            }
            sPlayRtttlState.DefaultOctave = tNumber;
            // aRTTTLArrayPtr now points to ‘,’ or ':'
        }

#if !defined(USE_NO_RTX_EXTENSIONS)
        else if (*aRTTTLArrayPtr == 's') {
            // get Style
            aRTTTLArrayPtr++;
            aRTTTLArrayPtr++;           // skip "s="
            tStyleChar = *aRTTTLArrayPtr++;
            tNumber = convertStyleCharacterToDivisorValue(tStyleChar);
            sPlayRtttlState.StyleDivisorValue = tNumber;
        }

        // get repetitions
        else if (*aRTTTLArrayPtr == 'l') {
            tNumber = 0;
            aRTTTLArrayPtr++;
            aRTTTLArrayPtr++;              // skip "l="
            while (RTTTL_IS_DIGIT(*aRTTTLArrayPtr)) {
                tNumber = (tNumber * 10) + (*aRTTTLArrayPtr++ - '0');
            }
            if (tNumber > MAXIMUM_OF_REPETITIONS_ACCEPTED) {
                tNumber = 0;
            }
            sPlayRtttlState.NumberOfRepetitions = tNumber;
        }
#endif

        else if (*aRTTTLArrayPtr == 'b') {
            // get BPM
            aRTTTLArrayPtr++;
            aRTTTLArrayPtr++;              // skip "b="
//            tBPM = 0;
            while (RTTTL_IS_DIGIT(*aRTTTLArrayPtr)) {
                tBPM = (tBPM * 10) + (*aRTTTLArrayPtr++ - '0');
            }
            if (tBPM < 10) { // BPM < 10 makes no sense and lead to overflow for tDuration
                tBPM = DEFAULT_BPM;
            }
            // BPM usually expresses the number of quarter notes per minute
            sPlayRtttlState.TimeForWholeNoteMillis = (60 * 1000L * 4) / tBPM;
        }

    } while (*aRTTTLArrayPtr != ':'); // do not check for end of string here, a string without any note makes no sense!

    aRTTTLArrayPtr++; // skip colon

#if defined(LOCAL_DEBUG)
    sPointerToSerial->print(F(" DefaultDuration="));
    sPointerToSerial->print(sPlayRtttlState.DefaultDuration);
    sPointerToSerial->print(F(" DefaultOctave="));
    sPointerToSerial->print(sPlayRtttlState.DefaultOctave);
    sPointerToSerial->print(F(" BPM="));
    sPointerToSerial->print(tBPM);
#  if !defined(USE_NO_RTX_EXTENSIONS)
    sPointerToSerial->print(F(" Style="));
    sPointerToSerial->print(tStyleChar);
    if (sPlayRtttlState.StyleDivisorValue != 0) {
        sPointerToSerial->print(F(" -> 1/"));
        sPointerToSerial->print(sPlayRtttlState.StyleDivisorValue);
        sPointerToSerial->print(F(" pause between notes,"));
    }
    sPointerToSerial->print(F(" Repetitions="));
    sPointerToSerial->print(sPlayRtttlState.NumberOfRepetitions);
#  endif
    sPointerToSerial->println();
#endif

    sPlayRtttlState.ToneStartMillis = 0;
    sPlayRtttlState.NextTonePointer = aRTTTLArrayPtr;
#if !defined(USE_NO_RTX_EXTENSIONS)
    sPlayRtttlState.LastTonePointer = aRTTTLArrayPtr;
#endif
    sPlayRtttlState.Flags.IsRunning = true;

    /*
     * Play first tone
     */
    updatePlayRtttl();
}

bool isPlayRtttlRunning() {
    return sPlayRtttlState.Flags.IsRunning;
}

void stopPlayRtttl(void) {
    noTone(sPlayRtttlState.TonePin);
#if defined(TCCR2A) && (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__) || defined(__AVR_ATmega168__))
    TCCR2A &= ~(_BV(COM2A0) | _BV(COM2B0)); // reset direct hardware toggle output at OC2A / pin 11 and OC2B / pin 3
#endif

    // noTone sets pin to LOW ->  need to handle inverted pin mode here
    if (sPlayRtttlState.Flags.IsTonePinInverted) {
        digitalWrite(sPlayRtttlState.TonePin, HIGH);
    }
    sPlayRtttlState.Flags.IsRunning = false;
}

char getNextCharFromRTTTLArray(const char *aRTTTLArrayPtr) {
    if (sPlayRtttlState.Flags.IsPGMMemory) {
        return pgm_read_byte(aRTTTLArrayPtr);
    }
    return *aRTTTLArrayPtr;
}

/*
 * Returns true if tone is playing, false if tone has ended or stopped
 */
bool updatePlayRtttl(void) {

    if (!sPlayRtttlState.Flags.IsRunning) {
        return false;
    }

#if defined(LOCAL_TRACE)
    bool isSharp = false;
    char tNoteCharUppercase;
#endif

    unsigned long tMillis = millis();
    if ((tMillis - sPlayRtttlState.ToneStartMillis) >= sPlayRtttlState.CurrentToneDuration) {
        sPlayRtttlState.ToneStartMillis = tMillis;

        const char *tRTTTLArrayPtr = sPlayRtttlState.NextTonePointer;

        char tChar;
        tChar = getNextCharFromRTTTLArray(tRTTTLArrayPtr);

        /*
         * Check if end of string reached
         */
        if (tChar == '\0') {
#if !defined(USE_NO_RTX_EXTENSIONS)
            uint8_t tNumberOfRepetitions = sPlayRtttlState.NumberOfRepetitions;
            if (tNumberOfRepetitions > 0) {
                sPlayRtttlState.NumberOfRepetitions--;
                // loop again
#  if defined(LOCAL_DEBUG)
                sPointerToSerial->print(F("Loop count="));
                sPointerToSerial->println(sPlayRtttlState.NumberOfRepetitions);
#  endif
                sPlayRtttlState.ToneStartMillis = 0;
                sPlayRtttlState.NextTonePointer = sPlayRtttlState.LastTonePointer;
                return updatePlayRtttl();
            }
#endif
            // end song
            stopPlayRtttl();
            if (sPlayRtttlState.OnComplete != nullptr) {
                sPlayRtttlState.OnComplete();
            }
            return false;

        }

        uint8_t tDurationNumber;
        uint16_t tDuration;
        uint8_t tNote;
        uint8_t tOctave;
        bool tIncrementOctave = false;

// first, get note duration, if available
        tDurationNumber = 0;
        while (RTTTL_IS_DIGIT(tChar)) {
            tDurationNumber = (tDurationNumber * 10) + (tChar - '0');
            tRTTTLArrayPtr++;
            tChar = getNextCharFromRTTTLArray(tRTTTLArrayPtr);
        }

        if (tDurationNumber == 0) {
            tDurationNumber = sPlayRtttlState.DefaultDuration; // we will need to check if we are a dotted note after
        }
        tDuration = sPlayRtttlState.TimeForWholeNoteMillis / tDurationNumber;

// now get the note
        tNote = NOTE_CODE_FOR_PAUSE;
#if defined(LOCAL_TRACE)
        tNoteCharUppercase = tChar - 0x20;
#endif

        switch (tChar) {
        case 'c':
            tNote = 0;
            break;
        case 'd':
            tNote = 2;
            break;
        case 'e':
            tNote = 4;
            break;
        case 'f':
            tNote = 5;
            break;
        case 'g':
            tNote = 7;
            break;
        case 'a':
            tNote = 9;
            break;
        case 'b':
        case 'h':  // I have seen this
            tNote = 11; // is MAXIMUM_NOTE_INDEX_IN_OCTAVE
            break;
        case 'p':
        default:
#if defined(LOCAL_TRACE)
            tNoteCharUppercase = 'P';
#endif
            tNote = NOTE_CODE_FOR_PAUSE; // pause
        }

        tRTTTLArrayPtr++;
        tChar = getNextCharFromRTTTLArray(tRTTTLArrayPtr);

        // now, get optional '#' sharp (or '_' as seen on many songs)
        if (tChar == '#' || tChar == '_') {
#if defined(LOCAL_TRACE)
            isSharp = true;
#endif
            if (tNote == MAXIMUM_NOTE_INDEX_IN_OCTAVE) {   // b# == c of next octave
                tNote = 0;
                tIncrementOctave = true;
            } else {
                tNote++;
            }
            tRTTTLArrayPtr++;
            tChar = getNextCharFromRTTTLArray(tRTTTLArrayPtr);
        }

// now, get optional '.' of dotted note
        if (tChar == '.') {
            tDuration += tDuration / 2;
#if defined(LOCAL_DEBUG)
            tDurationNumber += tDurationNumber / 2;
#endif
            tRTTTLArrayPtr++;
            tChar = getNextCharFromRTTTLArray(tRTTTLArrayPtr);
        }

// now, get octave
        if (RTTTL_IS_DIGIT(tChar)) {
            tOctave = tChar - '0';
            tRTTTLArrayPtr++;
            tChar = getNextCharFromRTTTLArray(tRTTTLArrayPtr);
        } else {
            tOctave = sPlayRtttlState.DefaultOctave;
        }
        if (tIncrementOctave) {
            tOctave++;
        }

        if (tChar == '.') {         // believe me I have seen this (e.g. in SilentNight)
            tDuration += tDuration / 2;
            tRTTTLArrayPtr++;
            tChar = getNextCharFromRTTTLArray(tRTTTLArrayPtr);
        }

        if (tChar == ',') {
            tRTTTLArrayPtr++;       // skip comma for next note (or we may be at the end)
        }

        /*
         * now play the note
         */
#if !defined(USE_NO_RTX_EXTENSIONS) && defined(LOCAL_TRACE)
        unsigned long tDurationOfTone;
#endif
        if (tNote <= MAXIMUM_NOTE_INDEX_IN_OCTAVE) {
#if defined(__AVR__)
            uint16_t tFrequency = pgm_read_word(&Notes[tNote]);
#else
            uint16_t tFrequency = Notes[tNote];
#endif // defined(__AVR__)
            if (tOctave <= NOTES_OCTAVE) {
                tFrequency >>= (NOTES_OCTAVE - tOctave);
            } else {
                tFrequency <<= (tOctave - NOTES_OCTAVE);
            }

#if !defined(USE_NO_RTX_EXTENSIONS)

            if (sPlayRtttlState.StyleDivisorValue != 0) {
                /*
                 * handle style parameter, compute duration of tone output for note and do rounding for integer division
                 */
#if !defined(LOCAL_TRACE)
                unsigned long tDurationOfTone; // This variable is required for trace below
#endif
                tDurationOfTone = tDuration
                        - ((tDuration + (sPlayRtttlState.StyleDivisorValue / 2)) / sPlayRtttlState.StyleDivisorValue);
                tone(sPlayRtttlState.TonePin, tFrequency, tDurationOfTone);
            } else {
                tone(sPlayRtttlState.TonePin, tFrequency, tDuration);
            }

#else
            /*
             * Generate the tone
             */
            // even without USE_NO_RTX_EXTENSIONS the default style is natural (Tone length = note length - 1/16)
            tone(sPlayRtttlState.TonePin, tFrequency, tDuration - (tDuration >> 4));
#endif
#if defined(TCCR2A) && (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__) || defined(__AVR_ATmega168__))
            if (sPlayRtttlState.TonePin == 11) {
                // switch to direct hardware toggle output at OC2A / pin 11
                TCCR2A |= _BV(COM2A0);
            }
            if (sPlayRtttlState.TonePin == 3) {
                // switch to direct hardware toggle output at OC2B / pin 3. This indeed works :-), despite of the tone() ISR TIMER2_COMPA_vect.
                TCCR2A |= _BV(COM2B0);
            }

#endif

        } else {
            // Play pause, need to handle inverted pin mode here
            noTone(sPlayRtttlState.TonePin);
#if defined(TCCR2A) && (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__) || defined(__AVR_ATmega168__))
            // reset direct hardware toggle output at OC2A / pin 11 and OC2B / pin 3
            TCCR2A &= ~(_BV(COM2A0) | _BV(COM2B0));
#endif

            if (sPlayRtttlState.Flags.IsTonePinInverted) {
                digitalWrite(sPlayRtttlState.TonePin, HIGH);
            }
        }
#if defined(LOCAL_TRACE)
        sPointerToSerial->print(F("Playing: NOTE_"));
        sPointerToSerial->print(tNoteCharUppercase);
        if (isSharp) {
            sPointerToSerial->print('#');
        }
        if (tNote <= MAXIMUM_NOTE_INDEX_IN_OCTAVE) {
            sPointerToSerial->print(tOctave, 10);
        }
        sPointerToSerial->print(F(", "));
        sPointerToSerial->print(tDurationNumber, 10);

        sPointerToSerial->print(F(" | "));
        if (tNote <= MAXIMUM_NOTE_INDEX_IN_OCTAVE && tOctave <= NOTES_OCTAVE) {
#  if defined(__AVR__)
            sPointerToSerial->print(pgm_read_word(&Notes[tNote]) >> (NOTES_OCTAVE - tOctave), 10);
#  else
            sPointerToSerial->print(Notes[tNote] >> (NOTES_OCTAVE - tOctave), 10);
#  endif
            sPointerToSerial->print(F(" Hz"));
        }
        sPointerToSerial->print(F(" for "));
#  if !defined(USE_NO_RTX_EXTENSIONS)
        if (sPlayRtttlState.StyleDivisorValue != 0 && tNote <= MAXIMUM_NOTE_INDEX_IN_OCTAVE) {
            sPointerToSerial->print(tDurationOfTone, 10);
            sPointerToSerial->print(F(" of "));
        }
#  endif
        sPointerToSerial->print(tDuration, 10);
        sPointerToSerial->println(F(" ms"));

#endif //LOCAL_TRACE
        sPlayRtttlState.CurrentToneDuration = tDuration;
        sPlayRtttlState.NextTonePointer = tRTTTLArrayPtr;
    }
    return true;
}

void getRtttlName(const char *aRTTTLArrayPtr, char *aBuffer, uint8_t aBuffersize) {
    char tChar = *aRTTTLArrayPtr++;
    while (tChar != ':' && aBuffersize > 1) { // do not check for end of string here, a string without any note makes no sense!
        *aBuffer++ = tChar;
        aBuffersize--;
        tChar = *aRTTTLArrayPtr++;
    }
    *aBuffer = '\0';
}

/*
 * Prints text "Now playing: Song xy"
 * call it e.g. printName(&Serial, MySongInRam);
 */
void printName(Print *aSerial, const char *aRTTTLArrayPtr) {
    aSerial->print(F("Now playing: "));
    char tChar;
    while ((tChar = *aRTTTLArrayPtr++) != ':') { // do not check for end of string here, a string without any note makes no sense!
        aSerial->print(tChar);
    }
    aSerial->println();
}

/**
 * @param  aRTTTLPGMArrayPtrPGM a pointer to PGM song data
 */
void printNamePGM(Print *aSerial, const char *aRTTTLArrayPtrPGM) {
#if !defined(__AVR__) // Let the function work for non AVR platforms
    printName(aSerial, aRTTTLArrayPtrPGM);
#else
    aSerial->print(F("Now playing: "));
    char tChar;
    while ((tChar = pgm_read_byte(aRTTTLArrayPtrPGM++)) != ':') { // do not check for end of string here, a string without any note makes no sense!
        aSerial->print(tChar);
    }
    aSerial->println();
#endif
}

/**
 * @param  aRTTTLPGMArrayPtrPGM a pointer to an PGM array of pointers to PGM song data
 */
void printNamePGMPGM(Print *aSerial, const char *const*aRTTTLPGMArrayPtrPGM) {
#if defined(__AVR__)
    const char *tRTTTLPGMArrayPtrPGM = (const char*) pgm_read_word(aRTTTLPGMArrayPtrPGM);
#else
    const char *tRTTTLPGMArrayPtrPGM = *aRTTTLPGMArrayPtrPGM;
#endif
    printNamePGM(aSerial, tRTTTLPGMArrayPtrPGM);
}

const char* getRandomSongPtr(uint8_t aNumberOfEntriesInSongArray, const char *const aSongArray[]) {
    return (aSongArray[random(0, aNumberOfEntriesInSongArray)]);
}
const char* getRandomSongPtrPGM(uint8_t aNumberOfEntriesInSongArrayPGM, const char *const aSongArrayPGM[]) {
#if defined(__AVR__)
    return (const char*) pgm_read_word(&aSongArrayPGM[random(aNumberOfEntriesInSongArrayPGM)]);
#else
    return aSongArrayPGM[random(aNumberOfEntriesInSongArrayPGM)];
#endif
}

/*
 * Plays one of the samples from RTTTLMelodies array
 */
void playRandomRtttlSampleBlocking(uint8_t aTonePin) {
#if defined(__AVR__)
    playRandomRtttlSampleBlockingPGM(aTonePin); // RTTTLMelodies is PROGMEM here, so use the right function
#else
    const char *tSongPtr = getRandomSongPtr(ARRAY_SIZE_MELODIES, RTTTLMelodies);
    playRtttlBlocking(aTonePin, tSongPtr);
#endif
}

void playRandomRtttlSampleBlockingAndPrintName(Print *aSerial, uint8_t aTonePin) {
#if defined(__AVR__)
    playRandomRtttlSampleBlockingPGMAndPrintName(aSerial, aTonePin); // RTTTLMelodies is PROGMEM here, so use the right function
#else
    const char *tSongPtr = getRandomSongPtr(ARRAY_SIZE_MELODIES, RTTTLMelodies);
    printName(aSerial, tSongPtr);
    playRtttlBlocking(aTonePin, tSongPtr);
#endif
}

/*
 * Plays one of the songs in the array specified non blocking. Ie. you must call updatePlayRtttl() in your loop or use the callback function.
 * aNumberOfEntriesInSongArrayPGM is (sizeof(<MyArrayName>) / sizeof(char *))
 * char tStringBuffer[16] is sufficient for most titles.
 */
void startPlayRandomRtttlFromArray(uint8_t aTonePin, const char *const aSongArray[], uint8_t aNumberOfEntriesInSongArray,
        char *aBufferPointer, uint8_t aBufferSize, void (*aOnComplete)()) {
    const char *tSongPtr = getRandomSongPtr(aNumberOfEntriesInSongArray, aSongArray);
    startPlayRtttl(aTonePin, tSongPtr, aOnComplete);
    if (aBufferPointer != nullptr) {
// copy title to buffer
        getRtttlName(tSongPtr, aBufferPointer, aBufferSize);
    }
}

void startPlayRandomRtttlFromArrayAndPrintName(Print *aSerial, uint8_t aTonePin, const char *const aSongArray[],
        uint8_t aNumberOfEntriesInSongArray, void (*aOnComplete)()) {
    const char *tSongPtr = getRandomSongPtr(aNumberOfEntriesInSongArray, aSongArray);
    startPlayRtttl(aTonePin, tSongPtr, aOnComplete);
    printName(aSerial, tSongPtr);
}

void playRtttlBlockingPGM(uint8_t aTonePin, const char *aRTTTLArrayPtrPGM) {
    startPlayRtttlPGM(aTonePin, aRTTTLArrayPtrPGM, nullptr);
    while (updatePlayRtttl()) {
        delay(1); // this in turn calls yield();
    }
}

/**
 * @param  aRTTTLArrayPtrPGM a pointer to an PGM array of pointers to PGM song data
 */
void startPlayRtttlPGMPGM(uint8_t aTonePin, const char *const*aRTTTLPGMArrayPtrPGM, void (*aOnComplete)()) {
#if defined(__AVR__)
    const char *tRTTTLPGMArrayPtrPGM = (const char*) pgm_read_word(aRTTTLPGMArrayPtrPGM);
#else
    const char *tRTTTLPGMArrayPtrPGM = *aRTTTLPGMArrayPtrPGM;
#endif
    startPlayRtttlPGM(aTonePin, tRTTTLPGMArrayPtrPGM, aOnComplete);
}

/*
 * Non blocking version for RTTTL Data in FLASH. Ie. you must call updatePlayRtttl() in your loop.
 * @param  aRTTTLArrayPtrPGM a pointer to PGM song data
 */
void startPlayRtttlPGM(uint8_t aTonePin, const char *aRTTTLArrayPtrPGM, void (*aOnComplete)()) {
    sPlayRtttlState.Flags.IsPGMMemory = true;
    sPlayRtttlState.OnComplete = aOnComplete;
    sPlayRtttlState.TonePin = aTonePin;

    int tNumber;

    /*
     * Skip name and :
     */
#if defined(LOCAL_DEBUG)
    sPointerToSerial->print(F("Title="));
#endif
    char tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
    while (tPGMChar != ':') { // do not check for end of string here, a string without any note makes no sense!
        /*
         * Read title
         */
#if defined(LOCAL_DEBUG)
        sPointerToSerial->print(tPGMChar);
#endif
        aRTTTLArrayPtrPGM++;
        tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
    }

    sPlayRtttlState.DefaultDuration = DEFAULT_DURATION;
    sPlayRtttlState.DefaultOctave = DEFAULT_OCTAVE;
    sPlayRtttlState.TimeForWholeNoteMillis = (60 * 1000L * 4) / DEFAULT_BPM;
#if !defined(USE_NO_RTX_EXTENSIONS)
    sPlayRtttlState.NumberOfRepetitions = 0;
    sPlayRtttlState.StyleDivisorValue = sDefaultStyleDivisorValue;
#endif

#if !defined(USE_NO_RTX_EXTENSIONS)
    char tStyleChar;
#endif
    int tBPM = 0;

    do {
        /*
         * Get character after separator (comma or colon)
         */
        aRTTTLArrayPtrPGM++;
        tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
        /*
         * Read song info with format: d=N(N),o=N,b=NNN:
         */
        if (tPGMChar == 'd') {
            /*
             * get default duration
             */
            aRTTTLArrayPtrPGM++;
            aRTTTLArrayPtrPGM++;              // skip "d="
            tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
            tNumber = 0;
            while (RTTTL_IS_DIGIT(tPGMChar)) {
                tNumber = (tNumber * 10) + (tPGMChar - '0');
                aRTTTLArrayPtrPGM++;
                tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
            }
            if (tNumber == 0) {
                tNumber = DEFAULT_DURATION;
            }
            sPlayRtttlState.DefaultDuration = tNumber;
            // aRTTTLArrayPtrPGM now points to ',' or ':' and tPGMChar contains it

        } else if (tPGMChar == 'o') {
            /*
             * get default octave
             */
            aRTTTLArrayPtrPGM++;
            aRTTTLArrayPtrPGM++;              // skip "o="
            tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
            tNumber = tPGMChar - '0';
            if (tNumber < 0 || tNumber > 9) { // not a digit
                tNumber = DEFAULT_OCTAVE;
            }
            sPlayRtttlState.DefaultOctave = tNumber;
            //get comma or colon
            aRTTTLArrayPtrPGM++;
            tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
            // aRTTTLArrayPtrPGM now points to ',' or ':' and tPGMChar contains it
        } else

#if !defined(USE_NO_RTX_EXTENSIONS)
        if (tPGMChar == 's') {
            // get Style
            aRTTTLArrayPtrPGM++;
            aRTTTLArrayPtrPGM++;              // skip "s="
            tStyleChar = pgm_read_byte(aRTTTLArrayPtrPGM);
            tNumber = convertStyleCharacterToDivisorValue(tStyleChar);
            sPlayRtttlState.StyleDivisorValue = tNumber;
            //get comma or colon
            aRTTTLArrayPtrPGM++;
            tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);

        } else if (tPGMChar == 'l') {
            // get repetitions
            aRTTTLArrayPtrPGM++;
            aRTTTLArrayPtrPGM++;              // skip "l="
            tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
            tNumber = 0;
            while (RTTTL_IS_DIGIT(tPGMChar)) {
                tNumber = (tNumber * 10) + (tPGMChar - '0');
                aRTTTLArrayPtrPGM++;
                tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
            }
            if (tNumber > MAXIMUM_OF_REPETITIONS_ACCEPTED) {
                tNumber = 0;
            }
            sPlayRtttlState.NumberOfRepetitions = tNumber;
        } else
#endif

        // get BPM
        if (tPGMChar == 'b') {
            aRTTTLArrayPtrPGM++;
            aRTTTLArrayPtrPGM++;              // skip "b="
            tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
            while (RTTTL_IS_DIGIT(tPGMChar)) {
                tBPM = (tBPM * 10) + (tPGMChar - '0');
                aRTTTLArrayPtrPGM++;
                tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
            }
            if (tBPM < 10) { // BPM < 10 makes no sense and lead to overflow for tDuration
                tBPM = DEFAULT_BPM;
            }
            // BPM usually expresses the number of quarter notes per minute
            sPlayRtttlState.TimeForWholeNoteMillis = (60 * 1000L * 4) / tBPM;
        }

    } while (tPGMChar != ':'); // do not check for end of string here, a string without any note makes no sense!

    aRTTTLArrayPtrPGM++; // skip colon

#if defined(LOCAL_DEBUG)
    sPointerToSerial->print(F(" DefaultDuration="));
    sPointerToSerial->print(sPlayRtttlState.DefaultDuration);
    sPointerToSerial->print(F(" DefaultOctave="));
    sPointerToSerial->print(sPlayRtttlState.DefaultOctave);
    sPointerToSerial->print(F(" BPM="));
    sPointerToSerial->print(tBPM);
#if !defined(USE_NO_RTX_EXTENSIONS)
    sPointerToSerial->print(F(" Style="));
    sPointerToSerial->print(tStyleChar);
    if (sPlayRtttlState.StyleDivisorValue != 0) {
        sPointerToSerial->print(F(" -> 1/"));
        sPointerToSerial->print(sPlayRtttlState.StyleDivisorValue);
        sPointerToSerial->print(F(" pause between notes,"));
    }
    sPointerToSerial->print(F(" Repetitions="));
    sPointerToSerial->print(sPlayRtttlState.NumberOfRepetitions);
#endif
    sPointerToSerial->println();
#endif

    sPlayRtttlState.ToneStartMillis = 0;
    sPlayRtttlState.NextTonePointer = aRTTTLArrayPtrPGM;
#if !defined(USE_NO_RTX_EXTENSIONS)
    sPlayRtttlState.LastTonePointer = aRTTTLArrayPtrPGM;
#endif
    sPlayRtttlState.Flags.IsRunning = true;

    /*
     * Play first tone
     */
    updatePlayRtttl();
}

/**
 * @param  aRTTTLPGMArrayPtrPGM a pointer to PGM song data
 */
void getRtttlNamePGM(const char *aRTTTLArrayPtrPGM, char *aBuffer, uint8_t aBuffersize) {
#if !defined(__AVR__) // Let the function work for non AVR platforms
    getRtttlName(aRTTTLArrayPtrPGM, aBuffer, aBuffersize);
#else
    char tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM++);
    while (tPGMChar != ':' && aBuffersize > 1) { // do not check for end of string here, a string without any note makes no sense!
        *aBuffer++ = tPGMChar;
        aBuffersize--;
        tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM++);
    }
    *aBuffer = '\0';
#endif
}

/*
 * !!! Songs are in an array stored in FLASH containing pointers to song arrays also stored in FLASH, see PlayRtttl.h. !!!
 * Plays one of the songs in the array specified non blocking. Ie. you must call updatePlayRtttl() in your loop or use the callback function.
 * aNumberOfEntriesInSongArrayPGM is (sizeof(<MyArrayName>) / sizeof(char *))
 * char tStringBuffer[16] is sufficient for most titles.
 */
void startPlayRandomRtttlFromArrayPGM(uint8_t aTonePin, const char *const aSongArrayPGM[], uint8_t aNumberOfEntriesInSongArrayPGM,
        char *aBufferPointer, uint8_t aBufferSize, void (*aOnComplete)()) {
#if !defined(__AVR__) // Let the function work for non AVR platforms
    startPlayRandomRtttlFromArray(aTonePin, aSongArrayPGM, aNumberOfEntriesInSongArrayPGM, aBufferPointer, aBufferSize,
            aOnComplete);
#else
    const char *tSongPtr = getRandomSongPtrPGM(aNumberOfEntriesInSongArrayPGM, aSongArrayPGM);
    startPlayRtttlPGM(aTonePin, tSongPtr, aOnComplete);
    if (aBufferPointer != nullptr) {
// copy title to buffer
        getRtttlNamePGM(tSongPtr, aBufferPointer, aBufferSize);
    }
#endif
}

/*
 * !!! Songs are in an array stored in FLASH containing pointers to song arrays also stored in FLASH, see PlayRtttl.h. !!!
 */
void startPlayRandomRtttlFromArrayPGMAndPrintName(Print *aSerial, uint8_t aTonePin, const char *const aSongArrayPGM[],
        uint8_t aNumberOfEntriesInSongArrayPGM, void (*aOnComplete)()) {
#if !defined(__AVR__) // Let the function work for non AVR platforms
    startPlayRandomRtttlFromArrayAndPrintName(aSerial, aTonePin, aSongArrayPGM, aNumberOfEntriesInSongArrayPGM, aOnComplete);
#else
    const char *tSongPtr = getRandomSongPtrPGM(aNumberOfEntriesInSongArrayPGM, aSongArrayPGM);
    startPlayRtttlPGM(aTonePin, tSongPtr, aOnComplete);
    printNamePGM(aSerial, tSongPtr);
#endif
}

/*
 * Plays one of the samples from RTTTLMelodies array
 */
void playRandomRtttlSampleBlockingPGM(uint8_t aTonePin) {
#if !defined(__AVR__) // Let the function work for non AVR platforms
    playRandomRtttlSampleBlocking(aTonePin);
#else
    const char *tSongPtr = getRandomSongPtrPGM(ARRAY_SIZE_MELODIES, RTTTLMelodies);
    playRtttlBlockingPGM(aTonePin, tSongPtr);
#endif
}

void playRandomRtttlSampleBlockingPGMAndPrintName(Print *aSerial, uint8_t aTonePin) {
#if !defined(__AVR__) // Let the function work for non AVR platforms
    playRandomRtttlSampleBlockingAndPrintName(aSerial, aTonePin);
#else
    const char *tSongPtr = getRandomSongPtrPGM(ARRAY_SIZE_MELODIES, RTTTLMelodies);
    printNamePGM(aSerial, tSongPtr);
    playRtttlBlockingPGM(aTonePin, tSongPtr);
#endif
}

void setTonePinIsInverted(bool aTonePinIsInverted) {
    sPlayRtttlState.Flags.IsTonePinInverted = aTonePinIsInverted;
}

#if !defined(USE_NO_RTX_EXTENSIONS)
/*
 * 0 means play melody only once without any repetition
 * Call after startPlayRtttl…()
 */
void setNumberOfRepetitions(uint8_t aNumberOfRepetitions) {
    sPlayRtttlState.NumberOfRepetitions = aNumberOfRepetitions;
#if defined(LOCAL_DEBUG)
    sPointerToSerial->print(F("Set NumberOfRepetitions to "));
    sPointerToSerial->println(sPlayRtttlState.NumberOfRepetitions);
#endif
}

/*
 * Set the divisor for formula: Tone length = note length - note length * (1 / divisor)
 * Default is 16 / RTTTL_STYLE_NATURAL
 */
void setDefaultStyle(uint8_t aDefaultStyleDivisorValue) {
    sDefaultStyleDivisorValue = aDefaultStyleDivisorValue;
}

uint8_t convertStyleCharacterToDivisorValue(char aStyleCharacter) {
    if (aStyleCharacter == RTX_STYLE_STACCATO) {
        return 2;
    } else if (aStyleCharacter == RTX_STYLE_NATURAL) {
        return 16;
    } else if (aStyleCharacter >= '2' && aStyleCharacter <= '9') {
        // convert ASCII to number
        return aStyleCharacter - '0';
    }
    return 0; // RTX_STYLE_CONTINUOUS
}
#endif // USE_NO_RTX_EXTENSIONS

#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#if defined(LOCAL_TRACE)
#undef LOCAL_TRACE
#endif

#undef RTTTL_IS_DIGIT

#endif // _PLAY_RTTTL_HPP
