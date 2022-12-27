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
    
    // Important Components of
    inline bool isPlaying() { return state != State::SLEEP; }

    float getNextSample();
    
private:
    Parameter *parameter = nullptr;
    list<std::complex<float>> waveTable;

    static inline std::function<float(std::complex<float>)> getSampleConversion(const SampleType type)
    {
        if (type == SampleType::REAL_VALUE)
            return [](std::complex<float> z) { return std::real(z); };
        else if (type == SampleType::IMAG_VALUE)
            return [](std::complex<float> z) { return std::imag(z); };
        else if (type == SampleType::SQARED_ABS)
            return [](std::complex<float> z) { return std::norm(z); };
        else
            return [](std::complex<float> z) { return 0; };
    }
    
    State state;
    float envelopeLevel = 0;
    float velocityLevel = 0;

    float phase = 0;
    float phaseIncrement = 0;

    void updateState();

};
