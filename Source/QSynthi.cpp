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
    oscillators = mutable_list<WavetableOscillator>(128, [parameter](size_t _){
        return WavetableOscillator(parameter);
    });

    plot.setup(800,500);
    plot.start();
    noteOnCount = 0;
    noteToDraw = -1;

    /*
    mutable_list<int> test(2);
    test[0] = 3;
    test += 41;
    test.forEach([](int& a) { a++; });
    auto t2 = test;
    t2 += {1,2,3};  // does not change test anymore
    //*/

    //using namespace std::complex_literals;
    //list<std::complex<float>> psiTest = {1.f+1if, 3.f};
    //float x = psiTest.getLinearInterpolation<float>(0.5f, [](std::complex<float> z) { return std::norm(z); });
}

void QSynthi::prepareToPlay(float sampleRate)
{
    this->sampleRate = sampleRate;
    reverb.setSampleRate((double) sampleRate);
}
/**
 Coordinates handleMidiEvent(...) and render(...) to process the midiMessages and fill the buffer
 */
void QSynthi::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    
    int currentSample = 0;
    
    reverb.setParameters(Reverb::Parameters{0.5f + 0.45f * parameter->reverbMix * parameter->reverbMix, 0.4f, parameter->reverbMix, 0.8f * (1-parameter->reverbMix), 0.8f, 0.0f});


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

    if (noteToDraw > -1 && !plot.isQuit())
    {
        plot.setDrawData(oscillators[noteToDraw].getWavetable(), oscillators[noteToDraw].getConverter(), parameter->potential);
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
        
        oscillators[noteNumber].prepareToPlay(noteNumber, sampleRate);// is calling here correct? was not called at all before
        oscillators[noteNumber].noteOn(midiEvent.getVelocity());
        
        if (noteOnCount <= 0)
            noteToDraw = noteNumber;
        noteOnCount++;
    }
    else if (midiEvent.isNoteOff())
    {
        oscillators[midiEvent.getNoteNumber()].noteOff();

        noteOnCount--;
        if (noteOnCount <= 0)
            noteToDraw = -1;
    }
    else if (midiEvent.isAllNotesOff())
    {
        oscillators.forEach([](auto oscillator) {
            oscillator.noteOff();
        });

        noteOnCount = 0;
        noteToDraw = -1;
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
