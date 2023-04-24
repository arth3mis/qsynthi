/*
  ==============================================================================

    Parameter.h
    Created: 16 Dec 2022 10:50:27pm
    Author:  Arthur

  ==============================================================================
*/

#pragma once
#include "list.hpp"
#include <functional>
#include <JuceHeader.h>

#define PARAM_VERSION 1

#define GAIN "Gain"
#define VOICE_COUNT "Number of Voices"
#define PORTAMENTO "Portamento"

#define ATTACK_TIME "Attack"
#define DECAY_TIME "Decay"
#define RELEASE_TIME "Release"
#define SUSTAIN_LEVEL "Sustain"

#define WAVE_TYPE "Wave Type"
#define WAVE_SHIFT "Wave Shift"
#define WAVE_SCALE "Wave Scale"

#define POTENTIAL_TYPE1 "Potential Type 1"
#define POTENTIAL_SHIFT1 "Potential Shift 1"
#define POTENTIAL_SCALE1 "Potential Scale 1"
#define POTENTIAL_AMOUNT1 "Potential Amount 1"
#define POTENTIAL_TYPE2 "Potential Type 2"
#define POTENTIAL_SHIFT2 "Potential Shift 2"
#define POTENTIAL_SCALE2 "Potential Scale 2"
#define POTENTIAL_AMOUNT2 "Potential Amount 2"

#define APPLY_WAVEFUNC "Schroedinger"
#define ACCURACY "Timesteps / Simulated sec"
#define SIMULATION_SPEED "Simulated sec / Real sec"
#define SIMULATION_OFFSET "Pre-start simulated sec"
#define SAMPLE_TYPE "Sample Type"
#define SHOW_FFT "FFT"

#define FILTER_FREQUENCY "Filter Frequency"
#define FILTER_RESONANCE "Filter Resonance"
#define FILTER_ENVELOPE "Filter Envelope Amount"
#define FILTER_KEY_TRACK "Filter Key Track"

#define STEREO_AMOUNT "Stereoize"

#define REVERB_MIX "Reverb Mix"

typedef std::complex<float> cfloat;

enum WaveType
{
    GAUSSIAN,
    SINE,
    COSINE,
    PARABOLA,
    BARRIER,
    SAWTOOTH,
    SQUARE
};

enum SampleType
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
    static constexpr float ATTACK_THRESHOLD = 0.01f;
    static constexpr float DECAY_THRESHOLD = 0.00001f;
    static constexpr float RELEASE_THRESHOLD = 0.00001f;
    
    static const StringArray WAVE_TYPES;
    static const StringArray SAMPLE_TYPES;
    
    static constexpr float POTENTIAL_SCALE = 1.f;
    
    // general
    float gainFactor = 0;
    float numVoices = 3;
    float portamentoTime = 0.5;
    
    // Envelope
    float attackFactor = 0;
    float decayFactor = 0;
    float sustainLevel = 0;
    float releaseFactor = 0;
    
    // Filter
    float filterFreq = 500;
    float filterQ = 1;
    float filterEnvelope = 4;
    
    // Waveforms
    int waveTypeNumber = 0;
    float waveShift = 0;
    float waveScale = 0;

    
    
    
    // For Schroedinger
    bool applyWavefunction = false; // True if Schrödinger's equation should be applied to waveform
    
    float samplesPerTimestep = 0;   // Number of timesteps which get performed after a sample is calculated. Always > 0, could get > 1
    float timestepDelta = 0;        // Time duration of each timestep
    size_t preStartTimesteps = 0;

    // Values per Wavetable for each
    list<float> potential;
    
    SampleType sampleType;          // for default value, go to layout creation
    bool showFFT = false;           // True if the FFT of the waveform should be played
    
    inline std::function<float(cfloat)> getSampleConverter()
    {
        if (sampleType == SampleType::REAL_VALUE)       return [](cfloat z) { return std::real(z); };
        else if (sampleType == SampleType::IMAG_VALUE)  return [](cfloat z) { return std::imag(z); };
        else if (sampleType == SampleType::SQARED_ABS)  return [](cfloat z) { return std::norm(z); };
        else                                            return [](cfloat z) { return 0; };
    }


    static AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void update(AudioProcessorValueTreeState& getParameter, float sampleRate);
    
    
    // Stereo
    list<float> stereoList;
    
    // FX
    float reverbMix = 0;
};
