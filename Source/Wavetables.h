/*
  ==============================================================================

    Wavetables.h
    Created: 13 Dec 2022 4:07:32pm
    Author:  Jannis Müller

  ==============================================================================
*/

#pragma once
#include "list.hpp"
#include <functional>

namespace wavetable {

juce::StringArray names = {
    "saw",
    "saw2"
};

constexpr size_t SIZE = 64;

inline list<float> generate(size_t type, float a, float b) {
    switch (type)
    {
        case 0: return list<float>(SIZE, [a,b](size_t i) { return a + b*i; });
    }
    return {};
}


}

//list<float> l = wavetable::generate(0, 1, 1);
