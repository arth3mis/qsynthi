//
//  WavetableOscillator.hpp
//  QSynthi
//
//  Created by Jannis Müller on 12.12.22.
//

#ifndef WavetableOscillator_hpp
#define WavetableOscillator_hpp

#include <stdio.h>
#include <vector>
#include "list.hpp"

#endif /* WavetableOscillator_hpp */

class WavetableOscillator
{
public:
    WavetableOscillator(list<float> waveTable, int midiNote, double sampleRate);
    
    /**
            Returns the next Sample and ajusts the phase accordingly.
     */
    float getNextSample();
    void noteOff();
    bool isDone();
    
private:
    std::vector<float> waveTable;
    float indexIncrement;
    double sampleRate;
    
    float index = 0.f;
    float interpolateLinearly();
    
    float midiNoteNumberToIncrement(int midiNoteNumber);
};
