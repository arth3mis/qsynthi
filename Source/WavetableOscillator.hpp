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
    WavetableOscillator(std::vector<float> waveTable, double sampleRate);
    
    void setFrequency(float frequency);
    
    float getSample();
    void stop();
    bool isPlaying();
    
private:
    std::vector<float> waveTable;
    double sampleRate;
    float index = 0.f;
    float indexIncrement = 0.f;
    
    float interpolateLinearly();
};
