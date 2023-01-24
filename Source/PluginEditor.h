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

    void resized() override;
};

class CustomComboBox : public ComboBox
{
public:
    CustomComboBox(QSynthiAudioProcessor& p, const String parameter, StringArray items) : ComboBox(),
    attachmentToParameter(p.treeState, parameter, *this)
    {
        addItemList(items, 1);
        //setSelectedId();
    }
    
    AudioProcessorValueTreeState::ComboBoxAttachment attachmentToParameter;
};

class CustomLabel : public Label
{
public:
    CustomLabel(const String text) : Label(text, text)
    {
        
    }
    
    void resized() override;
};

class WaveTableComponent : public Component, Timer
{
public:
    QSynthiAudioProcessor& p;
    
    WaveTableComponent(QSynthiAudioProcessor& p) : Timer(), p(p)
    {
        startTimerHz(30);
        logo.setImage(ImageFileFormat::loadFrom(BinaryData::logo_png, BinaryData::logo_pngSize), 1);
        addAndMakeVisible(&logo);
    }
  
    void paint(Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    
private:
    void drawLine(Graphics& g, Rectangle<int> bounds, list<float> values, ColourGradient gradient);
    
    ImageComponent logo;
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
    
    CustomLabel waveText{"Schrödingers Welle"};
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
    ImageComponent waveShiftImage;
    ImageComponent waveScaleImage;
    ImageComponent simulationSpeedImage;
    ImageComponent simulationOffsetImage;
    ImageComponent simulationAccuracyImage;
    //ImageComponent sampleTypeImage;
    
    ImageComponent potentialTypeImage1;
    ImageComponent potentialShiftImage1;
    ImageComponent potentialScaleImage1;
    ImageComponent potentialHeightImage1;
    ImageComponent potentialTypeImage2;
    ImageComponent potentialShiftImage2;
    ImageComponent potentialScaleImage2;
    ImageComponent potentialHeightImage2;
    
    
    list<Component*> waveComponents{
        &waveText,
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
    
    list<Component*> waveImages{
        &waveTypeImage,
        &waveShiftImage,
        &waveScaleImage,
        &simulationSpeedImage,
        &simulationOffsetImage,
        &simulationAccuracyImage
    };
    
    list<Component*> potentialImages{
        &potentialTypeImage1,
        &potentialShiftImage1,
        &potentialScaleImage1,
        &potentialHeightImage1,
        &potentialTypeImage2,
        &potentialShiftImage2,
        &potentialScaleImage2,
        &potentialHeightImage2
    };
    
    list<Component*> components = list<Component*>{&waveTable} + waveComponents + potentialComponents + synthiComponents + waveImages + potentialImages;
    
    Rectangle<int> trim(Rectangle<int> rect, int amount);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QSynthiAudioProcessorEditor)
};
