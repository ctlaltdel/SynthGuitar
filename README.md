# SynthGuitar
A ribbon synthesiser designed to be played like a guitar

The SynthGuitar is a rough-and-ready prototype with a view to creating a more refined, playable instrument in the future. The core of the instrument is a Daisy Seed (https://www.electro-smith.com/daisy/daisy). The current firmware implements a 2-oscillator, 3 voice synthesiser. The voices are activated with touch sensors. the touch sensors are analogous to guitar strings, in that they each have a pitch range associated with them. This pitch range is mapped to a single 500 mm SoftPot that is analogous to a fretless guitar neck. The output pitch(es) are a combination of the finger position along the SoftPot, and the active touch switches.   

## Synth Engine
The firmware is written in C++ using Visual Studio Code, and the synth functions are implemented with the libDaisy and DaisySP libraries. The signal chain is currently:

_Oscillators -> ADSR envelope -> Filter -> Chorus -> Reverb_

The synth parameters are set using potentiometers read via the A/D inputs of the Daisy Seed. 

## The SoftPot

## Acknowledgements
Thanks to YouTuber gr4yhound for the inspiration behind this project, check out his video https://www.youtube.com/watch?v=s3dBox-LB7I
Also thanks to donnerbono for his synth code (https://github.com/donnerbono/bitsandbobs/tree/main/DuoPolySynth%20v1) which enabled me to get the basic synth architecture up and working reasonably quickly.
