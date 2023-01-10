#include "Wavetable.hpp"
#include "Parameter.h"


list<float> Wavetable::generate(const size_t type, const float shift, const float scale)
{
    // Assert that the wavetype is defined
    jassert(type >= 0 && type < Parameter::WAVE_TYPES.size());

    switch (type)
    {
        case WaveType::GAUSSIAN: return list<float>(SIZE, [shift, scale](size_t i) {
            float scaledX = (15 + 13 * scale) * (i / SIZE_F - 0.5f - 0.5f * shift);
            return std::expf(- scaledX * scaledX);
        });

        case WaveType::SINE: return list<float>(SIZE, [shift, scale](size_t i) {
            float scaledX = (1.f - scale) * (i / SIZE_F - 0.5f - 0.5f * shift) + 0.5f;
            if (scaledX < 0.f || scaledX > 1.f) return 0.0f;
            return std::sin(TWO_PI * scaledX);
        });

        case WaveType::COSINE: return list<float>(SIZE, [shift, scale](size_t i) {
            float scaledX = (1.f - scale) * (i / SIZE_F - 0.5f - 0.5f * shift) + 0.5f;
            if (scaledX < 0.f || scaledX > 1.f) return 0.0f;
            return std::cos(TWO_PI * scaledX);
        });
            
        case WaveType::PARABOLA: return list<float>(SIZE, [shift, scale](size_t i) {
            float scaledX = i / SIZE_F - 0.5f - 0.5f * shift;
            return 4 * scale / (1 + 3 * shift) * scaledX * scaledX;
        });
    }
    return {};
}


float Wavetable::midiNoteToIncrement(const int noteNumber, const float sampleRate)
{
    const float frequency = A4_FREQUENCY * std::pow(2.f, (noteNumber - A4_NOTE_NUMBER) / SEMITONES_PER_OCTAVE);

    return frequency * (float)(SIZE / sampleRate);
}
