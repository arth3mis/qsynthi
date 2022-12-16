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


bool WavetableOscillator::isPlaying() {
    if (state == State::SLEEP) return true;
    
    // If done with playing
    if (state == State::RELEASE && envelopeLevel < ENVELOPE_THRESHOLD) {
        state = State::SLEEP;
        return true;
    }
}

float WavetableOscillator::getNextSample()
{
    const auto sample = envelopeLevel * velocityLevel * waveTable.getLinearInterpolation(phase);
    
    phase = std::fmod(phase + phaseIncrement, Wavetable::SIZE_F);
    
    return sample;
}
