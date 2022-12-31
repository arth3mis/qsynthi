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

typedef std::complex<float> cfloat;
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
    
    // MIDI
    void noteOn(int velocity);
    void noteOff();

    inline list<cfloat> getWavetable() { return waveTable; }
    inline std::function<float(cfloat)> getConverter() { return getSampleConversion(parameter->sampleType); }
    
    // Important Components of                                          ja was
    inline bool isPlaying() { return state != State::SLEEP; }

    float getNextSample();
    
private:
    Parameter *parameter = nullptr;
    list<cfloat> waveTable;

    inline std::function<float(cfloat)> getSampleConversion(const SampleType type);
    
    State state;
    float envelopeLevel = 0;
    float velocityLevel = 0;

    float phase = 0;
    float phaseIncrement = 0;

    // Schrödinger
    double timestepCounter = 0;
    double timestepCountTo = 0;
    void doTimestep();
    inline float potential(const float x);
    inline cvec fft(cvec in);

    void updateState();

};
