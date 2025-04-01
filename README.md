# FSCE: fun and simple coding examples..

This is a simple repo to demonstrate some fundamental concepts, and a simple way for a Dad to get involved in his teenage kids' homework.  ;)

In our scenario, there is a) an Arduino-based project with workshop code from school, and b) Darwin/Linux-based workstations to use for other purposes.

The idea is to share code between both an Arduino firmware, and a simple C-based binary build product.

We will demonstrate how to write C code that will produce:

- Arduino project code suitable to use in school workshops
- Simulator code, suitable to use in learning C

Two areas of the project will function in parallel, the Arduino school project code, and the 'test/hack/experiment' playground in the C-based binary environment.

This will provide cross-platform build scaffolding methods and other tooling for us to construct other things.

## Projects:

# trafficlight 

A traffic-light simulator project intended to introduce the idea of a basic state machine as it might work in a traffic control system.

# voiceTracker

A simple implementation of a synth voice-assignment tracker, intended for use in simple MIDI-driven synthesizers which need to keep track of a fixed number of voices in, e.g. a polyphonic synthesizer context.

- functions_named_like_this are utility/test/stub functions.
- functionsNamedLikeThis are implementation functions.

'make test' to see two test cases, first with fixed note assignments and then with random assignments.  Run voice_tracker manually to have a non-sorted test output.

There's a .lua implementation too, just for fun
