//
//  QSynthi.cpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//

#include "QSynthi.hpp"
#include "list.hpp"


void QSynthi::prepareToPlay(double sampleRate)
{
    this->sampleRate = sampleRate;

    initializeOscillators();
}

std::vector<float> QSynthi::generateSineWaveTable()
{
    constexpr auto WAVETABLE_LENGTH = 64;
    
    std::vector<float> sineWaveTable(WAVETABLE_LENGTH);
    
    for (auto i = 0; i < WAVETABLE_LENGTH; ++i) {
        //sineWaveTable[i] = std::sinf(juce::MathConstants<float>::twoPi * static_cast<float>(i) / static_cast<float>(WAVETABLE_LENGTH));
        sineWaveTable[i] = 2 * std::expf(-0.001 * (i - 32) * (i - 32) ) - 1.35;
    }
    
    return sineWaveTable;
}

void QSynthi::initializeOscillators()
{
    constexpr auto OSCILLATORS_COUNT = 128;
    
    const auto waveTable = generateSineWaveTable();
    
    oscillators.clear();
    for (auto i = 0; i < OSCILLATORS_COUNT; ++i) {
        oscillators.emplace_back(waveTable, sampleRate);
    }
}

void QSynthi::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    
    auto currentSample = 0;
    
    for (const auto midiMessage : midiMessages)
    {
        const auto midiEvent = midiMessage.getMessage();
        const auto midiEventSample = static_cast<int>(midiEvent.getTimeStamp());
        
        render(buffer, currentSample, midiEventSample);
        handleMidiEvent(midiEvent);
        
    
        currentSample = midiEventSample;
    }
    
    render(buffer, currentSample, buffer.getNumSamples());
    
}

void QSynthi::handleMidiEvent(const MidiMessage& midiEvent)
{
    if (midiEvent.isNoteOn())
    {
        const auto oscillatorId = midiEvent.getNoteNumber();
        const auto frequency = midiNoteNumberToFrequency(oscillatorId);
        oscillators[oscillatorId].setFrequency(frequency);
    }
    else if (midiEvent.isNoteOff())
    {
        const auto oscillatorId = midiEvent.getNoteNumber();
        oscillators[oscillatorId].stop();
    }
    else if (midiEvent.isAllNotesOff())
    {
        for (auto& oscillator : oscillators)
        {
            oscillator.stop();
        }
    }
}

float QSynthi::midiNoteNumberToFrequency(int midiNoteNumber)
{
    constexpr auto A4_FREQUENCY = 440.f;
    constexpr auto A4_NOTE_NUMBER = 69.f;
    constexpr auto SEMITONES_IN_AN_OCTAVE = 12.f;
    return A4_FREQUENCY * std::powf(2.f, (midiNoteNumber - A4_NOTE_NUMBER) / SEMITONES_IN_AN_OCTAVE);
}

void QSynthi::render(AudioBuffer<float>& buffer, int startSample, int endSample)
{
    auto* firstChannel = buffer.getWritePointer(0);
    
    for (auto& oscillator: oscillators)
    {
        if (oscillator.isPlaying())
        {
            for (auto sample = startSample; sample < endSample; ++sample)
            {
                firstChannel[sample] += oscillator.getSample();
            }
        }
    }
    
    for (auto channel = 1; channel < buffer.getNumChannels(); ++channel)
    {
        std::copy(firstChannel + startSample, firstChannel + endSample, buffer.getWritePointer(channel) + startSample);
    }
}
