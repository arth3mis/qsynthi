/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Wavetable.hpp"

#define FOR_EACH(list) for (size_t i = 0; i < list.length(); i++) list[i]


void WaveTableComponent::paint(Graphics& g)
{
    g.fillAll(Colour(0xff141010));
    
    // Draw Potential
    ColourGradient potentialGradient = ColourGradient();
    potentialGradient.addColour(0, Colour(0xFF9F1CA9));
    potentialGradient.addColour(0.5, Colour(0xFF00669E));
    potentialGradient.addColour(1, Colour(0xFF00D1BB));
    drawLine(g, getBounds(), p.parameter->potential, potentialGradient);
    
    // Draw Wave
    ColourGradient waveGradient = ColourGradient();
    waveGradient.addColour(0, Colour(0xFFFF3747));
    waveGradient.addColour(0.5, Colour(0xFFFF7738));
    waveGradient.addColour(1, Colour(0xFFF7BD2A));
    
    list<float> waveTable;
    if (p.synth->displayedOscillator != nullptr)
    {
        auto sampleConversion = p.synth->displayedOscillator->getConverter();
        auto table = p.synth->displayedOscillator->waveTable;
        waveTable = table.mapTo(sampleConversion);
    }
    else
    {
        waveTable = Wavetable::generate(p.parameter->waveTypeNumber, p.parameter->waveShift, p.parameter->waveScale);
    }
    
    drawLine(g, getBounds(), waveTable, waveGradient);
    
}

void WaveTableComponent::drawLine(Graphics& g, Rectangle<int> bounds, list<float> values, ColourGradient gradient)
{
    auto pathWidth = bounds.getWidth();
    auto stepSize = pathWidth / (values.length()-1);
    auto pathScale = 0.6 * bounds.getHeight() / 2;
    auto pathOffset = 1.25 * bounds.getHeight() / 2;
    
    auto lastX = 0;
    auto lastY = -pathScale * values[0] + pathOffset;
    
    for (auto i = 1; i < values.length(); ++i)
    {
        double newX = i * stepSize;
        double newY = -pathScale * values[i] + pathOffset;
        
        g.setColour(gradient.getColourAtPosition(0.25 * (values[i-1] + values[i]) + 0.5));
//        g.setColour(Colour(0xFFFF7738));
        g.drawLine(lastX, lastY, newX, newY, 4.5f);
        
        lastX = newX;
        lastY = newY;
    }
}

void WaveTableComponent::timerCallback() {
    repaint();
}



//==============================================================================
QSynthiAudioProcessorEditor::QSynthiAudioProcessorEditor (QSynthiAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p),

waveTable(p),

waveType(p, WAVE_TYPE, Parameter::WAVE_TYPES),
waveShift(p, WAVE_SHIFT),
waveScale(p, WAVE_SCALE),
simulationSpeed(p, SIMULATION_SPEED),
simulationOffset(p, SIMULATION_OFFSET),
simulationAccuracy(p, ACCURACY),
sampleType(p, SAMPLE_TYPE, Parameter::SAMPLE_TYPES),

potentialType1(p, POTENTIAL_TYPE1, Parameter::WAVE_TYPES),
potentialShift1(p, POTENTIAL_SHIFT1),
potentialScale1(p, POTENTIAL_SCALE1),
potentialHeight1(p, POTENTIAL_AMOUNT1),
potentialType2(p, POTENTIAL_TYPE2, Parameter::WAVE_TYPES),
potentialShift2(p, POTENTIAL_SHIFT2),
potentialScale2(p, POTENTIAL_SCALE2),
potentialHeight2(p, POTENTIAL_AMOUNT2),

gain(p, GAIN),
attack(p, ATTACK_TIME),
decay(p, DECAY_TIME),
sustain(p, SUSTAIN_LEVEL),
release(p, RELEASE_TIME),
stereoize(p, STEREO_AMOUNT),
reverbMix(p, REVERB_MIX)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    waveStyle = new LookAndFeel_V4(LookAndFeel_V4::ColourScheme{
        0xFF221C24, 0xff4C3F50, 0xff141010,
        0x00000000, 0xffffffff, 0xffffffff,
        0xffffffff, 0xFFFF7738, 0xffffffff });
    
    potentialStyle = new LookAndFeel_V4(LookAndFeel_V4::ColourScheme{
        0xFF221C24, 0xff4C3F50, 0xff141010,
        0x00000000, 0xffffffff, 0xffffffff,
        0xffffffff, 0xFF00D1BB, 0xffffffff });
    
    synthiStyle = new LookAndFeel_V4(LookAndFeel_V4::ColourScheme{
        0xFF221C24, 0xff4C3F50, 0xff141010,
        0x00000000, 0xffffffff, 0xffffffff,
        0xffffffff, 0xFF00CC48, 0xffffffff });
    

    setLookAndFeel(waveStyle);
    
    waveComponents.forEach([this](auto* c){ c->setLookAndFeel(this->waveStyle); });
    potentialComponents.forEach([this](auto* c){ c->setLookAndFeel(this->potentialStyle); });
    synthiComponents.forEach([this](auto* c){ c->setLookAndFeel(this->synthiStyle); });
    
    components.forEach([this](auto* c){ this->addAndMakeVisible(c); });

    setSize (900, 700);
}

QSynthiAudioProcessorEditor::~QSynthiAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    components.forEach([this](auto* c){ c->setLookAndFeel(nullptr); });
    
    delete waveStyle;
    delete potentialStyle;
    delete synthiStyle;
    waveStyle = nullptr;
    potentialStyle = nullptr;
    synthiStyle = nullptr;
}

//==============================================================================
void QSynthiAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
/*
    g.setColour (Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);*/

}

void QSynthiAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    // Get Bounds
    auto bounds = getLocalBounds();
    int width = bounds.getWidth();
    int height = bounds.getHeight();
    
    // Area for components
    auto synthiArea = bounds.removeFromRight(width / 3);
    auto visualisationArea = bounds.removeFromTop(height / 2);
    auto waveArea = bounds.removeFromLeft(width / 3);
    auto potentialArea = bounds;
    
    waveTable.setBounds(visualisationArea);
    
    // WaveArea
    int waveComponentHeight = waveArea.getHeight() / 8;
    FOR_EACH(waveComponents)->setBounds(waveArea.removeFromTop(waveComponentHeight));
    
    // Potential Area
    int potentialComponentHeight = potentialArea.getHeight() / 8;
    FOR_EACH(potentialComponents)->setBounds(potentialArea.removeFromTop(potentialComponentHeight));
    
    // Synthi Arey
    int synthiComponentHeight = synthiArea.getHeight() / 16;
    FOR_EACH(synthiComponents)->setBounds(synthiArea.removeFromTop(synthiComponentHeight));
    
}
