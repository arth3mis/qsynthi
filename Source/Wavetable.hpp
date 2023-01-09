/*
  ==============================================================================

    Wavetables.h
    Created: 13 Dec 2022 4:07:32pm
    Author:  Jannis Müller

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include <complex>
#include "list.hpp"

class Wavetable
{
public:
    // Important Constants
    constexpr static size_t SIZE = 128;            // Number of Samples per Wavetable
    constexpr static float A3_FREQUENCY = 440.f;
    constexpr static float A3_NOTE_NUMBER = 69.f + 12.f;
    constexpr static float SEMITONES_PER_OCTAVE = 12.f;

    // Constants for easy access
    constexpr static float SIZE_F = static_cast<float>(SIZE);
    constexpr static float TWO_PI = MathConstants<float>::twoPi;

    static list<std::complex<float>> generate(const size_t type, const float shift, const float scale);
    static float midiNoteToIncrement(const int noteNumber, const float sampleRate);
};
