//
//  WavetableOscillator.hpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//

#pragma once

#include <stdio.h>
#include <vector>
#include "list.hpp"
#include "Parameter.h"

enum class State {
    SLEEP,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
};

class WavetableOscillator
{
public:
    WavetableOscillator(Parameter *parameter);
    WavetableOscillator() {} // muss anscheinend noch
    
    // Initializer
    void prepareToPlay(int midiNote, float sampleRate);
    
    // MIDI
    void noteOn(int velocity);
    void noteOff();
    
    // Important Components of
    inline bool isPlaying() { return state != State::SLEEP; }

    float getNextSample();
    
private:
    Parameter *parameter;
    list<float> waveTable;
    
    State state;
    float envelopeLevel = 0;
    float velocityLevel;

    float phase;
    float phaseIncrement;
    
    void updateState();

};
