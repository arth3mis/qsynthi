//
//  WavetableOscillator.cpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//

#include "WavetableOscillator.hpp"
#include <cmath>
#include "Wavetables.h"
// Sowas geht
//namespace wvt = wavetable;

constexpr auto A4_FREQUENCY = 440.f;
constexpr auto A4_NOTE_NUMBER = 69.f;
constexpr auto SEMITONES_IN_AN_OCTAVE = 12.f;


WavetableOscillator::WavetableOscillator(int waveType, float waveShift, float waveScale, int midiNote, float sampleRate) :
    waveTable{ wavetable::generate(waveType, waveShift, waveScale) },
    phaseIncrement{ wavetable::midiNoteToIncrement(midiNote, sampleRate) }
{
    
}

inline float WavetableOscillator::getNextSample()
{
    const auto sample = waveTable.getLinearInterpolation(phase);
    phase = std::fmod(phase + phaseIncrement, wavetable::SIZE_F);
    
    return sample;
}
