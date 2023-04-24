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
    
    int midiNote = -1;
    
    WavetableOscillator(Parameter *parameter);
    WavetableOscillator() : state{ State::SLEEP } {}  // needed by list internals
    ~WavetableOscillator();
    
    // Initializer
    void prepareToPlay(float sampleRate);
    
    list<cfloat> waveTable;
    inline list<cfloat> getWavetable() { return waveTable; }
    
    // MIDI
    void noteOn(int midiNote, int velocity);
    void noteOff();
    
    inline bool isPlaying() { return state != State::SLEEP; }
    inline bool isNoteOn() { return state != State::SLEEP && state != State::SUSTAIN; }
    
    float getPhase();
    float getNextSample();
    
private:
    float sampleRate;
    
    Parameter *parameter = nullptr;
    bool showFFT = false;  // mirrors parameter on start, stays the same while playing
    
    State state;
    float envelopeLevel = 0;
    float velocityLevel = 0;
    
    float oldFrequency = 0;
    float targetFrequency = 0;
    float playingFrequency = 0;
    float phase = 0;
    float phaseIncrement = 0;
    
    // Filter
    SingleThreadedIIRFilter* filter;

    // Schrödinger
    double timestepCounter = 0;
    double timestepCountTo = 0;
    void doTimestep(const float dt);
    inline float potential(const size_t x);
    inline cvec fft(cvec in, bool forward);

    void updateState();
};
