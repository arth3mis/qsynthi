//
//  WavetableOscillator.cpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//

#include "WavetableOscillator.hpp"
#include <cmath>


WavetableOscillator::WavetableOscillator(std::vector<float> waveTable, double sampleRate) :
    waveTable{ std::move(waveTable) },
    sampleRate{ sampleRate }
{
    
}


void WavetableOscillator::setFrequency(float frequency)
{
    indexIncrement = frequency * static_cast<float>(waveTable.size() / sampleRate);
}

float WavetableOscillator::getSample()
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

bool WavetableOscillator::isPlaying()
{
    return indexIncrement != 0.f;
}
