//
//  WavetableOscillator.cpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//

#include "WavetableOscillator.hpp"
#include <cmath>
#include "Wavetables.hpp"

// Sowas geht
//namespace wvt = wavetable;

constexpr auto A4_FREQUENCY = 440.f;
constexpr auto A4_NOTE_NUMBER = 69.f;
constexpr auto SEMITONES_IN_AN_OCTAVE = 12.f;


WavetableOscillator::WavetableOscillator(int waveType, float waveShift, float waveScale, int midiNote, float sampleRate) :
    waveTable{ Wavetable::generate(waveType, waveShift, waveScale) },
    phaseIncrement{ Wavetable::midiNoteToIncrement(midiNote, sampleRate) }
{
}

float WavetableOscillator::getNextSample()
{
    const auto sample = waveTable.getLinearInterpolation(phase);
    phase = std::fmod(phase + phaseIncrement, Wavetable::SIZE_F);
    
    return sample;
}


void WavetableOscillator::noteOff()
{
}

bool WavetableOscillator::isDone() const
{
    return true;
}
