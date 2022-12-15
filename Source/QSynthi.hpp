//
//  QSynthi.hpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//

#ifndef QSynthi_hpp
#define QSynthi_hpp

#include <stdio.h>
#include <vector>
#include <map>
#include "JuceHeader.h"
#include "WavetableOscillator.hpp"

#endif /* QSynthi_hpp */


class QSynthi
{
public:
    void prepareToPlay(float sampleRate);
    void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages);
    
private:
    float sampleRate;
    
    /** Map for all playing oscillators
        noteNumber -> playing Oscillator on this note
     life-cycle of an Oscillator:
        noteOnEvent in handleMidiEvent(...): gets created and inserted in map
        noteOffEvent in handleMidiEvent(...): triggers the release state
        getSample(...): releases the sound
        processBlock(...) cleanup: checks for every oscillator if it thinks it's done with it's life cycle and removes if it's the case
     */
    std::map<int, WavetableOscillator*> oscillators;
    
    std::vector<float> generateSineWaveTable();
    void initializeOscillators();
    
    void handleMidiEvent(const MidiMessage& midiEvent);
    void render(AudioBuffer<float>& buffer, int startSample, int endSample);
};
