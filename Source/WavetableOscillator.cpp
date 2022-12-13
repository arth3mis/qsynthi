//
//  WavetableOscillator.cpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//

#include "WavetableOscillator.hpp"
#include <cmath>

constexpr auto A4_FREQUENCY = 440.f;
constexpr auto A4_NOTE_NUMBER = 69.f;
constexpr auto SEMITONES_IN_AN_OCTAVE = 12.f;

WavetableOscillator::WavetableOscillator(std::vector<float> waveTable, int midiNote, double sampleRate) :
    waveTable{ std::move(waveTable) },
    indexIncrement{  },
    sampleRate{ sampleRate }
{
    
}

float WavetableOscillator::getNextSample()
{
    const auto sample = interpolateLinearly();
    index = std::fmod(index + indexIncrement, static_cast<float>(waveTable.size()));
    
    return sample;
}

float WavetableOscillator::interpolateLinearly()
{
    const auto truncatedIndex = static_cast<int>(index);
    const auto nextIndex = (truncatedIndex + 1) % static_cast<int>(waveTable.size());
    
    const auto nextIndexWeight = index - static_cast<float>(truncatedIndex);
    const auto truncatedIndexWeight = 1.f - nextIndexWeight;
    
    return truncatedIndexWeight * waveTable[truncatedIndex] + nextIndexWeight * waveTable[nextIndex];
}

void WavetableOscillator::stop()
{
    index = 0.f;
    indexIncrement = 0.f;
}


float WavetableOscillator::midiNoteNumberToIncrement(int midiNoteNumber)
{
    const float frequency = A4_FREQUENCY * std::powf(2.f, (midiNoteNumber - A4_NOTE_NUMBER) / SEMITONES_IN_AN_OCTAVE);
    
    return frequency * (float)(waveTable.size() / sampleRate);
}
