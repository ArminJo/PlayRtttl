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
 - Suports RTX format.
 - Tone style (relation of tone output to note lenght) and loop count can be set for a melody.

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
## More songs
More RTTTL songs can be found under http://www.picaxe.com/RTTTL-Ringtones-for-Tune-Command/ or ask Google.
[C array of songs on GitHub](https://github.com/granadaxronos/120-SONG_NOKIA_RTTTL_RINGTONE_PLAYER_FOR_ARDUINO_UNO/blob/master/RTTTL_PLAYER/songs.h)

# Revision History

### Version 1.1.0
- RTX song format support.
- new setNumberOfLoops() and setDefaultStyle() functions.

### Version 1.0.0
Initial Arduino library version


## Requests for modifications / extensions
Please write me a PM including your motivation/problem if you need a modification or an extension.
