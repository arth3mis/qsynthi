/*
  ==============================================================================

    Parameter.cpp
    Created: 17 Dec 2022 7:03:54pm
    Author:  Jannis Müller

  ==============================================================================
*/

#include "Parameter.h"

list<AudioProcessorParameter> Parameter::processorParameters() {
    list<AudioProcessorParameter> pList = list<AudioProcessorParameter>();
    
    // TODO: Call init() in PluginProcessor
    // TODO: Call update() in PluginProcessor
    
    
    pList += pList.append();
    
    return parameterList;
}

void Parameter::update(std::function<float(StringRef)> getParameter) {
    
}
