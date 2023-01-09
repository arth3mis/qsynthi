/*
  ==============================================================================

    Parameter.h
    Created: 16 Dec 2022 10:50:27pm
    Author:  Arthur

  ==============================================================================
*/

#pragma once
#include "list.hpp"
#include <JuceHeader.h>

#define PARAM_VERSION 1

#define GAIN "Gain"

#define ATTACK_TIME "Attack"
#define DECAY_TIME "Decay"
#define RELEASE_TIME "Release"
#define SUSTAIN_LEVEL "Sustain"

#define WAVE_TYPE "Wave Type"
#define WAVE_SHIFT "Wave Shift"
#define WAVE_SCALE "Wave Scale"

#define APPLY_WAVEFUNC "Schroedinger"
#define ACCURACY "Timesteps / Simulated sec"
#define SIMULATION_SPEED "Simulated sec / Real sec"
#define SAMPLE_TYPE "Sample Type"
#define SHOW_FFT "FFT"



enum class SampleType
{
    // on update: check strings in layout creation!
    REAL_VALUE,
    IMAG_VALUE,
    SQARED_ABS
};

/**
 struct to communicate Parameter between Front- and Backend. General Idea:  struct "Parameter" contains only processed values which not necessarily correspond 1:1 to the Parameters of the Front-End
 */
class Parameter
{
public:
    // Envelope Constants
    static constexpr float ATTACK_THRESHOLD = 1 - 0.05f;
    static constexpr float DECAY_THRESHOLD = 0.01f;
    static constexpr float RELEASE_THRESHOLD = 0.005f;
    
    static const StringArray WAVE_TYPES;
    
    // general
    float gainFactor = 0;
    
    // Envelope
    float attackFactor = 0;
    float decayFactor = 0;
    float sustainLevel = 0;
    float releaseFactor = 0;
    
    // Waveforms
    int waveTypeNumber = 0;
    float waveShift = 0;
    float waveScale = 0;
    
    
    
    // For Schroedinger
    bool applyWavefunction = false; // True if Schrödinger's equation should be applied to waveform
    float timestepsPerSample = 0;   // Number of timesteps which get performed after a sample is calculated. Always > 0, could get > 1
    float timestepDelta = 0;        // Time duration of each timestep
    bool showFFT = false;           // True if the FFT of the waveform should be played
    SampleType sampleType = SampleType::SQARED_ABS; // for default value, go to layout creation

    static AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void update(AudioProcessorValueTreeState& getParameter, float sampleRate);
};
