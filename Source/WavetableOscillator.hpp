//
//  WavetableOscillator.hpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//

#pragma once

#include <stdio.h>
#include <complex>
#include "list.hpp"
#include "Parameter.h"
#include <vector>
typedef std::vector<cfloat> cvec;

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
    WavetableOscillator() : state{ State::SLEEP } {}  // needed by list internals
    
    // Initializer
    void prepareToPlay(int midiNote, float sampleRate);
    
    list<cfloat> waveTable;
    inline list<cfloat> getWavetable() { return waveTable; }
    
    // MIDI
    void noteOn(int velocity);
    void noteOff();
    
    inline bool isPlaying() { return state != State::SLEEP; }

    float getPhase();
    float getNextSample();

private:
    Parameter *parameter = nullptr;
    bool showFFT = false;  // mirrors parameter on start, stays the same while playing
    
    State state;
    float envelopeLevel = 0;
    float velocityLevel = 0;

    float phase = 0;
    float phaseIncrement = 0;

    // Schrödinger
    double timestepCounter = 0;
    double timestepCountTo = 0;
    void doTimestep(const float dt);
    inline float potential(const size_t x);
    inline cvec fft(cvec in, bool forward);

    void updateState();
};
