/*
  ==============================================================================

    Parameter.cpp
    Created: 17 Dec 2022 7:03:54pm
    Author:  Jannis Müller

  ==============================================================================
*/

#include "Parameter.h"

#define FLOAT_PARAM(paramName, range, baseValue) layout.add(std::make_unique<AudioParameterFloat>(ParameterID { (paramName), PARAM_VERSION }, (paramName), (range), (baseValue)))

#define BOOL_PARAM(paramName, baseValue) layout.add(std::make_unique<AudioParameterBool>(ParameterID { (paramName), PARAM_VERSION }, (paramName), (baseValue)))


#define GET(paramName) treeState.getRawParameterValue(paramName)->load()

AudioProcessorValueTreeState::ParameterLayout Parameter::createParameterLayout() {
    AudioProcessorValueTreeState::ParameterLayout layout;
    
    // MAKROS SIND JA DOCH GANZ PRAKTISCH (in C++ zumindest)
    // trotzdem Klammern nicht vergessen (mach ich auch imemr wieder)
    
    FLOAT_PARAM(GAIN, NormalisableRange<float>(-64.f, 0.f, 0.1f, 0.9f, true), -24.f);
    
    FLOAT_PARAM(ATTACK_TIME, NormalisableRange<float>(0.001f, 16.f, 0.001f, 0.4f, false), 0.1f);
    FLOAT_PARAM(DECAY_TIME, NormalisableRange<float>(0.001f, 16.f, 0.001f, 0.4f, false), 0.3f);
    FLOAT_PARAM(RELEASE_TIME, NormalisableRange<float>(0.001f, 16.f, 0.001f, 0.4f, false), 0.4f);
    FLOAT_PARAM(SUSTAIN_LEVEL, NormalisableRange<float>(-64.f, 0.f, 0.1f, 0.9f, true), -12.f);
    
    BOOL_PARAM(APPLY_WAVEFUNC, true);
    FLOAT_PARAM(ACCURACY, NormalisableRange<float>(100.f, 10000.f, 1.f, 0.3f, false), 500.f);
    FLOAT_PARAM(SIMULATION_SPEED, NormalisableRange<float>(0.01f, 2.f, 1.f, 0.3f, false), 0.2f);

    layout.add(std::make_unique<AudioParameterChoice>(ParameterID{ SAMPLE_TYPE, PARAM_VERSION }, SAMPLE_TYPE, StringArray({
        "Real Value",
        "Imaginary Value",
        "Squared Absolute" }),
        // default value (yes, enum class to int is - interesting, but I wanted to try it)
        static_cast<typename std::underlying_type<SampleType>::type>(SampleType::REAL_VALUE)));

    BOOL_PARAM(SHOW_FFT, false);
    
    
    
    
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
    
    // Simulation
    applyWavefunction = GET(APPLY_WAVEFUNC);
    float accuracy = GET(ACCURACY);
    float simulationSpeed = GET(SIMULATION_SPEED);
    timestepsPerSample = accuracy * simulationSpeed / sampleRate;
    timestepDelta = 1 / accuracy;
    
    sampleType = static_cast<SampleType>(GET(SAMPLE_TYPE));
    showFFT = GET(SHOW_FFT);
}
