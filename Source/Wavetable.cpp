#include "Wavetable.hpp"
#include "Parameter.h"


list<cfloat> Wavetable::generate(const size_t type, const float shift, const float scale)
{
    // Assert that the wavetype is defined
    jassert(type >= 0 && type < Parameter::WAVE_TYPES.size());

    switch (type)
    {
        case WaveType::GAUSSIAN:
            return list<cfloat>(SIZE, [shift, scale] (size_t i) {
                
                float min = std::min(gaussianCurve(0, shift, scale), gaussianCurve(1, shift, scale));
                return (gaussianCurve(i / SIZE_F, shift, scale) - min) / (1 - min);
                
            });

        case WaveType::SINE:
            return list<cfloat>(SIZE, [shift, scale] (size_t i) {
                
                float sineScale = pow(7, -scale);
                float factor = (sineScale * (1 + abs(shift))) < 0.5f ? 1 / std::max(sineCurve(0, shift, sineScale), abs(sineCurve(1, shift, sineScale))) : 1;
                return factor * sineCurve(i / SIZE_F, shift, sineScale);
                
            });

        case WaveType::COSINE:
            return list<cfloat>(SIZE, [shift, scale] (size_t i) {
                
                float cosScale = pow(7, -scale);
                if (cosScale * (1 + abs (shift)) < 1) {
                    float offset = 2 / (1 - std::min(cosCurve(0, shift, cosScale), cosCurve(1, shift, cosScale)));
                    return - offset * cosCurve(i / SIZE_F, shift, cosScale) - 1 + offset;
                } else {
                    return -cosCurve(i / SIZE_F, shift, cosScale);
                }
                
            });
            
        case WaveType::PARABOLA:
            return list<cfloat>(SIZE, [shift, scale] (size_t i) {
            
                float max = std::max(parabolaCurve(0, shift, scale), parabolaCurve(1, shift, scale));
                return parabolaCurve(i / SIZE_F, shift, scale) / max;
                
            });
            
        case WaveType::BARRIER:
            return list<cfloat>(SIZE, [shift, scale](size_t i) {
                
                if (i == SIZE / 2) return 99.f;
                // else parabola
                float scaledX = i / SIZE_F - 0.5f - 0.5f * shift;
                return 4 * scale / (1 + 3 * shift) * scaledX * scaledX;
                
        });
            
        case WaveType::SAWTOOTH:
            return list<cfloat>(SIZE, [shift, scale] (size_t i) {
                
                return sawtoothCurve(i / SIZE_F, shift, scale);
                
            });
            
    }
    return {};
}

// Like in Desmos
inline float Wavetable::gaussianCurve(float x, float shift, float scale)
{
    const float e = MathConstants<float>::euler;
    scale = pow(e, -1.75f * scale + 2.75f);
    x = x - 0.5f - 0.5f * shift;
    
    return pow(e, -pow(1 / sqrt(2) * scale * x, 2));
}

inline float Wavetable::sineCurve(float x, float shift, float scale)
{
    x = x - 0.5f - 0.5f * shift;
    x *= scale;
    
    return -(abs(x) <= 0.5f ? sin(TWO_PI * x) : 0);
}

inline float Wavetable::cosCurve(float x, float shift, float scale)
{
    x = x - 0.5f - 0.5f * shift;
    x *= scale;
    
    return (abs(x) <= 0.5f ? cos(TWO_PI * x) : 0);
}

inline float Wavetable::parabolaCurve(float x, float shift, float scale)
{
    scale = 0.25f + 1.75f / (1.f - 1.f / 15.f) * (pow(15.f, scale) - 1.f / 15.f);
    x = x - 0.5f - 0.5f * shift;
    
    return pow(2 * abs(x), scale);
}

inline float Wavetable::sawtoothCurve(float x, float shift, float scale)
{
    scale = sliderScaling(scale, 0.001f, 1.f, 16.f, 0.f);
    x = fmod(x - 0.5f * shift, 1) - 0.5f;

    return -((x > 0) ? 1 : -1) * pow(2 * abs(x), scale);
}




// From my desmos at https://www.desmos.com/calculator/rzdmd1hksl?lang=de
inline float Wavetable::sliderScaling(float sliderValue, float valueAtNeg1, float valueAt0, float valueAt1, float mixLinear)
{
    // Linear
    float leftDifference = abs(valueAt0 - valueAtNeg1);
    float rightDifference =abs(valueAt1 - valueAt0);
    auto linearValue = [valueAtNeg1, valueAt0, valueAt1, leftDifference, rightDifference](float x) {
        return ((valueAt1 - valueAtNeg1 > 0) ? 1 : -1) * std::min(leftDifference, rightDifference) * x + valueAt0;
    };
    
    if (leftDifference == rightDifference) return linearValue(sliderValue);
    
    // Exponential
    float expValueAtNeg1 = valueAtNeg1 - mixLinear * linearValue(-1);
    float expValueAt0    = valueAt0    - mixLinear * linearValue( 0);
    float expValueAt1    = valueAt1    - mixLinear * linearValue( 1);
    
    float base = (expValueAt1 - expValueAt0) / (expValueAt0 - expValueAtNeg1);
    float exponentialValue = expValueAtNeg1 + (expValueAt0 - expValueAtNeg1) / (1 - 1/base) * (pow(base, sliderValue) - 1/base);
    
    return mixLinear * linearValue(sliderValue) + exponentialValue;
    
}


float Wavetable::midiNoteToIncrement(const int noteNumber, const float sampleRate)
{
    const float frequency = A4_FREQUENCY * std::pow(2.f, (noteNumber - A4_NOTE_NUMBER) / SEMITONES_PER_OCTAVE);

    return frequency * (float)(SIZE / sampleRate);
}
