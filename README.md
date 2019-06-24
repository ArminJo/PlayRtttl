# PlayRtttl

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Build Status](https://travis-ci.org/ArminJo/PlayRtttl.svg?branch=master)](https://travis-ci.org/ArminJo/PlayRtttl)

Improved Arduino library version of the RTTTL.pde example code written by Brett Hagman http://www.roguerobotics.com/  bhagman@roguerobotics.com
 - Plays RTTTL melodies/ringtones from FLASH or RAM.
 - Non blocking version.
 - Name output function.
 - Sample melodies.
 - Random play of melodies from array.
 - Supports inverted tone pin logic i.e. tone pin is HIGH at playing a pause.
 - Accepts even invalid specified RTTTL files found in the wild.
 - Supports RTX format - 2 additional parameters: 1. l=<number_of_loops> 2.s=<Style[N|S|C]>).
 - Tone style (relation of tone output to note length) and loop count can be set for a melody.

Youtube video of the RandomMelody example in action

[![RandomMelody example](https://i.ytimg.com/vi/0n9_Fm3VP3w/hqdefault.jpg)](https://www.youtube.com/watch?v=0n9_Fm3VP3w)

# Sample code
## Blocking play melody from FLASH
```
#include <PlayRtttl.h>
const int TONE_PIN = 11;
...
    playRtttlBlockingPGM(TONE_PIN, Bond);
...

```
## Non blocking play

```
...
    startPlayRtttlPGM(TONE_PIN, TakeOnMe);
    while (updatePlayRtttl()) {
        // your own code here...
        delay(1);
    }
...
```

## Running with 1MHz
If running with 1MHz, e.g on an ATtiny, the millis() interrupt needs so much time, that it disturbes the tone() generation by interrupt. You can avoid this by using a tone pin, which is directly supported by hardware. Look at the appropriate pins_arduino.h, find `digital_pin_to_timer_PGM[]` and choose pins with TIMER1x entries.

## More songs
More RTTTL songs can be found under http://www.picaxe.com/RTTTL-Ringtones-for-Tune-Command/ or ask Google.
[C array of songs on GitHub](https://github.com/granadaxronos/120-SONG_NOKIA_RTTTL_RINGTONE_PLAYER_FOR_ARDUINO_UNO/blob/master/RTTTL_PLAYER/songs.h)

# Revision History
### Version 1.2.2
 - Tested with ATtiny85 and 167
 - Ported to non AVR architectures
### Version 1.2.1
 - Natural is the new default style.
 - New RTTTLMelodiesSmall sample array with less entries.
 - Parameter now order independent.
 - Modified oneMelody example.
### Version 1.2.0
No Serial.print statements in this library anymore, to avoid problems with different Serial implementations.
 - Function playRandomRtttlBlocking() + startPlayRandomRtttlFromArrayPGM() do not print name now. If needed, use new functions playRandomRtttlSampleBlockingAndPrintName() + startPlayRandomRtttlFromArrayPGMAndPrintName().
- Printing functions have parameter (..., Stream * aSerial) to print to any serial. Call it (..., &Serial) to use standard Serial;
- playRandomRtttlBlocking() renamed to playRandomRtttlSampleBlocking() and bug fixing.
### Version 1.1.0
- RTX song format support.
- new setNumberOfLoops() and setDefaultStyle() functions.

### Version 1.0.0
Initial Arduino library version

# Travis CI
The ServoEasing library examples are built on Travis CI for the following boards:

- Arduino Uno, Leonardo, mega2560, esp32, esp8266, Adafruit cplayClassic

## Requests for modifications / extensions
Please write me a PM including your motivation/problem if you need a modification or an extension.

#### If you find this library useful, please give it a star.
