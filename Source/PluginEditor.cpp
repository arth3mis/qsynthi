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

#define LINE(i, iconList, componentList, area) iconList[i]->setBounds(trim(area.withTrimmedBottom(area.getHeight() - lineHeight).withTrimmedRight(area.getWidth() - lineHeight), border)); componentList[i]->setBounds(trim(area.removeFromTop(lineHeight).withTrimmedLeft(lineHeight), border));

#define TEXT(textComponent, area) textComponent.setBounds(area.removeFromTop(lineHeight).withTrimmedTop(border/2))


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
    ColourGradient potentialGradient (Colour(0xFF43F7B9), 0.55f * getBounds().getWidth(), 0.25f * getBounds().getHeight(), Colour(0xFF4618D8), 0.45f * getBounds().getWidth(), 1.f * getBounds().getHeight(), false);
    potentialGradient.addColour(0.4, Colour(0xFF00D1BB));
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
    double pathScale = 0.7 * bounds.getHeight() / 2;
    double pathOffset = 1.174 * bounds.getHeight() / 2;
    
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

attack(p, ATTACK_TIME, "s"),
decay(p, DECAY_TIME, "s"),
sustain(p, SUSTAIN_LEVEL, ""),
release(p, RELEASE_TIME, "s"),

filterFrequency(p, FILTER_FREQUENCY, "Hz"),
filterResonance(p, FILTER_RESONANCE, ""),
filterEnvelope(p, FILTER_ENVELOPE, ""),

stereoize(p, STEREO_AMOUNT, "%"),
reverbMix(p, REVERB_MIX, "%"),
gain(p, GAIN, "dB")
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
    sampleTypeImage.setImage(ImageFileFormat::loadFrom(BinaryData::filter_png, BinaryData::filter_pngSize));
    
    
    potentialTypeImage1.setImage(ImageFileFormat::loadFrom(BinaryData::potentialShape_png, BinaryData::potentialShape_pngSize));
    potentialShiftImage1.setImage(ImageFileFormat::loadFrom(BinaryData::potentialShift_png, BinaryData::potentialShift_pngSize));
    potentialScaleImage1.setImage(ImageFileFormat::loadFrom(BinaryData::potentialScale_png, BinaryData::potentialScale_pngSize));
    potentialHeightImage1.setImage(ImageFileFormat::loadFrom(BinaryData::potentialHeight_png, BinaryData::potentialHeight_pngSize));
    potentialTypeImage2.setImage(ImageFileFormat::loadFrom(BinaryData::potentialShape_png, BinaryData::potentialShape_pngSize));
    potentialShiftImage2.setImage(ImageFileFormat::loadFrom(BinaryData::potentialShift_png, BinaryData::potentialShift_pngSize));
    potentialScaleImage2.setImage(ImageFileFormat::loadFrom(BinaryData::potentialScale_png, BinaryData::potentialScale_pngSize));
    potentialHeightImage2.setImage(ImageFileFormat::loadFrom(BinaryData::potentialHeight_png, BinaryData::potentialHeight_pngSize));
    
    
    attackImage.setImage(ImageFileFormat::loadFrom(BinaryData::attack_png, BinaryData::attack_pngSize));
    decayImage.setImage(ImageFileFormat::loadFrom(BinaryData::decay_png, BinaryData::decay_pngSize));
    sustainImage.setImage(ImageFileFormat::loadFrom(BinaryData::sustain_png, BinaryData::sustain_pngSize));
    releaseImage.setImage(ImageFileFormat::loadFrom(BinaryData::release_png, BinaryData::release_pngSize));
    filterFrequencyImage.setImage(ImageFileFormat::loadFrom(BinaryData::filterFreq_png, BinaryData::filterFreq_pngSize));
    filterResonanceImage.setImage(ImageFileFormat::loadFrom(BinaryData::filterRes_png, BinaryData::filterRes_pngSize));
    filterEnvelopeImage.setImage(ImageFileFormat::loadFrom(BinaryData::filterEnv_png, BinaryData::filterEnv_pngSize));
    stereoImage.setImage(ImageFileFormat::loadFrom(BinaryData::stereo_png, BinaryData::stereo_pngSize));
    reverbImage.setImage(ImageFileFormat::loadFrom(BinaryData::reverb_png, BinaryData::reverb_pngSize));
    gainImage.setImage(ImageFileFormat::loadFrom(BinaryData::gian_png, BinaryData::gian_pngSize));
    
    
    
    

    
    setLookAndFeel(waveStyle);
    
    waveComponents.forEach([this](auto* c){ c->setLookAndFeel(this->waveStyle); });
    potentialComponents.forEach([this](auto* c){ c->setLookAndFeel(this->potentialStyle); });
    synthiComponents.forEach([this](auto* c){ c->setLookAndFeel(this->synthiStyle); });
    
    components.forEach([this](auto* c){ this->addAndMakeVisible(c); });
    
    setSize (800, 580);
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
    int lineHeight = (int) (height / 16.f);
    
    auto synthiArea = trim(bounds.removeFromRight(width / 3), border);
    auto visualisationArea = bounds.removeFromTop((int) (lineHeight * (16 - 9)));
    auto waveArea = trim(bounds.removeFromLeft(width / 3), border);
    auto potentialArea = trim(bounds, border);
    
    waveTable.setBounds(visualisationArea);
    
    // WaveArea
    TEXT(waveText, waveArea);
    LINE(0, waveImages, waveComponents, waveArea);
    LINE(1, waveImages, waveComponents, waveArea);
    LINE(2, waveImages, waveComponents, waveArea);
    
    TEXT(simulationText, waveArea);
    LINE(3, waveImages, waveComponents, waveArea);
    LINE(4, waveImages, waveComponents, waveArea);
    LINE(5, waveImages, waveComponents, waveArea);
    LINE(6, waveImages, waveComponents, waveArea);
    
    // Potential Area
    TEXT(potentialText, potentialArea);
    LINE(0, potentialImages, potentialComponents, potentialArea);
    LINE(1, potentialImages, potentialComponents, potentialArea);
    LINE(2, potentialImages, potentialComponents, potentialArea);
    LINE(3, potentialImages, potentialComponents, potentialArea);
    LINE(4, potentialImages, potentialComponents, potentialArea);
    LINE(5, potentialImages, potentialComponents, potentialArea);
    LINE(6, potentialImages, potentialComponents, potentialArea);
    LINE(7, potentialImages, potentialComponents, potentialArea);
    
    // Synthi Area
    TEXT(envelopeText, synthiArea);
    LINE(0, synthiImages, synthiComponents, synthiArea);
    LINE(1, synthiImages, synthiComponents, synthiArea);
    LINE(2, synthiImages, synthiComponents, synthiArea);
    LINE(3, synthiImages, synthiComponents, synthiArea);
    
    synthiArea.removeFromTop(lineHeight);
    
    TEXT(filterText, synthiArea);
    LINE(4, synthiImages, synthiComponents, synthiArea);
    LINE(5, synthiImages, synthiComponents, synthiArea);
    LINE(6, synthiImages, synthiComponents, synthiArea);
    
    synthiArea.removeFromTop(lineHeight);
    
    TEXT(generalText, synthiArea);
    LINE(7, synthiImages, synthiComponents, synthiArea);
    LINE(8, synthiImages, synthiComponents, synthiArea);
    LINE(9, synthiImages, synthiComponents, synthiArea);

}

Rectangle<int> QSynthiAudioProcessorEditor::trim(Rectangle<int> rect, int amount)
{
    
    return rect.withTrimmedTop(amount).withTrimmedRight(amount).withTrimmedBottom(amount).withTrimmedLeft(amount);
}
