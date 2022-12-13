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
        sineWaveTable[i] = 2 * std::expf(-0.01 * (i - 32) * (i - 32) ) - 1;
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
    
    // Ask every playing oscillator if it's still playing
    for (const auto& [note, oscillator] : oscillators)
        if (oscillator.isStopped()) oscillators.erase(note);
    
    
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
        int noteNumber = midiEvent.getNoteNumber()
        if (oscillators.contains(noteNumber))
            return;
        
        oscillators.insert(midiEvent.getNoteNumber(), WavetableOscillator(/*WAVETABLE*/, noteNumber, sampleRate));
        
    }
    else if (midiEvent.isNoteOff())
    {
        oscillators.erase(midiEvent);
        
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

void QSynthi::render(AudioBuffer<float>& buffer, int startSample, int endSample)
{
    auto* firstChannel = buffer.getWritePointer(0);
    
    for (auto& oscillator: oscillators)
    {
        if (oscillator.isPlaying())
        {
            for (auto sample = startSample; sample < endSample; ++sample)
            {
                firstChannel[sample] += oscillator.getNextSample();
            }
        }
    }
    
    for (auto channel = 1; channel < buffer.getNumChannels(); ++channel)
    {
        std::copy(firstChannel + startSample, firstChannel + endSample, buffer.getWritePointer(channel) + startSample);
    }
}
