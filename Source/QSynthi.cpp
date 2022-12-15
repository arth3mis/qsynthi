//
//  QSynthi.cpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//

#include "QSynthi.hpp"
//#include "Wavetables.hpp"
#include "list.hpp"


void QSynthi::prepareToPlay(float sampleRate)
{
    this->sampleRate = sampleRate;

    oscillators.clear();
}

std::vector<float> QSynthi::generateSineWaveTable()
{
    return {};
}

void QSynthi::initializeOscillators()
{
}

/**
 Coordinates handleMidiEvent(...) and render(...) to process the midiMessages and fill the buffer
 */
void QSynthi::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    
    auto currentSample = 0;


    // idea 1: create all, keep all always, ask everyone for isPlaying
    // idea 2: make list with playing oscis' references, maybe self-updating
    
    // Ask every playing oscillator if it's still playing
    /*for (const auto& [key, oscillator] : oscillators)
    {
        if (oscillator.isDone())
        {
            
        }
    }*/
    
    
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
        
        
        oscillators.emplace(noteNumber, std::move(WavetableOscillator(0, 0, 0, noteNumber, sampleRate)));
        
    }
    else if (midiEvent.isNoteOff())
    {
        oscillators[midiEvent.getNoteNumber()].noteOff();
    }
    else if (midiEvent.isAllNotesOff())
    {
        for (const auto& [_, oscillator] : oscillators) oscillator.noteOff();
    }
}

void QSynthi::render(AudioBuffer<float>& buffer, int startSample, int endSample)
{
    auto* firstChannel = buffer.getWritePointer(0);
    
    for (const auto& [_, oscillator] : oscillators)
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
