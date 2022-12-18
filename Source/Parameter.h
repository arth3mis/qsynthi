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

#define GAIN "Gain"

#define ATTACK "Attack"
#define DECAY "Decay"
#define SUSTAIN "Sustain"
#define RELEASE "Release"

#define ACCURACY "Accuracy"
#define SIMULATION_SPEED "Simulation Speed"
#define SHOW_FFT "FFT"

/**
 struct to communicate Parameter between Front- and Backend. General Idea:  struct "Parameter" contains only processed values which not necessarily correspond 1:1 to the Parameters of the Front-End
 */
class Parameter
{
    // general
    float gainFactor;
    
    // Envelope
    float attackFactor;
    float decayFactor;
    float sustainLevel;
    float releaseFactor;
    
    // For Schroedinger
    float timestepsPerSample;   // Number of timesteps which get performed after a sample is calculated. Always > 0, could get > 1
    float timestepDelta;        // Time duration of each timestep
    bool showFFT;               // True if the FFT of the waveform should be played

    list<AudioProcessorParameter> processorParameters();
    void update(std::function<float(StringRef)> getParameter);
    
    Parameter(float attackFactor,
              float decayFactor,
              float releaseFactor,
              float sustainLevel) :
    attackFactor{ attackFactor },
    decayFactor{ decayFactor },
    sustainLevel{ sustainLevel },
    releaseFactor{ releaseFactor }
    {}
    
    
};
