//
//  WavetableOscillator.cpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//
#include "WavetableOscillator.hpp"
#include <cmath>
#include "Wavetable.hpp"

#define POCKETFFT_CACHE_SIZE 1000
#define POCKETFFT_NO_MULTITHREADING
#include "pocketfft_hdronly.h"


WavetableOscillator::WavetableOscillator(Parameter *parameter) 
    : parameter{ parameter }
    , state{ State::SLEEP }
{
    filter = new SingleThreadedIIRFilter();
}

WavetableOscillator::~WavetableOscillator()
{

}

// Schrödinger equation functions -------------------------------------------------------------------------------------------------
//

// based on: http://www.articlesbyaphysicist.com/quantum4prog.html
void WavetableOscillator::doTimestep(const float dt)
{
    // note: 2 FFTs are minimum, regardless of showFFT setting

    cvec v = waveTable.toVector();
    const size_t n = v.size();

    if (showFFT)
        v = fft(v, false);


    // "timestepV"
    for (size_t i = 0; i < n; i++)
    {
        v[i] *= std::polar(1.f, dt * potential(i));
    }

    v = fft(v, true);

    // "timestepT"
    const float PRE = powf(Wavetable::TWO_PI / n, 2) * dt;
    for (size_t i = 0; i < n / 2; i++)
    {
        v[i]         *= std::polar(1.f, PRE * i * i);
        v[n - (i+1)] *= std::polar(1.f, PRE * (i+1) * (i+1));
    }

    if (!showFFT)
        v = fft(v, false);

    waveTable = list(v);
    
    
}

inline float WavetableOscillator::potential(const size_t x)
{
    return parameter->potential[x];
}

inline cvec WavetableOscillator::fft(cvec in, bool forward)
{
    // https://gitlab.mpcdf.mpg.de/mtr/pocketfft/-/blob/cpp/pocketfft_demo.cc
    // args: sampleCount, byteOffsetIn, byteOffsetOut, direction, inputArray, outputArray, scaleFactor
    //
    cvec out(in.size());
    pocketfft::c2c({ in.size() }, { sizeof(cfloat) }, { sizeof(cfloat) }, { 0 }, forward, in.data(), out.data(), (float)(1.0 / sqrt(in.size())));
    return out;
}


// Note and sample processing ------------------------------------------------------------------------------------------------------
//

void WavetableOscillator::prepareToPlay(float sampleRate)
{
    playingFrequency = 0.f;
    envelopeLevel = 0.f;
    this->sampleRate = sampleRate;
}

void WavetableOscillator::noteOn(int midiNote, int velocity)
{
    // Do pitch stuff
    this->midiNote = midiNote;
    targetFrequency = Wavetable::midiNoteToFrequency(midiNote);
    if (playingFrequency == 0) playingFrequency = targetFrequency;
    oldFrequency = playingFrequency;
    
    phaseIncrement = Wavetable::frequencyToIncrement(playingFrequency, sampleRate);
    
    
    
    if (!isPlaying()) {
        // generate new wavetable
         waveTable = Wavetable::generate(parameter->waveTypeNumber, parameter->waveShift, parameter->waveScale).to<cfloat>();
        
        // pre-start simulation
        const size_t steps = parameter->preStartTimesteps;
        for (size_t i = 0; i < steps; i++)
        {
            doTimestep(parameter->timestepDelta);
        }
        
        phase = 0.f; // Not necessary for the sound, but helpful for null-tests
    }
    
    // FFT result as standard form?
    if (parameter->showFFT)
    {
        waveTable = list(fft(waveTable.toVector(), true));
        showFFT = true;
    }
    
    // Do not set envelopeLevel = 0 here, to enable smooth retriggers of one note
    
    if (!isNoteOn()) {
        velocityLevel = Decibels::decibelsToGain(-20 + 20 * velocity / 127.f); // TODO: Rethink velocity sensitivity
    }
    state = State::ATTACK;
    
    if (envelopeLevel < Parameter::ATTACK_THRESHOLD)
    {
        envelopeLevel = Parameter::ATTACK_THRESHOLD;
    }
    
}

void WavetableOscillator::noteOff() {
    state = State::RELEASE;
}

float WavetableOscillator::getPhase()
{
    return phase;
}

float WavetableOscillator::getNextSample()
{
    // Schrödinger
    // parameter changed?
    if (parameter->samplesPerTimestep != timestepCountTo)
    {
        timestepCountTo = parameter->samplesPerTimestep;
        timestepCounter = 0;
    }
    // update counter/do timestep if active
    else if (parameter->applyWavefunction)
    {
        // multiple timesteps per sample?
        if (timestepCountTo < 1)
        {
            while (timestepCounter < 1)
            {
                timestepCounter += timestepCountTo;
                doTimestep(parameter->timestepDelta);
            }
        }
        // multiple samples pass before timestep?
        else
        {
            timestepCounter += 1;
            if (timestepCounter >= timestepCountTo)
                doTimestep(parameter->timestepDelta);
        }
        timestepCounter = fmod(timestepCounter, timestepCountTo);
    }

    // Audio calculations
    //
    const auto sample =
          envelopeLevel 
        * velocityLevel 
        * waveTable.getLinearInterpolation(phase, parameter->getSampleConverter());
    
    // Update Frequency if playing Frequency didnt reach targetFrequency yet
    if (targetFrequency != playingFrequency)
    {
        if (parameter->portamentoTime == 0)
        {
            playingFrequency = targetFrequency;
            phaseIncrement = Wavetable::frequencyToIncrement(playingFrequency, sampleRate);
        }
        else if (((oldFrequency < targetFrequency && playingFrequency < targetFrequency) || (oldFrequency > targetFrequency && playingFrequency > targetFrequency)))
        {
            playingFrequency += (targetFrequency - oldFrequency) / parameter->portamentoTime / sampleRate;
            phaseIncrement = Wavetable::frequencyToIncrement(playingFrequency, sampleRate);
        } else {
            playingFrequency = targetFrequency;
            phaseIncrement = Wavetable::frequencyToIncrement(playingFrequency, sampleRate);
        }
    }
    phase = std::fmod(phase + phaseIncrement, Wavetable::SIZE_F);

    updateState();
    
    
    // Filter and Return
    filter->setCoefficients(IIRCoefficients::makeLowPass(sampleRate, std::max(std::min(parameter->filterFreq + parameter->filterFreq * parameter->filterEnvelope * (envelopeLevel - parameter->sustainLevel), sampleRate * 0.5f - 1), parameter->filterFreq * 0.25f), parameter->filterQ));
    return filter->processSingleSampleRaw(sample);
}

void WavetableOscillator::updateState() {
    switch (state) {
        case State::SLEEP:
            return;
            
        case State::ATTACK:
            envelopeLevel += envelopeLevel * parameter->attackFactor;
            if (envelopeLevel >= 1.f) {
                state = State::DECAY;
                envelopeLevel = 1.f;
            }
            return;
            
        case State::DECAY:
        {
            float difference = (envelopeLevel - parameter->sustainLevel);
            envelopeLevel -= parameter->decayFactor * difference;
            if (difference < Parameter::DECAY_THRESHOLD * 0.01f) {
                state = State::SUSTAIN;
                envelopeLevel = parameter->sustainLevel;
            }
        } return;
        
        case State::SUSTAIN:
            return;
        
        case State::RELEASE:
            envelopeLevel *= parameter->releaseFactor;
            // Is done playing?
            // Make threshold even smaller to keep playing super quietly after the set release time
            if (envelopeLevel < Parameter::RELEASE_THRESHOLD * 0.01f) {
                state = State::SLEEP;
                envelopeLevel = 0;
            }
            return;

    }
}
