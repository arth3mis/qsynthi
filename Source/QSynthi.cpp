//
//  QSynthi.cpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//

#include "QSynthi.hpp"
#include "list.hpp"
#include "Wavetables.h"


void QSynthi::prepareToPlay(float sampleRate)
{
    this->sampleRate = sampleRate;

    oscillators = {};
}

/**
 Coordinates handleMidiEvent(...) and render(...) to process the midiMessages and fill the buffer
 */
void QSynthi::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    
    auto currentSample = 0;
    
    // Ask every playing oscillator if it's still playing
    for (auto& [key, oscillator] : oscillators)
        if (oscillator.isDone()) oscillators.erase(key);
    
    
    for (const auto midiMessage : midiMessages)
    {
        const auto midiEvent = midiMessage.getMessage();
        const auto midiEventSample = static_cast<int>(midiEvent.getTimeStamp());
        
        // Render everything before the event and handle it
        render(buffer, currentSample, midiEventSample);
        handleMidiEvent(midiEvent);
        
        currentSample = midiEventSample;
    }
    
    // Render everything after the last midiEvent in this block
    render(buffer, currentSample, buffer.getNumSamples());
    
}

void QSynthi::handleMidiEvent(const MidiMessage& midiEvent)
{
    if (midiEvent.isNoteOn())
    {
        int noteNumber = midiEvent.getNoteNumber();
        
        if (oscillators.contains(noteNumber)) return;
        
        
        oscillators.emplace(noteNumber, WavetableOscillator(0, 0, 0, noteNumber, sampleRate));
        
    }
    else if (midiEvent.isNoteOff())
    {
        oscillators[midiEvent.getNoteNumber()].noteOff();
    }
    else if (midiEvent.isAllNotesOff())
    {
        for (auto& [_, oscillator] : oscillators) oscillator.noteOff();
    }
}

void QSynthi::render(AudioBuffer<float>& buffer, int startSample, int endSample)
{
    auto* firstChannel = buffer.getWritePointer(0);
    
    for (auto& [_, oscillator] : oscillators)
    {
        for (auto sample = startSample; sample < endSample; ++sample)
        {
            firstChannel[sample] += oscillator.getNextSample();
        }
    }

    // Copy rendered signal to all other channels
    for (auto channel = 1; channel < buffer.getNumChannels(); ++channel)
    {
        std::copy(firstChannel + startSample, firstChannel + endSample, buffer.getWritePointer(channel) + startSample);
    }
}
