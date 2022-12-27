//
//  WavetableOscillator.cpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//


#include "WavetableOscillator.hpp"
#include <cmath>
#include "Wavetable.hpp"

#define POCKETFFT_CACHE_SIZE 1
#define POCKETFFT_NO_MULTITHREADING
#include "pocketfft_hdronly.h"

// Sowas geht
//namespace wvt = wavetable;

WavetableOscillator::WavetableOscillator(Parameter *parameter) 
    : parameter{ parameter }
    , state{ State::SLEEP }
{
}

void WavetableOscillator::prepareToPlay(int midiNote, float sampleRate)
{
    phaseIncrement = Wavetable::midiNoteToIncrement(midiNote, sampleRate);
    envelopeLevel = 0.f;
}

void WavetableOscillator::noteOn(int velocity) {
    waveTable = Wavetable::generate(0, 0, 0);
    
    // Do not set envelopeLevel = 0 here, to enable smooth retriggers of one note
    
    state = State::ATTACK;
    velocityLevel = velocity / 127.f; // TODO: Rethink velocity sensitivity
    
    phase = 0.f; // Not necessary for the sound, but helpful for null-tests
    // phaseIncrement is set by prepareToPlay
}

void WavetableOscillator::noteOff() {
    state = State::RELEASE;
}

float WavetableOscillator::getNextSample()
{
    const auto sample = 
          envelopeLevel 
        * velocityLevel 
        * waveTable.getLinearInterpolation(phase, getSampleConversion(parameter->sampleType));
    
    phase = std::fmod(phase + phaseIncrement, Wavetable::SIZE_F);

    updateState();
    
    return sample;
}

void WavetableOscillator::updateState() {
    switch (state) {
        case State::SLEEP:
            // Noch weniger als bei Sustain hehe
            break;
            
        case State::ATTACK:
            envelopeLevel += parameter->attackFactor * (1 - envelopeLevel);
            if (envelopeLevel > Parameter::ATTACK_THRESHOLD) state = State::DECAY;
            break;
            
        case State::DECAY:
        {
            float difference = (envelopeLevel - parameter->sustainLevel);
            envelopeLevel -= parameter->decayFactor * difference;
            if (difference < Parameter::DECAY_THRESHOLD) state = State::SUSTAIN;
        } break;
        
        case State::SUSTAIN:
            // Nothing hehe
            break;
        
        case State::RELEASE:
            envelopeLevel *= parameter->releaseFactor;
            // Is done playing?
            // Make threshold even smaller to keep playing super quietly after the set release time
            if (envelopeLevel < (Parameter::RELEASE_THRESHOLD * 0.01f)) {
                state = State::SLEEP;
                envelopeLevel = 0;
            }

    }
}
