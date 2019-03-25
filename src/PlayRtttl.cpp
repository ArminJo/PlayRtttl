/*
 * PlayRttl.cpp
 * Plays RTTTL melodies / ringtones from FLASH or RAM.
 * Includes a non blocking version and a name output function.
 *
 *  Copyright (C) 2018  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *     Based on the RTTTL.pde example code written by Brett Hagman
 *     http://www.roguerobotics.com/
 *     bhagman@roguerobotics.com
 *
 *     The example melodies may have copyrights you have to respect.
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

#include "PlayRtttl.h"

//uncomment next line to see debug output, which shows the note played, on serial.
//#define DEBUG

struct playRtttlState sPlayRtttlState;

int notes[] = { 0,
NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4,
NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6,
NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7 };

#define OCTAVE_OFFSET 0
#define isdigit(n) (n >= '0' && n <= '9')

/*
 * Blocking versions
 */
void playRtttlBlocking(uint8_t aTonePin, char *aRTTTLArrayPtr) {
    startPlayRtttl(aTonePin, aRTTTLArrayPtr, NULL);
    while (updatePlayRtttl()) {
        delay(1); // this in turn calls yield();
    }
}

void playRtttlBlockingPGM(uint8_t aTonePin, const char *aRTTTLArrayPtrPGM) {
    startPlayRtttlPGM(aTonePin, aRTTTLArrayPtrPGM, NULL);
    while (updatePlayRtttl()) {
        delay(1); // this in turn calls yield();
    }
}

/*
 * Non blocking version for RTTTL Data in FLASH. Ie. you must call updatePlayRtttl() in your loop.
 */
void startPlayRtttlPGM(uint8_t aTonePin, const char * aRTTTLArrayPtrPGM, void (*aOnComplete)()) {
    sPlayRtttlState.Flags.IsPGMMemory = true;
    sPlayRtttlState.OnComplete = aOnComplete;
    sPlayRtttlState.TonePin = aTonePin;
    int tNumber;

    /*
     * Skip name and :
     */
#ifdef DEBUG
    Serial.print("Title=");
#endif
    char tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
    while (tPGMChar != ':') {
#ifdef DEBUG
        Serial.print(tPGMChar);
#endif
        aRTTTLArrayPtrPGM++;
        tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
    }
    aRTTTLArrayPtrPGM++;
    tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);

    /*
     * Read song info with format: d=N(N),o=N,b=NNN:
     */

    /*
     * get default duration
     */
    tNumber = DEFAULT_DURATION;
    if (tPGMChar == 'd') {
        aRTTTLArrayPtrPGM++;
        aRTTTLArrayPtrPGM++;              // skip "d="
        tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
        tNumber = 0;
        while (isdigit(tPGMChar)) {
            tNumber = (tNumber * 10) + (tPGMChar - '0');
            aRTTTLArrayPtrPGM++;
            tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
        }
        if (tNumber == 0) {
            tNumber = DEFAULT_DURATION;
        }
        aRTTTLArrayPtrPGM++;                   // skip comma
        tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
    }
    sPlayRtttlState.DefaultDuration = tNumber;

    /*
     * get default octave
     */
    tNumber = DEFAULT_OCTAVE;
    if (tPGMChar == 'o') {
        aRTTTLArrayPtrPGM++;
        aRTTTLArrayPtrPGM++;              // skip "o="
        tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
        aRTTTLArrayPtrPGM++;
        tNumber = tPGMChar - '0';
        if (tNumber < 3 && tNumber > 7) {
            tNumber = DEFAULT_OCTAVE;
        }

        aRTTTLArrayPtrPGM++;                   // skip comma
        tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
    }
    sPlayRtttlState.DefaultOctave = tNumber;

    // get BPM
    tNumber = DEFAULT_BPM;
    if (tPGMChar == 'b') {
        aRTTTLArrayPtrPGM++;
        aRTTTLArrayPtrPGM++;              // skip "b="
        tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
        tNumber = 0;
        while (isdigit(tPGMChar)) {
            tNumber = (tNumber * 10) + (tPGMChar - '0');
            aRTTTLArrayPtrPGM++;
            tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
        }
        if (tNumber == 0) {
            tNumber = DEFAULT_BPM;
        }
        aRTTTLArrayPtrPGM++;                   // skip colon
        tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM);
    }
    // BPM usually expresses the number of quarter notes per minute
    sPlayRtttlState.TimeForWholeNoteMillis = (60 * 1000L / tNumber) * 4;

#ifdef DEBUG
    Serial.print(" DefaultDuration=");
    Serial.print(sPlayRtttlState.DefaultDuration);
    Serial.print(" DefaultOctave=");
    Serial.print(sPlayRtttlState.DefaultOctave);
    Serial.print(" BPM=");
    Serial.println(tNumber);
#endif

    sPlayRtttlState.MillisOfNextAction = 0;
    sPlayRtttlState.NextTonePointer = aRTTTLArrayPtrPGM;
    sPlayRtttlState.Flags.IsStopped = false;

    /*
     * Play first tone
     */
    updatePlayRtttl();
}

/*
 * Version for RTTTL Data in RAM. Ie. you must call updatePlayRtttl() in your loop.
 * Since we do not need all the pgm_read_byte() calls this version is more simple and maybe better to understand.
 */
void startPlayRtttl(uint8_t aTonePin, char * aRTTTLArrayPtr, void (*aOnComplete)()) {
    sPlayRtttlState.Flags.IsPGMMemory = false;
    sPlayRtttlState.OnComplete = aOnComplete;
    sPlayRtttlState.TonePin = aTonePin;
    int tNumber;
    /*
     * Skip name and :
     */
#ifdef DEBUG
    Serial.print("Title=");
#endif
    while (*aRTTTLArrayPtr != ':') {
#ifdef DEBUG
        Serial.print(*aRTTTLArrayPtr);
#endif
        aRTTTLArrayPtr++;
    }
    aRTTTLArrayPtr++;

    /*
     * Read song info with format: d=N,o=N,b=NNN:
     */

    /*
     * get default duration
     */
    tNumber = DEFAULT_DURATION;
    if (*aRTTTLArrayPtr == 'd') {
        aRTTTLArrayPtr++;
        aRTTTLArrayPtr++;              // skip "d="
        tNumber = 0;
        while (isdigit(*aRTTTLArrayPtr)) {
            tNumber = (tNumber * 10) + (*aRTTTLArrayPtr++ - '0');
        }
        if (tNumber == 0) {
            tNumber = DEFAULT_DURATION;
        }
        aRTTTLArrayPtr++;                   // skip comma
    }
    sPlayRtttlState.DefaultDuration = tNumber;

    /*
     * get default octave
     */
    tNumber = DEFAULT_OCTAVE;
    if (*aRTTTLArrayPtr == 'o') {
        aRTTTLArrayPtr++;
        aRTTTLArrayPtr++;              // skip "o="
        tNumber = *aRTTTLArrayPtr++ - '0';
        if (tNumber < 3 && tNumber > 7) {
            tNumber = DEFAULT_OCTAVE;
        }
        aRTTTLArrayPtr++;                   // skip comma
    }
    sPlayRtttlState.DefaultOctave = tNumber;

    // get BPM
    tNumber = DEFAULT_BPM;
    if (*aRTTTLArrayPtr == 'b') {
        aRTTTLArrayPtr++;
        aRTTTLArrayPtr++;              // skip "b="
        tNumber = 0;
        while (isdigit(*aRTTTLArrayPtr)) {
            tNumber = (tNumber * 10) + (*aRTTTLArrayPtr++ - '0');
        }
        if (tNumber == 0) {
            tNumber = DEFAULT_BPM;
        }
        aRTTTLArrayPtr++;                   // skip colon
    }
    // BPM usually expresses the number of quarter notes per minute
    sPlayRtttlState.TimeForWholeNoteMillis = (60 * 1000L / tNumber) * 4;

#ifdef DEBUG
    Serial.print(" DefaultDuration=");
    Serial.print(sPlayRtttlState.DefaultDuration);
    Serial.print(" DefaultOctave=");
    Serial.print(sPlayRtttlState.DefaultOctave);
    Serial.print(" BPM=");
    Serial.println(tNumber);
#endif

    sPlayRtttlState.MillisOfNextAction = 0;
    sPlayRtttlState.NextTonePointer = aRTTTLArrayPtr;
    sPlayRtttlState.Flags.IsStopped = false;

    /*
     * Play first tone
     */
    updatePlayRtttl();
}

void stopPlayRtttl(void) {
    noTone(sPlayRtttlState.TonePin);
    sPlayRtttlState.Flags.IsStopped = true;
}

char getNextCharFromRTTLArray(const char* aRTTTLArrayPtr) {
    if (sPlayRtttlState.Flags.IsPGMMemory) {
        return pgm_read_byte(aRTTTLArrayPtr);
    }
    return *aRTTTLArrayPtr;
}

/*
 * Returns true if tone is playing, false if tone has ended or stopped
 */
bool updatePlayRtttl(void) {

    if (sPlayRtttlState.Flags.IsStopped) {
        return false;
    }

#ifdef DEBUG
    bool isSharp = false;
    char tNoteCharUppercase;
#endif

    long tMillis = millis();
    if (sPlayRtttlState.MillisOfNextAction <= tMillis) {
        noTone(sPlayRtttlState.TonePin);
        const char * tRTTTLArrayPtr = sPlayRtttlState.NextTonePointer;

        char tChar;
        tChar = getNextCharFromRTTLArray(tRTTTLArrayPtr);

        /*
         * Check if end of string reached
         */
        if (tChar == '\0') {
            noTone(sPlayRtttlState.TonePin);
            sPlayRtttlState.Flags.IsStopped = true;
            if (sPlayRtttlState.OnComplete != NULL) {
                sPlayRtttlState.OnComplete();
            }
            return false;
        }

        uint8_t tDurationNumber;
        long tDuration;
        uint8_t tNote;
        uint8_t tOctave;

// first, get note duration, if available
        tDurationNumber = 0;
        while (isdigit(tChar)) {
            tDurationNumber = (tDurationNumber * 10) + (tChar - '0');
            tRTTTLArrayPtr++;
            tChar = getNextCharFromRTTLArray(tRTTTLArrayPtr);
        }

        if (tDurationNumber == 0) {
            tDurationNumber = sPlayRtttlState.DefaultDuration; // we will need to check if we are a dotted note after
        }
        tDuration = sPlayRtttlState.TimeForWholeNoteMillis / tDurationNumber;

// now get the note
        tNote = 0;
#ifdef DEBUG
        tNoteCharUppercase = tChar - 0x20;
#endif

        switch (tChar) {
        case 'c':
            tNote = 1;
            break;
        case 'd':
            tNote = 3;
            break;
        case 'e':
            tNote = 5;
            break;
        case 'f':
            tNote = 6;
            break;
        case 'g':
            tNote = 8;
            break;
        case 'a':
            tNote = 10;
            break;
        case 'b':
        case 'h':  // I have seen this
            tNote = 12;
            break;
        case 'p':
        default:
            tNote = 0;
        }

        tRTTTLArrayPtr++;
        tChar = getNextCharFromRTTLArray(tRTTTLArrayPtr);

        // now, get optional '#' sharp (or '_' as seen on many songs)
        if (tChar == '#' || tChar == '_') {
#ifdef DEBUG
            isSharp = true;
#endif
            tNote++;
            tRTTTLArrayPtr++;
            tChar = getNextCharFromRTTLArray(tRTTTLArrayPtr);
        }

// now, get optional '.' dotted note
        if (tChar == '.') {
            tDuration += tDuration / 2;
#ifdef DEBUG
            tDurationNumber += tDurationNumber / 2;
#endif
            tRTTTLArrayPtr++;
            tChar = getNextCharFromRTTLArray(tRTTTLArrayPtr);
        }

// now, get octave
        if (isdigit(tChar)) {
            tOctave = tChar - '0';
            tRTTTLArrayPtr++;
            tChar = getNextCharFromRTTLArray(tRTTTLArrayPtr);
        } else {
            tOctave = sPlayRtttlState.DefaultOctave;
        }

        tOctave += OCTAVE_OFFSET;

        if (tChar == '.') { 	// believe me I have seen this (e.g. in SilentNight)
            tDuration += tDuration / 2;
            tRTTTLArrayPtr++;
            tChar = getNextCharFromRTTLArray(tRTTTLArrayPtr);
        }

        if (tChar == ',') {
            tRTTTLArrayPtr++;       // skip comma for next note (or we may be at the end)
        }

        /*
         * now play the note
         */
        if (tNote > 0) {
            tone(sPlayRtttlState.TonePin, notes[(tOctave - 4) * 12 + tNote]);
        } else {
            noTone(sPlayRtttlState.TonePin);
            if (sPlayRtttlState.Flags.IsTonePinInverted) {
                digitalWrite(sPlayRtttlState.TonePin, HIGH);
            }
        }
#ifdef DEBUG
        Serial.print("Playing: ");
        Serial.print("NOTE_");
        Serial.print(tNoteCharUppercase);
        if (isSharp) {
            Serial.print('#');
        }
        Serial.print(tOctave, 10);
        Serial.print(", ");
        Serial.print(tDurationNumber, 10);

        Serial.print(" | ");
        Serial.print(notes[(tOctave - 4) * 12 + tNote], 10);
        Serial.print("Hz for ");
        Serial.print(tDuration, 10);
        Serial.println("ms");

#endif
        sPlayRtttlState.MillisOfNextAction = tMillis + tDuration;
        sPlayRtttlState.NextTonePointer = tRTTTLArrayPtr;
    }
    return true;
}

void getRtttlNamePGM(const char *aRTTTLArrayPtrPGM, char * aBuffer, uint8_t aBuffersize) {
    char tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM++);
    while (tPGMChar != ':' && aBuffersize > 1) {
        *aBuffer++ = tPGMChar;
        aBuffersize--;
        tPGMChar = pgm_read_byte(aRTTTLArrayPtrPGM++);
    }
    *aBuffer = '\0';
}

void getRtttlName(char *aRTTTLArrayPtr, char * aBuffer, uint8_t aBuffersize) {
    char tChar = *aRTTTLArrayPtr++;
    while (tChar != ':' && aBuffersize > 1) {
        *aBuffer++ = tChar;
        aBuffersize--;
        tChar = *aRTTTLArrayPtr++;
    }
    *aBuffer = '\0';
}

/*
 * Prints text "Now playing: Song xy"
 */
void printNamePGM(const char *aRTTTLArrayPtrPGM) {
    char StringBuffer[16];
    Serial.print(F("Now playing: "));
    getRtttlNamePGM(aRTTTLArrayPtrPGM, StringBuffer, sizeof(StringBuffer));
    Serial.println(StringBuffer);
}

/*
 * not used yet
 */
void printName(char *aRTTTLArrayPtr) {
    char StringBuffer[16];
    Serial.print(F("Now playing: "));
    getRtttlName(aRTTTLArrayPtr, StringBuffer, sizeof(StringBuffer));
    Serial.println(StringBuffer);
}

/*
 * Plays one of the examples
 */
void playRandomRtttlBlocking(uint8_t aTonePin) {
    uint8_t tRandomIndex = random(0, sizeof(RTTTLMelodies) / sizeof(char *) - 1);
    printNamePGM(RTTTLMelodies[tRandomIndex]);
    playRtttlBlockingPGM(aTonePin, RTTTLMelodies[tRandomIndex]);
}

/*
 * Plays one of the songs in the array specified non blocking. Ie. you must call updatePlayRtttl() in your loop or use the callback function.
 * aNumberOfEntriesInSongArrayPGM is (sizeof(<MyArrayName>) / sizeof(char *) - 1)
 */
void startPlayRandomRtttlFromArrayPGM(uint8_t aTonePin, const char * const aSongArrayPGM[], uint8_t aNumberOfEntriesInSongArrayPGM,
        char* aBufferPointer, uint8_t aBufferPointerSize, void (*aOnComplete)()) {
    uint8_t tRandomIndex = random(0, aNumberOfEntriesInSongArrayPGM - 1);
    const char* tSongPtr = (char*) pgm_read_word(&aSongArrayPGM[tRandomIndex]);
    startPlayRtttlPGM(aTonePin, tSongPtr, aOnComplete);
    if (aBufferPointer != NULL) {
// copy title to buffer
        getRtttlNamePGM(tSongPtr, aBufferPointer, aBufferPointerSize);
    } else {
// print title
        printNamePGM(tSongPtr);
    }
}

void setTonePinIsInverted(bool aTonePinIsInverted) {
    sPlayRtttlState.Flags.IsTonePinInverted = aTonePinIsInverted;
}
