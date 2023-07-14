# SynthGuitar
A ribbon synthesiser designed to be played like a guitar

The SynthGuitar is a rough-and-ready prototype with a view to creating a more refined, playable instrument in the future. The core of the instrument is a Daisy Seed (https://www.electro-smith.com/daisy/daisy). The current firmware implements a 2-oscillator, 3 voice synthesiser. The voices are activated with touch sensors. the touch sensors are analogous to guitar strings, in that they each have a pitch range associated with them. This pitch range is mapped to a single 500 mm SoftPot that is analogous to a fretless guitar neck. The output pitch(es) are a combination of the finger position along the SoftPot, and the active touch switches.   

## Synth Engine
The firmware is written in C++ using Visual Studio Code, and the synth functions are implemented with the libDaisy and DaisySP libraries. The signal chain is currently:

_Oscillators -> ADSR envelope -> Filter -> Chorus -> Reverb_

The synth parameters are set using potentiometers read via the A/D inputs of the Daisy Seed. Controllable parameters on this prototype are:
- Oscillator A waveshape
- Oscillator B waveshape
- Oscillator A/B mix
- Filter cutoff
- Attack/Decay mix
- Dry/Reverb mix
- Chorus Rate
- Chorus Depth

## The SoftPot
The SoftPot is the SP-L-500-203-ST made by Spectra Symbol. It is available from several sources, including Sparkfun and Digikey. The SoftPot is driven by a constant current (rather than the usual constant voltage), which enables the detection of the position of 2 simultaneous presses by measuring the voltage at the supply terminal as well as the wiper terminal. This in turn should allow the firmware to provide the player with the ability to do hammer-ons and pull-offs and make the synth more 'guitarlike'.
The firmware implements an exponential correction of the pitch resistance of the SoftPot so that the semitones are evenly spaced on the 'neck'. This differs from a conventional guitar neck that has the semitones move closer together as they increase in pitch.

## The Touch Sensors
The touch sensors are W7ED-11F made by Omron, available from Digikey. They are open collector output, and connected directly to digital inputs on the Daisy. oThey are active low when touched. They require a an external conductive electrode, I used scraps of single sided PCB material.

## Construction
Since this is a highly experimental prototype, functionality has been prioritised over asthetics or durability. Apart from the electronics, it has been built out of scrap wood, nails and screws, cable ties and a plastic enclosure I had lying around. The instrument is powered by a 12V 1.2Ah SLA battery that feeds a 5V switchmode module, which in turn powers the Daisy Seed. The potentiometers are supplied by the 3.3V output on the Seed. The instrument has a 1/4" jack that allows connection to a guitar amp using a standard instrument cable.

## Current Status
I am currently working on implementing the two-touch detection. After that I would like to have the volume increase with slide velocity as shown here: https://www.youtube.com/watch?v=fh7W3UtsQkw-

## Acknowledgements
Thanks to YouTuber gr4yhound for the inspiration behind this project, check out his video https://www.youtube.com/watch?v=s3dBox-LB7I .
Also thanks to donnerbono for his synth code (https://github.com/donnerbono/bitsandbobs/tree/main/DuoPolySynth%20v1) which enabled me to get the basic synth architecture up and working reasonably quickly.
