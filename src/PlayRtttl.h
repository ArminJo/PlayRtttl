/*
 * PlayRtttl.h
 *
 * Includes 21 sample melodies.
 * The example melodies may have copyrights you have to respect.
 * More RTTTL songs can be found under http://www.picaxe.com/RTTTL-Ringtones-for-Tune-Command/
 *
 *  Copyright (C) 2018  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *     Based on the RTTTL.pde example code written by Brett Hagman
 *     http://www.roguerobotics.com/
 *     bhagman@roguerobotics.com
 *
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
 *
 */

#ifndef SRC_PLAYRTTTL_H_
#define SRC_PLAYRTTTL_H_

#include <avr/pgmspace.h>
#include "pitches.h"

#define DEFAULT_DURATION 4
#define DEFAULT_OCTAVE 6
#define DEFAULT_BPM 63

void setTonePinIsInverted(bool aTonePinIsInverted);
void startPlayRtttl(uint8_t aTonePin, char *aRTTTLArrayPtr, void (*aOnComplete)()=NULL);
void playRtttlBlocking(uint8_t aTonePin, char *aRTTTLArrayPtr);

void startPlayRtttlPGM(uint8_t aTonePin, const char *aRTTTLArrayPtrPGM, void (*aOnComplete)()=NULL);
void playRtttlBlockingPGM(uint8_t aTonePin, const char *aRTTTLArrayPtrPGM);

void startPlayRandomRtttlFromArrayPGM(uint8_t aTonePin, const char* const aSongArrayPGM[], uint8_t aNumberOfEntriesInSongArrayPGM,
        char* aBufferPointer = NULL, uint8_t aBufferPointerSize = 0, void (*aOnComplete)()=NULL);
void playRandomRtttlBlocking(uint8_t aTonePin);

// To be called from loop. - Returns true if tone is playing, false if tone has ended or stopped
bool updatePlayRtttl(void);

void stopPlayRtttl(void);

void getRtttlNamePGM(const char *aRTTTLArrayPtrPGM, char * aBuffer, uint8_t aBuffersize);
void getRtttlName(char *aRTTTLArrayPtr, char * aBuffer, uint8_t aBuffersize);

void printNamePGM(const char *aRTTTLArrayPtrPGM);

struct playRtttlState {
    long MillisOfNextAction;
    const char * NextTonePointer;

    struct {
        uint8_t IsStopped :1;
        uint8_t IsPGMMemory :1;
        uint8_t IsTonePinInverted :1; // True if tone pin has inverted logic i.e. is active on low.
    } Flags;

    // Tone pin to use for output
    uint8_t TonePin;
    // Callback on completion of tone
    void (*OnComplete)(void);

    uint8_t DefaultDuration;
    uint8_t DefaultOctave;
    long TimeForWholeNoteMillis;
};

/*
 * RTTTL format:
 *  opt duration
 *  note
 *  opt dot to increase duration by half
 *  opt octave
 */
/*
 * Disclaimer: These ringtone melodies are for personal enjoyment only. All copyright belongs to its respective author.
 */
static const char StarWars[] PROGMEM
        = "StarWars:d=4,o=5,b=45:32p,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#.6,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#6";
static const char MahnaMahna[] PROGMEM
        = "MahnaMahna:d=16,o=6,b=125:c#,c.,b5,8a#.5,8f.,4g#,a#,g.,4d#,8p,c#,c.,b5,8a#.5,8f.,g#.,8a#.,4g,8p,c#,c.,b5,8a#.5,8f.,4g#,f,g.,8d#.,f,g.,8d#.,f,8g,8d#.,f,8g,d#,8c,a#5,8d#.,8d#.,4d#,8d#.";
static const char MissionImp[] PROGMEM
        = "MissionImp:d=16,o=6,b=95:32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d,32d,32d#,32e,32f,32f#,32g,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,a#,g,2d,32p,a#,g,2c#,32p,a#,g,2c,a#5,8c,2p,32p,a#5,g5,2f#,32p,a#5,g5,2f,32p,a#5,g5,2e,d#,8d";
static const char Entertainer[] PROGMEM
        = "Entertainer:d=4,o=5,b=140:8d,8d#,8e,c6,8e,c6,8e,2c.6,8c6,8d6,8d#6,8e6,8c6,8d6,e6,8b,d6,2c6,p,8d,8d#,8e,c6,8e,c6,8e,2c.6,8p,8a,8g,8f#,8a,8c6,e6,8d6,8c6,8a,2d6";
static const char Muppets[] PROGMEM
        = "Muppets:d=4,o=5,b=250:c6,c6,a,b,8a,b,g,p,c6,c6,a,8b,8a,8p,g.,p,e,e,g,f,8e,f,8c6,8c,8d,e,8e,8e,8p,8e,g,2p,c6,c6,a,b,8a,b,g,p,c6,c6,a,8b,a,g.,p,e,e,g,f,8e,f,8c6,8c,8d,e,8e,d,8d,c";
static const char Flinstones[] PROGMEM
        = "Flinstones:d=4,o=5,b=40:32p,16f6,16a#,16a#6,32g6,16f6,16a#.,16f6,32d#6,32d6,32d6,32d#6,32f6,16a#,16c6,d6,16f6,16a#.,16a#6,32g6,16f6,16a#.,32f6,32f6,32d#6,32d6,32d6,32d#6,32f6,16a#,16c6,a#,16a6,16d.6,16a#6,32a6,32a6,32g6,32f#6,32a6,8g6,16g6,16c.6,32a6,32a6,32g6,32g6,32f6,32e6,32g6,8f6,16f6,16a#.,16a#6,32g6,16f6,16a#.,16f6,32d#6,32d6,32d6,32d#6,32f6,16a#,16c.6,32d6,32d#6,32f6,16a#,16c.6,32d6,32d#6,32f6,16a#6,16c7,8a#.6";
static const char YMCA[] PROGMEM
        ="YMCA:d=4,o=5,b=160:8c#6,8a#,2p,8a#,8g#,8f#,8g#,8a#,c#6,8a#,c#6,8d#6,8a#,2p,8a#,8g#,8f#,8g#,8a#,c#6,8a#,c#6,8d#6,8b,2p,8b,8a#,8g#,8a#,8b,d#6,8f#6,d#6,f.6,d#.6,c#.6,b.,a#,g#";
static const char Simpsons[] PROGMEM
= "The Simpsons:d=4,o=5,b=160:c.6,e6,f#6,8a6,g.6,e6,c6,8a,8f#,8f#,8f#,2g,8p,8p,8f#,8f#,8f#,8g,a#.,8c6,8c6,8c6,c6";
static const char Indiana[] PROGMEM
        = "Indiana:d=4,o=5,b=250:e,8p,8f,8g,8p,1c6,8p.,d,8p,8e,1f,p.,g,8p,8a,8b,8p,1f6,p,a,8p,8b,2c6,2d6,2e6,e,8p,8f,8g,8p,1c6,p,d6,8p,8e6,1f.6,g,8p,8g,e.6,8p,d6,8p,8g,e.6,8p,d6,8p,8g,f.6,8p,e6,8p,8d6,2c6";
static const char TakeOnMe[] PROGMEM
        = "TakeOnMe:d=4,o=4,b=160:8f#5,8f#5,8f#5,8d5,8p,8b,8p,8e5,8p,8e5,8p,8e5,8g#5,8g#5,8a5,8b5,8a5,8a5,8a5,8e5,8p,8d5,8p,8f#5,8p,8f#5,8p,8f#5,8e5,8e5,8f#5,8e5,8f#5,8f#5,8f#5,8d5,8p,8b,8p,8e5,8p,8e5,8p,8e5,8g#5,8g#5,8a5,8b5,8a5,8a5,8a5,8e5,8p,8d5,8p,8f#5,8p,8f#5,8p,8f#5,8e5,8e5";
static const char Looney[] PROGMEM
= "Looney:d=4,o=5,b=140:32p,c6,8f6,8e6,8d6,8c6,a.,8c6,8f6,8e6,8d6,8d#6,e.6,8e6,8e6,8c6,8d6,8c6,8e6,8c6,8d6,8a,8c6,8g,8a#,8a,8f";
static const char _20thCenFox[] PROGMEM
        = "20thCenFox:d=16,o=5,b=140:b,8p,b,b,2b,p,c6,32p,b,32p,c6,32p,b,32p,c6,32p,b,8p,b,b,b,32p,b,32p,b,32p,b,32p,b,32p,b,32p,b,32p,g#,32p,a,32p,b,8p,b,b,2b,4p,8e,8g#,8b,1c#6,8f#,8a,8c#6,1e6,8a,8c#6,8e6,1e6,8b,8g#,8a,2b";
static const char Bond[] PROGMEM
        = "Bond:d=4,o=5,b=80:32p,16c#6,32d#6,32d#6,16d#6,8d#6,16c#6,16c#6,16c#6,16c#6,32e6,32e6,16e6,8e6,16d#6,16d#6,16d#6,16c#6,32d#6,32d#6,16d#6,8d#6,16c#6,16c#6,16c#6,16c#6,32e6,32e6,16e6,8e6,16d#6,16d6,16c#6,16c#7,c.7,16g#6,16f#6,g#.6";
static const char GoodBad[] PROGMEM
        = "GoodBad:d=4,o=5,b=56:32p,32a#,32d#6,32a#,32d#6,8a#.,16f#.,16g#.,d#,32a#,32d#6,32a#,32d#6,8a#.,16f#.,16g#.,c#6,32a#,32d#6,32a#,32d#6,8a#.,16f#.,32f.,32d#.,c#,32a#,32d#6,32a#,32d#6,8a#.,16g#.,d#";
static const char PinkPanther[] PROGMEM
= "PinkPanther:d=4,o=5,b=160:8d#,8e,2p,8f#,8g,2p,8d#,8e,16p,8f#,8g,16p,8c6,8b,16p,8d#,8e,16p,8b,2a#,2p,16a,16g,16e,16d,2e";
static const char A_Team[] PROGMEM
= "A-Team:d=8,o=5,b=125:4d#6,a#,2d#6,16p,g#,4a#,4d#.,p,16g,16a#,d#6,a#,f6,2d#6,16p,c#.6,16c6,16a#,g#.,2a#";
static const char Jeopardy[] PROGMEM
        = "Jeopardy:d=4,o=6,b=125:c,f,c,f5,c,f,2c,c,f,c,f,a.,8g,8f,8e,8d,8c#,c,f,c,f5,c,f,2c,f.,8d,c,a#5,a5,g5,f5,p,d#,g#,d#,g#5,d#,g#,2d#,d#,g#,d#,g#,c.7,8a#,8g#,8g,8f,8e,d#,g#,d#,g#5,d#,g#,2d#,g#.,8f,d#,c#,c,p,a#5,p,g#.5,d#,g#";
static const char Gadget[] PROGMEM
= "Gadget:d=16,o=5,b=50:32d#,32f,32f#,32g#,a#,f#,a,f,g#,f#,32d#,32f,32f#,32g#,a#,d#6,4d6,32d#,32f,32f#,32g#,a#,f#,a,f,g#,f#,8d#";
static const char Smurfs[] PROGMEM
        = "Smurfs:d=32,o=5,b=200:4c#6,16p,4f#6,p,16c#6,p,8d#6,p,8b,p,4g#,16p,4c#6,p,16a#,p,8f#,p,8a#,p,4g#,4p,g#,p,a#,p,b,p,c6,p,4c#6,16p,4f#6,p,16c#6,p,8d#6,p,8b,p,4g#,16p,4c#6,p,16a#,p,8b,p,8f,p,4f#";
static const char LeisureSuit[] PROGMEM
        = "LeisureSuit:d=16,o=6,b=56:f.5,f#.5,g.5,g#5,32a#5,f5,g#.5,a#.5,32f5,g#5,32a#5,g#5,8c#.,a#5,32c#,a5,a#.5,c#.,32a5,a#5,32c#,d#,8e,c#.,f.,f.,f.,f.,f,32e,d#,8d,a#.5,e,32f,e,32f,c#,d#.,c#";
static const char Toccata[] PROGMEM
="Toccata:d=4,o=5,b=160:16a4,16g4,1a4,16g4,16f4,16d4,16e4,2c#4,16p,d.4,2p,16a4,16g4,1a4,8e.4,8f.4,8c#.4,2d4";
/*
 * Array of songs. Useful for random melody
 */
#pragma GCC diagnostic ignored "-Wunused-variable"
static const char * const RTTTLMelodies[] PROGMEM = { StarWars, MahnaMahna, LeisureSuit, MissionImp, Flinstones, YMCA, Simpsons,
        Indiana, TakeOnMe, Entertainer, Muppets, Looney, _20thCenFox, Bond, StarWars, GoodBad, PinkPanther, A_Team, Jeopardy,
        Gadget, Smurfs, Toccata };
#define ARRAY_SIZE_MELODIES (sizeof(RTTTLMelodies)/sizeof(const char *))

static const char JingleBell[] PROGMEM
        = "JingleBell:d=8,o=5,b=112:a,a,4a,a,a,4a,a,c6,f.,16g,2a,a#,a#,a#.,16a#,a#,a,a.,16a,a,g,g,a,4g,4c6,16p,a,a,4a,a,a,4a,a,c6,f.,16g,2a,a#,a#,a#.,16a#,a#,a,a.,16a,c6,c6,a#,g,2f";
static const char Rudolph[] PROGMEM
        = "Rudolph:d=16,o=6,b=100:32p,g#5,8a#5,g#5,8f5,8c#,8a#5,4g#.5,g#5,a#5,g#5,a#5,8g#5,8c#,2c,f#5,8g#5,f#5,8d#5,8c,8a#5,4g#.5,g#5,a#5,g#5,a#5,8g#5,8a#5,2f5,g#5,8a#5,a#5,8f5,8c#,8a#5,4g#.5,g#5,a#5,g#5,a#5,8g#5,8c#,2c,f#5,8g#5,f#5,8d#5,8c,8a#5,4g#.5,g#5,a#5,g#5,a#5,8g#5,8d#,2c#";
static const char WeWishYou[] PROGMEM
        = "WeWishYou:d=4,o=5,b=200:d,g,8g,8a,8g,8f#,e,e,e,a,8a,8b,8a,8g,f#,d,d,b,8b,8c6,8b,8a,g,e,d,e,a,f#,2g,d,g,8g,8a,8g,8f#,e,e,e,a,8a,8b,8a,8g,f#,d,d,b,8b,8c6,8b,8a,g,e,d,e,a,f#,1g,d,g,g,g,2f#,f#,g,f#,e,2d,a,b,8a,8a,8g,8g,d6,d,d,e,a,f#,2g";
static const char WinterWonderland[] PROGMEM
        = "WinterWonderland:d=16,o=5,b=112:8a#.,a#,2a#.,8a#.,a#,4g,2a#,8a#.,a#,2a#.,8a#.,a#,4g#,2a#,8p,a#,8d.6,d6,8d.6,4c.6,8p,c6,8a#.,a#,8a#.,4g#.,8p,g#,8g.,g,8g.,g,8f.,f,8f.,f,2d#,4p,8a#.,a#,2a#.,8a#.,a#,4g,2a#,8a#.,a#,2a#.,8a#.,a#,4g#,2a#,8p,a#,8d.6,d6,8d.6,4c.6,8p,c6,8a#.,a#,8a#.,4g#.,8p,g#,8g.,g,8g.,g,8f.,f,8f.,f,2d#,4p,8d.,d,8b.,b,8e.,e,8c.6,c6,4b,2g,4p,8d.,d,8b.,b,8e.,e,8c.6,c6,2b.";
static const char OhDennenboom[] PROGMEM
        = "OhDennenboom:d=4,o=6,b=100:8c5,8f.5,16f5,f.5,8g5,8a.5,16a5,a5,8p,8a5,8g5,8a5,a_5,e5,g5,f.5,8c5,8f.5,16f5,f.5,8g5,8a.5,16a5,a5,8p,8a5,8g5,8a5,a_5,e5,g5,f.5,8c,8c,8a5,d.,8c,8c,8a_5,a_.5,8a_5,8a_5,8g5,c.,8a_5,8a_5,8a5,a.5,8c5,8f.5,16f5,f.5,8g5,8a.5,16a5,a5,8p,8a5,8g5,8a5,a_5,e5,g5,2f5";
static const char LetItSnow[] PROGMEM
        = "LetItSnow:d=4,o=5,b=125:8c,8c,8c6,8c6,a#,a,g,f,2c,8c,16c,g.,8f,g.,8f,e,2c,d,8d6,8d6,c6,a#,a,2g.,8e.6,16d6,c6,8c.6,16a#,a,8a#.,16a,2f.,c,8c6,8c6,a#,a,g,f,2c,8c.,16c,g.,8f,g.,8f,e,2c,d,8d6,8d6,c6,a#,a,2g.,8e.6,16d6,c6,8c.6,16a#,a,8a.,16g,2f.";
static const char Frosty[] PROGMEM
= "Frosty:d=4,o=5,b=125:2g,e.,8f,g,2c6,8h,8c6,d6,c6,h,a,2g.,8h,8c6,d6,c6,h,8a,8a,g,c6,e,8g,8a,g,f,e,f,1g";
static const char SilentNight[] PROGMEM
        = "SilentNight:d=4,o=5,b=112:g.,8a,g,2e.,g.,8a,g,2e.,2d6,d6,2b.,2c6,c6,2g.,2a,a,c6.,8b,a,g.,8a,g,2e.,2a,a,c6.,8b,a,g.,8a,g,2e.,2d6,d6,f6.,8d6,b,2c6.,2e6.,c6,g,e,g.,8f,d,2c.";
static const char LastChristmas[] PROGMEM
        = "LastChristmas:d=4,o=6,b=112:g,16f5,16p,8g,16f5,16p,8f,16f5,16p,8c,8g,8g,8a,f,16f5,16p,16f5,16p,8c,8g,8g,8a,16d5,16p,f,16d5,16p,8f,8e,8f,8e,d,16d5,16p,16d5,16p,16d5,16p,a,16g5,16p,8a,16g5,16p,g,8d,8a,8a,8a_,g,16g5,16p,16g5,16p,8f,8e,8f,16c5,16p,8e,16c5,16p,8f,16c5,16p,8e,16c5,16p,c,16c5,16p,16c5,16p,16c5,16p,16c5,16p,16d5,16p,8f5";
static const char AllIWant[] PROGMEM
        = "AllIWant:d=4,o=6,b=160:c5,e5,g5,8b5,c,b.5,8a5,g.5,d,c,8c,b5,c,b5,8a5,2g5,a5,c,8d,e,d,c,a.5,f5,8g#5,c.,8d,d#,d,a#5,g#.5,c,d,b5,8c,a5,b5,2g#5,c,d,b5,8c,a5,b5,2g#5,g5,a5,8c,g,f,8g,2f,e,d,c,a5,g#5,2d,e,8d.,2c.";
static const char AmazingGrace[] PROGMEM = "AmazingGrace:d=8,o=5,b=80:c,f,2f,a,g,f,2a,a,g,2f,4d,2c,c,f,2f,a,g,f,2a,g,a,2c.6";
/*
 * Array of Christmas songs. Useful for random melody
 */
static const char * const RTTTLChristmasMelodies[] PROGMEM = { JingleBell, Rudolph, OhDennenboom, SilentNight, WeWishYou,
        WinterWonderland, LetItSnow, Frosty, LastChristmas, AllIWant, AmazingGrace };
#define ARRAY_SIZE_CHRISTMAS_SONGS (sizeof(RTTTLChristmasMelodies)/sizeof(const char *))

#endif /* SRC_PLAYRTTTL_H_ */
