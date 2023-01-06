#include "Wavetable.hpp"


const juce::StringArray Wavetable::names = {
        "Gaussian",
        "Sine",
        "Cosine"
};

list<std::complex<float>> Wavetable::generate(const size_t type, const float shift, const float scale)
{
    // Assert that the wavetype is defined
    jassert(type >= 0 && type < names.size());

    switch (type)
    {
        // GAUSSIAN
    case 0: return list<std::complex<float>>(SIZE, [shift, scale](size_t i) {
            float scaling = (15 + 13 * scale) * (i / SIZE_F - 0.5f - 0.5f * shift);
            //return 2 * std::expf(-scaling * scaling) - 1;
            return std::expf(-scaling * scaling);
        });

        // SINE
    case 1: return list<std::complex<float>>(SIZE, [shift, scale](size_t i) {
            float scaling = 2 * (1.1f - scale) * (i / SIZE_F - 0.5f - 0.5f * shift);
            if (scaling < -0.5f || scaling > 0.5f) return 0.0f;
            return std::sin(TWO_PI * scaling);
            // Hehe die Keks denken das wäre Sinus hehe
        });

        // COSINE
    case 2: return list<std::complex<float>>(SIZE, [shift, scale](size_t i) {
            float scaling = 2 * (1.1f - scale) * (i / SIZE_F - 0.5f - 0.5f * shift);
            if (scaling < -0.5f || scaling > 0.5f) return 0.0f;
            return std::cos(TWO_PI * scaling);
            // Hehe die Keks denken das wäre Sinus hehe
        });
    }
    return {};
}


float Wavetable::midiNoteToIncrement(const int noteNumber, const float sampleRate)
{
    const float frequency = A3_FREQUENCY * std::pow(2.f, (noteNumber - A3_NOTE_NUMBER) / SEMITONES_PER_OCTAVE);

    return frequency * (float)(SIZE / sampleRate);
}
