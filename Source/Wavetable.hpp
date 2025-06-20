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

typedef std::complex<float> cfloat;

class Wavetable
{
public:
    // Important Constants
    constexpr static size_t SIZE = 128;            // Number of Samples per Wavetable
    constexpr static float A4_FREQUENCY = 440.f;
    constexpr static float A4_NOTE_NUMBER = 69.f;
    constexpr static float SEMITONES_PER_OCTAVE = 12.f;

    // Constants for easy access
    constexpr static float SIZE_F = static_cast<float>(SIZE);
    constexpr static float TWO_PI = MathConstants<float>::twoPi;

    static list<cfloat> generate(const size_t type, const float shift, const float scale);
    static float midiNoteToFrequency(const int noteNumber);
    static float frequencyToIncrement(const float frequency, const float sampleRate);
    
private:
    static inline float gaussianCurve(float x, float shift, float scale);
    static inline float sineCurve(float x, float shift, float scale);
    static inline float cosCurve(float x, float shift, float scale);
    static inline float parabolaCurve(float x, float shift, float scale);
    static inline float sawtoothCurve(float x, float shift, float scale);
    static inline float squareCurve(float x, float shift, float scale);
    
    static inline float sliderScaling(float sliderValue, float valueAtNeg1, float valueAt0, float valueAt1, float mixLinear);
};
