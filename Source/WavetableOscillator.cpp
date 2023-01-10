//
//  WavetableOscillator.cpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//


#include "WavetableOscillator.hpp"
#include <cmath>
#include "Wavetable.hpp"

#define POCKETFFT_CACHE_SIZE 1
#define POCKETFFT_NO_MULTITHREADING
#include "pocketfft_hdronly.h"

#define PI 3.1415926f

// Sowas geht
//namespace wvt = wavetable;

WavetableOscillator::WavetableOscillator(Parameter *parameter) 
    : parameter{ parameter }
    , state{ State::SLEEP }
{
}

void WavetableOscillator::prepareToPlay(int midiNote, float sampleRate)
{
    phaseIncrement = Wavetable::midiNoteToIncrement(midiNote, sampleRate);
    envelopeLevel = 0.f;
}

void WavetableOscillator::noteOn(int velocity) {
    /*
    
    Interesting scenarios:
    Gaussian:
    (0, -0.6f, 0.5f) with V(x) = 0.0015*x²
    todo: linear potential?
    (0, -0.6f, 0.5f) with V(x) = 0

    Sine:
    (1, -0.4f, -0.6f) with V(x) = 0.0015*x²
    todo: other potentials?
    
    */

    //waveTable = Wavetable::generate(0, -0.6f, 0.5f);
    //waveTable = Wavetable::generate(1, -0.4f, 0.f);
    
    waveTable = Wavetable::generate(parameter->waveTypeNumber, parameter->waveShift, parameter->waveScale).to<cfloat>();
    
    // FFT result as standard form?
    if (parameter->showFFT)
    {
        waveTable = list(fft(waveTable.toVector()));
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

float WavetableOscillator::getNextSample()
{
    // Schrödinger
    // parameter changed?
    if (1.0 / parameter->timestepsPerSample != timestepCountTo)
    {
        timestepCountTo = 1.0 / parameter->timestepsPerSample;
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
                for (int i=0; i<2; ++i) doTimestep(parameter->timestepDelta / 2);
            }
        }
        // multiple samples pass before timestep?
        else
        {
            timestepCounter += 1;
            if (timestepCounter >= timestepCountTo)
                for (int i=0; i<2; ++i) doTimestep(parameter->timestepDelta / 2);
        }
        timestepCounter = fmod(timestepCounter, timestepCountTo);
    }

    // Audio calculations
    //
    const auto sample = 
          envelopeLevel 
        * velocityLevel 
        * waveTable.getLinearInterpolation(phase, getSampleConversion(parameter->sampleType, 1, 0));
    
    phase = std::fmod(phase + phaseIncrement, Wavetable::SIZE_F);

    updateState();
    
    return sample;
}

inline std::function<float(cfloat)> WavetableOscillator::getSampleConversion(const SampleType type, float scale, float yShift)
{
    if (type == SampleType::REAL_VALUE)         return [scale, yShift](cfloat z) { return std::real(z) * scale + yShift; };
    else if (type == SampleType::IMAG_VALUE)    return [scale, yShift](cfloat z) { return std::imag(z) * scale + yShift; };
    else if (type == SampleType::SQARED_ABS)    return [scale, yShift](cfloat z) { return std::norm(z) * scale - yShift; };
    else                                        return [scale, yShift](cfloat z) { return 0; };
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


inline cvec WavetableOscillator::fft(cvec in)
{
    // https://gitlab.mpcdf.mpg.de/mtr/pocketfft/-/blob/cpp/pocketfft_demo.cc
    // args: sampleCount, byteOffsetIn, byteOffsetOut, direction, inputArray, outputArray, scaleFactor
    //
    cvec out(in.size());
    pocketfft::c2c({ in.size() }, { sizeof(cfloat) }, { sizeof(cfloat) }, { 0 }, pocketfft::FORWARD, in.data(), out.data(), (float)(1.0 / sqrt(in.size())));
    return out;
}

// based on: http://www.articlesbyaphysicist.com/quantum4prog.html
void WavetableOscillator::doTimestep(const float dt)
{
    // note: 2 FFTs are minimum, regardless of showFFT setting

    cvec v = waveTable.toVector();
    const size_t n = v.size();

    if (parameter->showFFT)
        v = fft(v);
    
    // "timestepV"
    for (size_t i = 1; i < n - 1; i++)
    {
        v[i] *= std::polar(1.f, dt * potential(i));
    }

    v = fft(v);
    
    // "timestepT"
    const float PRE = powf(2*PI/n, 2);
    for (size_t i = 1; i < n / 2; i++)
    {
        v[i]   *= std::polar(1.f, PRE*i*i * dt);
        v[n-i] *= std::polar(1.f, PRE*i*i * dt);
    }
    
    if (!parameter->showFFT)
        v = fft(v);

    waveTable = list(v);
}

inline float WavetableOscillator::potential(const size_t x)
{
    /*
    return x * x * 0.0015f;
    /*/
    return parameter->potential[x];
    //*/
}
