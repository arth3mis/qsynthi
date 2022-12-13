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
    void prepareToPlay(double sampleRate);
    void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages);
    
private:
    double sampleRate;
    // Map for all playing oscillators
    // noteNumber -> playing Oscillator on this note
    std::map<int, WavetableOscillator> oscillators;
    
    std::vector<float> generateSineWaveTable();
    void initializeOscillators();
    
    void handleMidiEvent(const MidiMessage& midiEvent);
    float midiNoteNumberToFrequency(int midiNoteNumber);
    void render(AudioBuffer<float>& buffer, int startSample, int endSample);
};
