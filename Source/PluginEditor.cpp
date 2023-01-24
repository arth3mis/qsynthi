/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Wavetable.hpp"
#include "BinaryData.h"

#define FOR_EACH(list) for (size_t i = 0; i < list.length(); i++) list[i]


void CustomSlider::resized()
{
    Slider::resized();
    setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxRight, false, getWidth()/4, getHeight());
}

void CustomLabel::resized()
{
    Label::resized();
    setFont(Font("Inter", "Thin", getHeight() - 5));
}


void WaveTableComponent::paint(Graphics& g)
{
    g.fillAll(Colour(0xff141010));
    
    // Draw Potential
    ColourGradient potentialGradient (Colour(0xFF3AF4D0), 0.55f * getBounds().getWidth(), 0.33f * getBounds().getHeight(), Colour(0xFF4618D8), 0.45f * getBounds().getWidth(), 1.f * getBounds().getHeight(), false);
    potentialGradient.addColour(0.5, Colour(0xFF00D1BB));
    drawLine(g, getBounds(), p.parameter->potential, potentialGradient);
    
    
    // Draw Wave
    ColourGradient waveGradient (Colour(0xFFF7BD2A), 0.55f * getBounds().getWidth(), 0.25f * getBounds().getHeight(), Colour(0xFFFF3747), 0.45f * getBounds().getWidth(), getBounds().getHeight(), false);
    waveGradient.addColour(0.5, Colour(0xFFFF7738));
    
    list<cfloat> waveTable;
    auto sampleConversion = p.parameter->getSampleConverter();

    if (p.synth->displayedOscillator != nullptr)
        waveTable = p.synth->displayedOscillator->waveTable;
    else
        waveTable = Wavetable::generate(p.parameter->waveTypeNumber, p.parameter->waveShift, p.parameter->waveScale);
    
    drawLine(g, getBounds(), waveTable.mapTo(sampleConversion), waveGradient);
    
}

void WaveTableComponent::resized()
{
    Component::resized();
    logo.setBounds(getWidth()/70, getHeight()/25, getWidth(), getHeight()/7);
}

void WaveTableComponent::drawLine(Graphics& g, Rectangle<int> bounds, list<float> values, ColourGradient gradient)
{
    double stepSize = (double)bounds.getWidth() / (values.length()-1);
    double pathScale = 0.75 * bounds.getHeight() / 2;
    double pathOffset = 1.25 * bounds.getHeight() / 2;
    
    auto lastX = 0;
    auto lastY = -pathScale * values[0] + pathOffset;
    Path path;
    path.startNewSubPath(lastX, lastY);
    for (auto i = 1; i < values.length(); ++i)
    {
        path.lineTo(i * stepSize, -pathScale * values[i] + pathOffset);
    }
    
    g.setGradientFill(gradient);
    g.strokePath(path, PathStrokeType(5.f, PathStrokeType::JointStyle::beveled, PathStrokeType::EndCapStyle::rounded));
}

void WaveTableComponent::timerCallback() {
    repaint();
}



//==============================================================================
QSynthiAudioProcessorEditor::QSynthiAudioProcessorEditor (QSynthiAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p),

waveTable(p),

waveType(p, WAVE_TYPE, Parameter::WAVE_TYPES),
waveShift(p, WAVE_SHIFT, ""),
waveScale(p, WAVE_SCALE, ""),
simulationSpeed(p, SIMULATION_SPEED, " x"),
simulationOffset(p, SIMULATION_OFFSET, ""),
simulationAccuracy(p, ACCURACY, ""),
sampleType(p, SAMPLE_TYPE, Parameter::SAMPLE_TYPES),

potentialType1(p, POTENTIAL_TYPE1, Parameter::WAVE_TYPES),
potentialShift1(p, POTENTIAL_SHIFT1, ""),
potentialScale1(p, POTENTIAL_SCALE1, ""),
potentialHeight1(p, POTENTIAL_AMOUNT1, ""),
potentialType2(p, POTENTIAL_TYPE2, Parameter::WAVE_TYPES),
potentialShift2(p, POTENTIAL_SHIFT2, ""),
potentialScale2(p, POTENTIAL_SCALE2, ""),
potentialHeight2(p, POTENTIAL_AMOUNT2, ""),

gain(p, GAIN, " dB"),
attack(p, ATTACK_TIME, " ms"),
decay(p, DECAY_TIME, " ms"),
sustain(p, SUSTAIN_LEVEL, " dB"),
release(p, RELEASE_TIME, " ms"),
stereoize(p, STEREO_AMOUNT, " %"),
reverbMix(p, REVERB_MIX, " %")
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
    
    waveTypeImage.setImage(ImageFileFormat::loadFrom(BinaryData::waveShape_png, BinaryData::waveShape_pngSize));
    waveShiftImage.setImage(ImageFileFormat::loadFrom(BinaryData::waveShift_png, BinaryData::waveShift_pngSize));
    waveScaleImage.setImage(ImageFileFormat::loadFrom(BinaryData::waveScale_png, BinaryData::waveScale_pngSize));
    simulationSpeedImage.setImage(ImageFileFormat::loadFrom(BinaryData::simSpeed_png, BinaryData::simSpeed_pngSize));
    simulationOffsetImage.setImage(ImageFileFormat::loadFrom(BinaryData::simOffset_png, BinaryData::simOffset_pngSize));
    simulationAccuracyImage.setImage(ImageFileFormat::loadFrom(BinaryData::simAccuracy_png, BinaryData::simAccuracy_pngSize));
    //sampleTypeImage.setImage(ImageFileFormat::loadFrom(BinaryData::waveShape_png, BinaryData::waveShape_pngSize));
    
    
    potentialTypeImage1.setImage(ImageFileFormat::loadFrom(BinaryData::potentialShape_png, BinaryData::potentialShape_pngSize));
    potentialShiftImage1.setImage(ImageFileFormat::loadFrom(BinaryData::potentialShift_png, BinaryData::potentialShift_pngSize));
    potentialScaleImage1.setImage(ImageFileFormat::loadFrom(BinaryData::potentialScale_png, BinaryData::potentialScale_pngSize));
    potentialHeightImage1.setImage(ImageFileFormat::loadFrom(BinaryData::potentialHeight_png, BinaryData::potentialHeight_pngSize));
    potentialTypeImage2.setImage(ImageFileFormat::loadFrom(BinaryData::potentialShape_png, BinaryData::potentialShape_pngSize));
    potentialShiftImage2.setImage(ImageFileFormat::loadFrom(BinaryData::potentialShift_png, BinaryData::potentialShift_pngSize));
    potentialScaleImage2.setImage(ImageFileFormat::loadFrom(BinaryData::potentialScale_png, BinaryData::potentialScale_pngSize));
    potentialHeightImage2.setImage(ImageFileFormat::loadFrom(BinaryData::potentialHeight_png, BinaryData::potentialHeight_pngSize));
    
    
    
    

    
    setLookAndFeel(waveStyle);
    
    waveComponents.forEach([this](auto* c){ c->setLookAndFeel(this->waveStyle); });
    potentialComponents.forEach([this](auto* c){ c->setLookAndFeel(this->potentialStyle); });
    synthiComponents.forEach([this](auto* c){ c->setLookAndFeel(this->synthiStyle); });
    
    components.forEach([this](auto* c){ this->addAndMakeVisible(c); });
    
    setSize (900, 700);
    setResizable(true, false);
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
    int border = (width+height) / 250;
    
    // Area for components
    auto synthiArea = trim(bounds.removeFromRight(width / 3), border);
    auto visualisationArea = bounds.removeFromTop(height / 2);
    auto waveArea = trim(bounds.removeFromLeft(width / 3), border);
    auto potentialArea = trim(bounds, border);
    
    waveTable.setBounds(visualisationArea);
    
    // WaveArea
    int waveComponentHeight = waveArea.getHeight() / 8;
    auto waveImageBounds = waveArea.removeFromLeft(waveComponentHeight);
    
    FOR_EACH(waveImages)->setBounds(trim(waveImageBounds.removeFromTop(waveComponentHeight), border));
    FOR_EACH(waveComponents)->setBounds(trim(waveArea.removeFromTop(waveComponentHeight), border));
    
    // Potential Area
    int potentialComponentHeight = potentialArea.getHeight() / 8;
    auto potentialImageBounds = potentialArea.removeFromLeft(waveComponentHeight);
    FOR_EACH(potentialImages)->setBounds(trim(potentialImageBounds.removeFromTop(potentialComponentHeight), border));
    FOR_EACH(potentialComponents)->setBounds(trim(potentialArea.removeFromTop(potentialComponentHeight), border));
    
    // Synthi Area
    int synthiComponentHeight = synthiArea.getHeight() / 16;
    FOR_EACH(synthiComponents)->setBounds(trim(synthiArea.removeFromTop(synthiComponentHeight), border));
    
}

Rectangle<int> QSynthiAudioProcessorEditor::trim(Rectangle<int> rect, int amount)
{
    
    return rect.withTrimmedTop(amount).withTrimmedRight(amount).withTrimmedBottom(amount).withTrimmedLeft(amount);
}
