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

list<cfloat> QSynthi::getDisplayedWavetable() {
    std::lock_guard lock(displayAccessMutex);
    
    // Safety check: ensure displayedOscillator is valid and still playing
    if (displayedOscillator != nullptr && displayedOscillator->isPlaying()) {
        return displayedOscillator->waveTable;
    }
    
    // If current displayed oscillator is invalid, try to find a valid one
    if (displayedOscillator == nullptr || !displayedOscillator->isPlaying()) {
        // Check if there's a valid oscillator in the queue
        while (!displayQueue.empty()) {
            auto nextOsc = displayQueue.front();
            displayQueue.pop_front();
            
            if (nextOsc != nullptr && nextOsc->isPlaying()) {
                displayedOscillator = nextOsc;
                return displayedOscillator->waveTable;
            }
        }
        
        // No valid oscillators found, clear display
        displayedOscillator = nullptr;
    }
    
    return list<cfloat>(); // Return empty list if no valid oscillator
}

bool QSynthi::hasDisplayedWavetable() const {
    return displayedOscillator != nullptr;
}

void QSynthi::prepareToPlay(const float sampleRate)
{
    this->sampleRate = sampleRate;
    reverb.setSampleRate(sampleRate);
    
    // Clear display system to prevent dangling pointers
    {
        std::lock_guard lock(displayAccessMutex);
        displayedOscillator = nullptr;
        displayQueue.clear();
    }
    
    playingOscillators = {};
    sleepingOscillators = {};
    
    oscillators = mutable_list<WavetableOscillator>(static_cast<size_t>(parameter->numVoices), [this](size_t _){
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
void QSynthi::processBlock(AudioBuffer<float>& buffer, const MidiBuffer& midiMessages)
{
    // Test if number of oscillators changed
    if (static_cast<size_t>(parameter->numVoices) != oscillators.length())
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
        
        // Update display system
        {
            std::lock_guard lock(displayAccessMutex);
            if (displayedOscillator == nullptr) {
                displayedOscillator = oscillator;
            } else {
                // Only add to queue if not already present
                bool alreadyInQueue = false;
                for (auto& queuedOsc : displayQueue) {
                    if (queuedOsc == oscillator) {
                        alreadyInQueue = true;
                        break;
                    }
                }
                if (!alreadyInQueue) {
                    displayQueue.push_back(oscillator);
                }
            }
        }
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
        for (size_t i = 0; i < playingOscillators.length(); i++) {
            playingOscillators[i]->noteOff();
            this->sleepingOscillators.append(playingOscillators[i]);
        }

        // Clear display system
        {
            std::lock_guard lock(displayAccessMutex);
            displayedOscillator = nullptr;
            displayQueue.clear();
        }
        
        playingOscillators = {};
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

void QSynthi::setDisplayedOscillator(WavetableOscillator *o) {
    std::lock_guard lock(displayAccessMutex);
    
    // Safety check: only set valid oscillators that are playing
    if (o == nullptr || o->isPlaying()) {
        displayedOscillator = o;
    } else {
        displayedOscillator = nullptr;
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
                
                // Remove from display queue
                {
                    std::lock_guard lock(displayAccessMutex);
                    for (auto it = displayQueue.begin(); it != displayQueue.end(); ++it) {
                        if (*it == o) {
                            displayQueue.erase(it);
                            break;
                        }
                    }
                    
                    // Update displayed oscillator if needed
                    if (displayedOscillator == o) {
                        displayedOscillator = nullptr;
                        if (!displayQueue.empty()) {
                            displayedOscillator = displayQueue.front();
                            displayQueue.pop_front();
                        }
                    }
                }
                
                playingOscillators.erase(i--);
                
            } else {
                // Voice stealing - reuse this oscillator for a stolen note
                int midiNote = stolenNotes[stolenNotes.length() - 1];
                stolenNotes.eraseItem(midiNote);
                
                // Remove from display queue before reusing
                {
                    std::lock_guard lock(displayAccessMutex);
                    for (auto it = displayQueue.begin(); it != displayQueue.end(); ++it) {
                        if (*it == o) {
                            displayQueue.erase(it);
                            break;
                        }
                    }
                }
                
                playingOscillators.erase(i--);
                playingOscillators.append(o);
                o->noteOn(midiNote, 127);
                
                // Add back to display queue with new note
                {
                    std::lock_guard lock(displayAccessMutex);
                    if (displayedOscillator == nullptr) {
                        displayedOscillator = o;
                    } else {
                        displayQueue.push_back(o);
                    }
                }
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
