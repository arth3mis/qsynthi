//
//  QSynthi.hpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//

#pragma once

#include "JuceHeader.h"
#include <stdio.h>
#include <vector>
#include "list.hpp"
#include "WavetableOscillator.hpp"
#include "Parameter.h"
#include "WavetablePlot.h"



class QSynthi
{
public:
    QSynthi(Parameter *parameter);
    QSynthi() {}
    
    WavetableOscillator* displayedOscillator = nullptr;
    
    void prepareToPlay(float sampleRate);
    void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages);
    
private:
    bool sustain = false;
    
    Parameter *parameter;
    float sampleRate;

    
    /** Map for all playing oscillators
        noteNumber -> playing Oscillator on this note
     life-cycle of an Oscillator:
        noteOnEvent in handleMidiEvent(...): gets created and inserted in map
        noteOffEvent in handleMidiEvent(...): triggers the release state
        getSample(...): releases the sound
        processBlock(...) cleanup: checks for every oscillator if it thinks it's done with it's life cycle and removes if it's the case
     */
    mutable_list<WavetableOscillator> oscillators;
    mutable_list<WavetableOscillator*> playingOscillators;
    mutable_list<WavetableOscillator*> sleepingOscillators;
    
    mutable_list<int> stolenNotes;
    mutable_list<int> sustainedNotes;
    
    Reverb reverb;
    
    void noteOff(int noteNumber);
    void handleMidiEvent(const MidiMessage& midiEvent);
    void render(AudioBuffer<float>& buffer, int startSample, int endSample);
};
