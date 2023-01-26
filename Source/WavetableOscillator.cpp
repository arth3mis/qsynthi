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

// Sowas geht
//namespace wvt = wavetable;

WavetableOscillator::WavetableOscillator(Parameter *parameter) 
    : parameter{ parameter }
    , state{ State::SLEEP }
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
    for (size_t i = 1; i < n - 1; i++)
    {
        v[i] *= std::polar(1.f, dt * potential(i));
    }

    v = fft(v, true);

    // "timestepT"
    const float PRE = powf(Wavetable::TWO_PI / n, 2) * dt;
    for (size_t i = 1; i < n / 2; i++)
    {
        const float theta = PRE * i * i;
        v[i] *= std::polar(1.f, theta);
        v[n - i] *= std::polar(1.f, theta);
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

void WavetableOscillator::prepareToPlay(int midiNote, float sampleRate)
{
    phaseIncrement = Wavetable::midiNoteToIncrement(midiNote, sampleRate);
    envelopeLevel = 0.f;
}

void WavetableOscillator::noteOn(int velocity) 
{
    // generate new wavetable
    waveTable = Wavetable::generate(parameter->waveTypeNumber, parameter->waveShift, parameter->waveScale).to<cfloat>();
    
    // FFT result as standard form?
    if (parameter->showFFT)
    {
        waveTable = list(fft(waveTable.toVector(), true));
        showFFT = true;
    }

    // pre-start simulation
    const size_t steps = parameter->preStartTimesteps;
    for (size_t i = 0; i < steps; i++)
    {
        doTimestep(parameter->timestepDelta);
    }
    
    // Do not set envelopeLevel = 0 here, to enable smooth retriggers of one note
    
    state = State::ATTACK;
    velocityLevel = velocity / 127.f; // TODO: Rethink velocity sensitivity
    
    phase = 0.f; // Not necessary for the sound, but helpful for null-tests
    // phaseIncrement is set by prepareToPlay
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
    
    phase = std::fmod(phase + phaseIncrement, Wavetable::SIZE_F);

    updateState();
    
    return sample;
}

void WavetableOscillator::updateState() {
    switch (state) {
        case State::SLEEP:
            break;
            
        case State::ATTACK:
            envelopeLevel += parameter->attackFactor * (1 - envelopeLevel);
            if (envelopeLevel > Parameter::ATTACK_THRESHOLD) state = State::DECAY;
            break;
            
        case State::DECAY:
        {
            float difference = (envelopeLevel - parameter->sustainLevel);
            envelopeLevel -= parameter->decayFactor * difference;
            if (difference < Parameter::DECAY_THRESHOLD) state = State::SUSTAIN;
        } break;
        
        case State::SUSTAIN:
            break;
        
        case State::RELEASE:
            envelopeLevel *= parameter->releaseFactor;
            // Is done playing?
            // Make threshold even smaller to keep playing super quietly after the set release time
            if (envelopeLevel < (Parameter::RELEASE_THRESHOLD * 0.01f)) {
                state = State::SLEEP;
                envelopeLevel = 0;
            }

    }
}
