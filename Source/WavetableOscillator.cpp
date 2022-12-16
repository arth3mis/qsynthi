//
//  WavetableOscillator.cpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//


#include "WavetableOscillator.hpp"
#include <cmath>
#include "Wavetable.hpp"

// Sowas geht
//namespace wvt = wavetable;

WavetableOscillator::WavetableOscillator(struct Parameter& parameter) : parameter{ parameter }
{
    
}

void WavetableOscillator::prepareToPlay(int midiNote, float sampleRate)
{
    phaseIncrement = Wavetable::midiNoteToIncrement(midiNote, sampleRate);
}

void WavetableOscillator::noteOn(int velocity) {
    waveTable = Wavetable::generate(0, 0, 0);
    
    state = State::ATTACK;
    envelopeLevel = 0.f;
    velocityLevel = velocity / 127.f; // TODO: Rethink velocity sensitivity
    
    phase = 0.f; // Not necessary for the sound, but helpful for null-tests
    // phaseIncrement is set by prepareToPlay
}

void WavetableOscillator::noteOff() {
    state = State::RELEASE;
}


inline bool WavetableOscillator::isPlaying() {
    return state == State::SLEEP;
}

float WavetableOscillator::getNextSample()
{
    const auto sample = envelopeLevel * velocityLevel * waveTable.getLinearInterpolation(phase);
    
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
            envelopeLevel += parameter.attackFactor * (1 - envelopeLevel);
            if (envelopeLevel > ATTACK_THRESHOLD) state = State::DECAY;
            break;
            
        case State::DECAY:
            float difference = (envelopeLevel - parameter.sustainLevel);
            envelopeLevel -= parameter.decayFactor * difference;
            if (difference < DECAY_THRESHOLD) state = State::SUSTAIN;
            break;
        
        case State::SUSTAIN:
            // Nothing hehe
            break;
        
        case State::RELEASE:
            envelopeLevel *= parameter.releaseFactor;
            // Is done playing?
            if (envelopeLevel < RELEASE_THRESHOLD) {
                state = State::SLEEP;
                envelopeLevel = 0;
            }

    }
}
