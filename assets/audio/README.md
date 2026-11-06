# Game audio

These 25 MP3 files were copied from the supplied `audio aseets` folder. The
original files are unchanged. `Audio.cpp` maps each typed `SoundId` to a filename;
gameplay modules use IDs rather than filesystem paths.

Music starts at 35% and effects at 75%. The main menu and pause menu offer
independent sliders and mute buttons. Drag a slider, or press Tab to focus a
control, Left/Right to adjust by 5%, Home/End to set 0/100%, and Space/Enter to
toggle a focused mute button. Unmuting restores the last nonzero value. Settings
last for the session, including restarts and menu changes.

Menu and gameplay music loop. Intro, victory and defeat tracks play once.
Skipping the intro stops its track. Pause freezes gameplay music and combat
effects, while menu feedback remains audible. Resume continues from the same
position; Restart starts the gameplay track over. Leaving gameplay clears old
combat effects. Sound hooks run when actions occur, never during rendering.

The mixer predecodes short effects, streams music, and limits playback to 24
effect voices (four reserved for UI feedback). Repeated effects are coalesced
within 80 ms; the insect-approaching cue uses a 750 ms cooldown. Missing or invalid
assets are logged and skipped; a failed audio-device initialization leaves the
game playable silently.

A soft limiter leaves normal levels unchanged and smoothly compresses peaks
above 80% of full scale so overlapping combat effects do not hard-clip.

CMake copies this folder into `assets/audio` beside the game executable. Playback
looks there first, then in the source asset directory for development. The
packaging step also runs on subsequent default builds after replacing files.

## Dependency

`external/miniaudio.h` is miniaudio **0.11.23**, pinned to upstream commit
`f40cf03f80cdb7e741d43e53b7e706e8c1394bcf`. Its upstream license is included in
`external/miniaudio.LICENSE`. It is compiled once by the `GameAudio` target; no
external player or MP3 codec installation is required.

## Verification

Configure with `-DBUILD_AUDIO_TESTS=ON`, build, then run:

```sh
ctest --test-dir build-audio --output-on-failure -R '^Audio'
```

`AudioEvents` checks state changes, real action hooks, volume controls, cooldowns,
bounded overlapping effects, corrupt-file fallback, and actual offline PCM
rendering. It checks that pause preserves the exact playback cursor and restart
seeks to the beginning, without opening a speaker device. `AudioAssets` decodes
all 25 MP3s fully. `AudioControlsRender` checks production callbacks, visible
keyboard focus, and hidden-window rendering at seven sizes, including portrait
and 320×240. Its captures are written under the build's `audio-captures` folder.
`AudioPackagedPlayback` checks the copies beside the executable and renders them
from a different working directory.

`AudioConcurrentLifecycle` runs 500 pause/restart/menu cycles while a separate
thread mixes actual PCM into memory, without using the speakers. The controls
render test also verifies that a rapid double-click on Main Menu cannot activate
Exit on the next screen; a deliberate Exit works after the 500 ms guard interval.
