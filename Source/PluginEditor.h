/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Parameter.h"

//==============================================================================
/**
*/
class CustomSlider : public Slider
{
public:
    CustomSlider(QSynthiAudioProcessor& p, const String parameter, const String suffix) : Slider(Slider::SliderStyle::LinearHorizontal, Slider::TextEntryBoxPosition::TextBoxRight),
    attachmentToParameter(p.treeState, parameter, *this)
    {
        setTextValueSuffix(suffix);
    }
    
    AudioProcessorValueTreeState::SliderAttachment attachmentToParameter;

};

class CustomComboBox : public ComboBox
{
public:
    CustomComboBox(QSynthiAudioProcessor& p, const String parameter, StringArray items) : ComboBox(),
    attachmentToParameter(p.treeState, parameter, *this)
    {
        addItemList(items, 1);
        setSelectedId(1);
    }
    
    AudioProcessorValueTreeState::ComboBoxAttachment attachmentToParameter;
};

class WaveTableComponent : public Component, Timer
{
public:
    QSynthiAudioProcessor& p;
    
    WaveTableComponent(QSynthiAudioProcessor& p) : Timer(), p(p)
    {
        startTimerHz(30);
    }
  
    void paint(Graphics& g) override;
    
    void timerCallback() override;
private:
    void drawLine(Graphics& g, Rectangle<int> bounds, list<float> values, ColourGradient gradient);
};


class QSynthiAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    QSynthiAudioProcessorEditor (QSynthiAudioProcessor&);
    ~QSynthiAudioProcessorEditor() override;
    
    
    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    
    QSynthiAudioProcessor& audioProcessor;
    
    LookAndFeel_V4* waveStyle;
    LookAndFeel_V4* potentialStyle;
    LookAndFeel_V4* synthiStyle;
    
    WaveTableComponent waveTable;
    
    CustomComboBox waveType;
    CustomSlider waveShift;
    CustomSlider waveScale;
    CustomSlider simulationSpeed;
    CustomSlider simulationOffset;
    CustomSlider simulationAccuracy;
    CustomComboBox sampleType;
    
    CustomComboBox potentialType1;
    CustomSlider potentialShift1;
    CustomSlider potentialScale1;
    CustomSlider potentialHeight1;
    CustomComboBox potentialType2;
    CustomSlider potentialShift2;
    CustomSlider potentialScale2;
    CustomSlider potentialHeight2;
    
    CustomSlider gain;
    CustomSlider attack;
    CustomSlider decay;
    CustomSlider sustain;
    CustomSlider release;
    CustomSlider stereoize;
    CustomSlider reverbMix;
    
    ImageComponent waveTypeImage;
    
    
    list<Component*> waveComponents{
        &waveType,
        &waveShift,
        &waveScale,
        &simulationSpeed,
        &simulationOffset,
        &simulationAccuracy,
        &sampleType
    };
    
    list<Component*> potentialComponents{
        &potentialType1,
        &potentialShift1,
        &potentialScale1,
        &potentialHeight1,
        &potentialType2,
        &potentialShift2,
        &potentialScale2,
        &potentialHeight2
    };
    
    list<Component*> synthiComponents{
        &gain,
        &attack,
        &decay,
        &sustain,
        &release,
        &stereoize,
        &reverbMix
    };
    
    list<Component*> components = list<Component*>{&waveTable, &waveTypeImage} + waveComponents + potentialComponents + synthiComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QSynthiAudioProcessorEditor)
};
