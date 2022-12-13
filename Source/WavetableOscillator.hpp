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

#endif /* WavetableOscillator_hpp */

class WavetableOscillator
{
public:
    WavetableOscillator(std::vector<float> waveTable, float frequency, double sampleRate);
    
    float getNextSample();
    void stop();
    bool isPlaying();
    
private:
    std::vector<float> waveTable;
    float indexIncrement;
    double sampleRate;
    
    float index = 0.f;
    float interpolateLinearly();
};
