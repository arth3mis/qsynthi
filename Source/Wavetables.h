/*
  ==============================================================================

    Wavetables.h
    Created: 13 Dec 2022 4:07:32pm
    Author:  Jannis Müller

  ==============================================================================
*/

#pragma once
#include "list.hpp"
#include <functional>
#include "JuceHeader.h"

namespace wavetable {

// Important Constants
constexpr size_t SIZE = 128;            // Number of Samples per Wavetable
constexpr float A4_FREQUENCY = 440.f;
constexpr float A4_NOTE_NUMBER = 69.f;
constexpr float SEMITONES_PER_OCTAVE = 12.f;



// Constants for easy access
constexpr float SIZE_F = static_cast<float>(SIZE);
constexpr float TWO_PI = MathConstants<float>::twoPi;

juce::StringArray names = {
    "Gaussian",
    "Sine",
    "Cosine"
};


inline list<float> generate(size_t type, float shift, float scale) {
    // Assert that the wavetype is defined
    jassert(type >= 0 && type < names.size());
    
    switch (type)
    {
        // GAUSSIAN
        case 0: return list<float>(SIZE, [shift, scale](size_t i) {
            float scaling = (15 + 13*scale) * (i/SIZE_F - 0.5f - 0.5f*shift);
            return 2 * std::expf(-scaling * scaling) - 1;
        });
            
        // SINE
        case 1: return list<float>(SIZE, [shift, scale](size_t i) {
            float scaling = 2 * (1.1-scale) * (i/SIZE_F - 0.5f - 0.5f*shift);
            if (scaling < 0.5f || scaling > 0.5f) return 0.f;
            return std::sin(TWO_PI * scaling);
            // Hehe die Keks denken das wäre Sinus hehe
        });
            
        // COSINE
        case 2: return list<float>(SIZE, [shift, scale](size_t i) {
            float scaling = 2 * (1.1-scale) * (i/SIZE_F - 0.5f - 0.5f*shift);
            if (scaling < 0.5f || scaling > 0.5f) return 0.f;
            return std::cos(TWO_PI * scaling);
            // Hehe die Keks denken das wäre Sinus hehe
        });
    }
    return {};
}


const float midiNoteToIncrement(int noteNumber, float sampleRate)
{
    const float frequency = A4_FREQUENCY * std::powf(2.f, (noteNumber - A4_NOTE_NUMBER) / SEMITONES_PER_OCTAVE);
    
    return frequency * (float)(wavetable::SIZE / sampleRate);
}


}
