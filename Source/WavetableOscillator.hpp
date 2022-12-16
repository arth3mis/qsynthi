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
#include "PluginProcessor.h"

enum State {
    SLEEP,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
};

constexpr float ENVELOPE_THRESHOLD = 0.05f;

class WavetableOscillator
{
public:
    WavetableOscillator(struct Parameter& parameter);
    
    // Initializer
    void prepareToPlay(int midiNote, float sampleRate);
    
    // MIDI
    void noteOn(int velocity);
    void noteOff();
    
    // Important Components of
    bool isPlaying();
    float getNextSample();
    
private:
    struct Parameter parameter;
    list<float> waveTable;
    
    State state;
    float envelopeLevel;
    float velocityLevel;

    float phase;
    float phaseIncrement;

};
