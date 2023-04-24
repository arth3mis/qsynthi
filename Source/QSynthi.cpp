//
//  QSynthi.cpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//

#include "QSynthi.hpp"
#include <complex>
#include "Wavetable.hpp"


QSynthi::QSynthi(Parameter *parameter) : parameter{ parameter }
{
    

    /*
    mutable_list<int> test(2);
    test[0] = 3;
    test += 41;
    test.forEach([](int& a) { a++; });
    auto t2 = test;
    t2 += {1,2,3};  // does not change test anymore
    test.eraseItem(42);
    //*/

    //using namespace std::complex_literals;
    //list<std::complex<float>> psiTest = {1.f+1if, 3.f};
    //float x = psiTest.getLinearInterpolation<float>(0.5f, [](std::complex<float> z) { return std::norm(z); });
}

void QSynthi::prepareToPlay(float sampleRate)
{
    this->sampleRate = sampleRate;
    reverb.setSampleRate((double) sampleRate);
    
    playingOscillators = {};
    sleepingOscillators = {};
    
    oscillators = mutable_list<WavetableOscillator>(parameter->numVoices, [sampleRate, this](size_t _){
        return WavetableOscillator(parameter);
    });
    
    for (int i = 0; i < oscillators.length(); i++)
    {
        oscillators[i].prepareToPlay(sampleRate);
        sleepingOscillators.append(&(oscillators[i]));
    }
    
    stolenNotes = {};
    sustainedNotes = {};
    

}
/**
 Coordinates handleMidiEvent(...) and render(...) to process the midiMessages and fill the buffer
 */
void QSynthi::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    // Test if number of oscillators changed
    if (parameter->numVoices != oscillators.length())
    {
        prepareToPlay(sampleRate);
    }
    
    
    int currentSample = 0;
    
    reverb.setParameters(Reverb::Parameters{0.4f + 0.5f * parameter->reverbMix * parameter->reverbMix, 0.4f, 0.35f * parameter->reverbMix, (1-parameter->reverbMix), 0.8f, 0.0f});


    // idea 1: create all, keep all always, ask everyone for isPlaying
    // idea 2: make list with playing oscis' references, maybe self-updating
    
    
    
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
    
    // Apply Reverb
    if (parameter->reverbMix != 0) {
        reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
    }
    
}

void QSynthi::handleMidiEvent(const MidiMessage& midiEvent)
{
    if (midiEvent.isNoteOn())
    {
        int noteNumber = midiEvent.getNoteNumber();
        sustainedNotes.eraseItem(noteNumber);
        
        WavetableOscillator* oscillator = nullptr;
        
        // Find playing Osci with same note
        for (size_t i = 0; i < playingOscillators.length(); i++)
        {
            if (playingOscillators[i]->midiNote == noteNumber)
            {
                oscillator = playingOscillators[i];
                playingOscillators.erase(i);
                break;
            }
        }
        // No playing Osci found
        if (oscillator == nullptr)
        {
            if (sleepingOscillators.length() > 0) {
                oscillator = sleepingOscillators[0];
                sleepingOscillators.erase(0);
            } else {
                oscillator = playingOscillators[0];
                playingOscillators.erase(0);
                stolenNotes.append(oscillator->midiNote);
            }
        }
        
        playingOscillators.append(oscillator);
        oscillator->noteOn(noteNumber, midiEvent.getVelocity());
        
        if (displayedOscillator == nullptr) displayedOscillator = oscillator;
    }
    else if (midiEvent.isNoteOff())
    {
        int noteNumber = midiEvent.getNoteNumber();
        
        if (sustain)
        {
            sustainedNotes.append(noteNumber);
        }
        else
        {
            noteOff(midiEvent.getNoteNumber());
        }
        
        
    }
    else if (midiEvent.isAllNotesOff())
    {
        playingOscillators.forEach([this](auto o) {
            o->noteOff();
            this->sleepingOscillators.append(o);
        });

        displayedOscillator = nullptr;
        stolenNotes = {};
        
    }
    else if (midiEvent.isSustainPedalOn())
    {
        sustain = true;
    }
    else if (midiEvent.isSustainPedalOff())
    {
        sustain = false;
        for (int i = 0; i < sustainedNotes.length(); i++) {
            noteOff(sustainedNotes[i]);
        }
        sustainedNotes = {};
    }
}

void QSynthi::noteOff(int noteNumber) {
    stolenNotes.eraseItem(noteNumber);
    
    for (int i = 0; i < playingOscillators.length(); i++) {
        if (playingOscillators[i]->midiNote == noteNumber)
        {
            auto o = playingOscillators[i];
            
            if (stolenNotes.length() <= 0) {
                o->noteOff();
                sleepingOscillators.append(o);
                if (displayedOscillator == o) displayedOscillator = nullptr;
                playingOscillators.erase(i--);
                
            } else {
                
                int midiNote = stolenNotes[stolenNotes.length() - 1];
                stolenNotes.eraseItem(midiNote);
                
                playingOscillators.erase(i--);
                playingOscillators.append(o);
                o->noteOn(midiNote, 127);
            }
        }
    }
}


void QSynthi::render(AudioBuffer<float>& buffer, int startSample, int endSample)
{
    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    
    oscillators
        //.filter([](auto oscillator) { return oscillator.isPlaying(); })
        .forEach([startSample, endSample, left, right, this](auto& oscillator) {
            // skip sleeping oscis
            if (!oscillator.isPlaying())
                return;
            // 
            for (int sample = startSample; sample < endSample; ++sample)
            {
                float phase = oscillator.getPhase();
                float sampleData = oscillator.getNextSample();
                
                // Apply stereoize
                left[sample] += sampleData * parameter->stereoList[phase];
                right[sample] += sampleData * parameter->stereoList[Wavetable::SIZE - 1 - phase];
            }
        });
    
    // Apply Gian
    for (int sample = startSample; sample < endSample; ++sample) {
        left[sample] *= parameter->gainFactor;
        right[sample] *= parameter->gainFactor;
    }
}
