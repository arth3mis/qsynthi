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
    WavetableOscillator(int waveType, float waveShift, float waveScale, int midiNote, float sampleRate);
    WavetableOscillator() {}
    
    /**
            Returns the next Sample and ajusts the phase accordingly.
     */
    float getNextSample();
    void noteOff();
    bool isDone() const;
    
private:
    list<float> waveTable;
    float phaseIncrement;
    
    /**
    phase from 0 to wavetable::SIZE
     */
    float phase = 0.f;

};
