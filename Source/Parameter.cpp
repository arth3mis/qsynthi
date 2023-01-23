/*
  ==============================================================================

    Parameter.cpp
    Created: 17 Dec 2022 7:03:54pm
    Author:  Jannis Müller

  ==============================================================================
*/

#include "Parameter.h"
#include "Wavetable.hpp"

#define FLOAT_PARAM(paramName, range, baseValue) layout.add(std::make_unique<AudioParameterFloat>(ParameterID { (paramName), PARAM_VERSION }, (paramName), (range), (baseValue)))

#define BOOL_PARAM(paramName, baseValue) layout.add(std::make_unique<AudioParameterBool>(ParameterID { (paramName), PARAM_VERSION }, (paramName), (baseValue)))

#define CHOICE_PARAM(paramName, choices, baseValue) layout.add(std::make_unique<AudioParameterChoice>(ParameterID{ (paramName), PARAM_VERSION }, (paramName), (choices), (baseValue)));

#define GET(paramName) treeState.getRawParameterValue(paramName)->load()


const StringArray Parameter::WAVE_TYPES = {
    "Gaussian",
    "Sine",
    "Cosine",
    "Parabola",
    "Barrier"
};

const StringArray Parameter::SAMPLE_TYPES = {
    "Real Value",
    "Imaginary Value",
    "Squared Absolute"
};

AudioProcessorValueTreeState::ParameterLayout Parameter::createParameterLayout() {
    AudioProcessorValueTreeState::ParameterLayout layout;
    
    // MAKROS SIND JA DOCH GANZ PRAKTISCH (in C++ zumindest)
    // trotzdem Klammern nicht vergessen (mach ich auch imemr wieder)
    
    FLOAT_PARAM(GAIN, NormalisableRange<float>(-64.f, 0.f, 0.1f, 0.9f, true), -24.f);
    
    FLOAT_PARAM(ATTACK_TIME, NormalisableRange<float>(0.001f, 16.f, 0.001f, 0.4f, false), 0.1f);
    FLOAT_PARAM(DECAY_TIME, NormalisableRange<float>(0.001f, 16.f, 0.001f, 0.4f, false), 0.5f);
    FLOAT_PARAM(RELEASE_TIME, NormalisableRange<float>(0.001f, 16.f, 0.001f, 0.4f, false), 0.5f);
    FLOAT_PARAM(SUSTAIN_LEVEL, NormalisableRange<float>(-64.f, 0.f, 0.1f, 0.9f, true), -12.f);
    
    CHOICE_PARAM(WAVE_TYPE, WAVE_TYPES, WaveType::GAUSSIAN);
    FLOAT_PARAM(WAVE_SHIFT, NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f, true), 0.f);
    FLOAT_PARAM(WAVE_SCALE, NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f, true), 0.f);
    
    BOOL_PARAM(APPLY_WAVEFUNC, true);

    FLOAT_PARAM(ACCURACY, NormalisableRange<float>(0.1f, 100.f, 0.1f, 0.25f, false), .5f);
    FLOAT_PARAM(SIMULATION_SPEED, NormalisableRange<float>(0.1f, 1000.f, 0.01f, 0.25f, false), 42.f);
    FLOAT_PARAM(SIMULATION_OFFSET, NormalisableRange<float>(0.f, 1000.f, 1.f, 0.5f, false), 0.f);

    // Potential
    CHOICE_PARAM(POTENTIAL_TYPE1, WAVE_TYPES, WaveType::PARABOLA);
    FLOAT_PARAM(POTENTIAL_SHIFT1, NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f, true), 0.f);
    FLOAT_PARAM(POTENTIAL_SCALE1, NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f, true), 0.f);
    FLOAT_PARAM(POTENTIAL_AMOUNT1, NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f, true), 0.f);
    CHOICE_PARAM(POTENTIAL_TYPE2, WAVE_TYPES, WaveType::PARABOLA);
    FLOAT_PARAM(POTENTIAL_SHIFT2, NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f, true), 0.f);
    FLOAT_PARAM(POTENTIAL_SCALE2, NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f, true), 0.f);
    FLOAT_PARAM(POTENTIAL_AMOUNT2, NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f, true), 0.f);
    
    CHOICE_PARAM(SAMPLE_TYPE, SAMPLE_TYPES, SampleType::SQARED_ABS);
    

    BOOL_PARAM(SHOW_FFT, false);

    FLOAT_PARAM(STEREO_AMOUNT, NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f, true), 0.f);
    
    FLOAT_PARAM(REVERB_MIX, NormalisableRange<float>(0.f, 1.f, 0.01f, 0.9f, false), 0.15f);
    
    
    
    
    return layout;
}

void Parameter::update(AudioProcessorValueTreeState& treeState, float sampleRate)
{
    gainFactor = Decibels::decibelsToGain(GET(GAIN));
    
    // Envelope
    attackFactor = 1 - std::pow(RELEASE_THRESHOLD, 1 / (sampleRate * GET(ATTACK_TIME)));
    decayFactor = 1 - std::pow(RELEASE_THRESHOLD, 1 / (sampleRate * GET(DECAY_TIME)));
    releaseFactor = std::pow(RELEASE_THRESHOLD, 1 / (sampleRate * GET(RELEASE_TIME)));
    sustainLevel = Decibels::decibelsToGain(GET(SUSTAIN_LEVEL));
    
    // Waveforms
    waveTypeNumber = GET(WAVE_TYPE);
    waveShift = GET(WAVE_SHIFT);
    waveScale = GET(WAVE_SCALE);

    
    // Simulation
    applyWavefunction       = GET(APPLY_WAVEFUNC);
    float accuracy          = GET(ACCURACY);
    float simulationSpeed   = GET(SIMULATION_SPEED);
    
    // Potential
    const float potentialAmount1 = GET(POTENTIAL_AMOUNT1);
    const float potentialAmount2 = GET(POTENTIAL_AMOUNT2);
    const auto p1 = Wavetable::generate(GET(POTENTIAL_TYPE1), GET(POTENTIAL_SHIFT1), GET(POTENTIAL_SCALE1));
    const auto p2 = Wavetable::generate(GET(POTENTIAL_TYPE2), GET(POTENTIAL_SHIFT2), GET(POTENTIAL_SCALE2));
    potential = p1.zip(p2)
        .map([](cfloat a, cfloat b) { return a + b; })
        .mapTo<float>([potentialAmount1, potentialAmount2](cfloat v)
            {
                return 10.f * potentialAmount1 * potentialAmount2 * std::real(v);
            });
    
    samplesPerTimestep  = sampleRate / (accuracy * simulationSpeed);
    timestepDelta       = 1.f / accuracy;
    preStartTimesteps   = round(GET(SIMULATION_OFFSET) * accuracy);
    
    sampleType  = static_cast<SampleType>(GET(SAMPLE_TYPE));
    showFFT     = GET(SHOW_FFT);
    
    //Stereo
    float stereoAmount = GET(STEREO_AMOUNT);
    stereoList = list<float>(Wavetable::SIZE, [stereoAmount](size_t i){
        return 1 - stereoAmount * 0.5f * (std::tanhf(Wavetable::TWO_PI * (i / Wavetable::SIZE_F - 0.5f)) + 1);
    });
    
    // FX
    reverbMix = GET(REVERB_MIX);
}
