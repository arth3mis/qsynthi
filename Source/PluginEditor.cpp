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
    setFont(Font("Inter", "Light", getHeight() - 5));
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
    const auto sampleConversion = p.parameter->getSampleConverter();

    if (p.synth->hasDisplayedWavetable())
        waveTable = p.synth->getDisplayedWavetable();
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
    if (values.length() > 0) {
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

voiceCount(p, VOICE_COUNT, ""),
portamento(p, PORTAMENTO, "s"),
stereoize(p, STEREO_AMOUNT, "%"),
reverbMix(p, REVERB_MIX, "%"),
gain(p, GAIN, "dB")
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    Colour highlightFG{ 0xff4C3F50 };
    Colour tooltipFG{ 0xffD6D0DF };
    
    waveStyle = new LookAndFeel_V4(LookAndFeel_V4::ColourScheme{
        0xFF221C24, highlightFG, 0xff141010,
        0x00000000, 0xffffffff, 0xffffffff,
        0xffffffff, 0xFFFF7738, 0xffffffff });
    
    potentialStyle = new LookAndFeel_V4(LookAndFeel_V4::ColourScheme{
        0xFF221C24, highlightFG, 0xff141010,
        0x00000000, 0xffffffff, 0xffffffff,
        0xffffffff, 0xFF00D1BB, 0xffffffff });
    
    synthiStyle = new LookAndFeel_V4(LookAndFeel_V4::ColourScheme{
        0xFF221C24, highlightFG, 0xff141010,
        0x00000000, 0xffffffff, 0xffffffff,
        0xffffffff, 0xFF00CC48, 0xffffffff });
    
    waveTypeImage.setImage(ImageCache::getFromMemory(BinaryData::waveShape_png, BinaryData::waveShape_pngSize));
    waveShiftImage.setImage(ImageCache::getFromMemory(BinaryData::waveShift_png, BinaryData::waveShift_pngSize));
    waveScaleImage.setImage(ImageCache::getFromMemory(BinaryData::waveScale_png, BinaryData::waveScale_pngSize));
    simulationSpeedImage.setImage(ImageCache::getFromMemory(BinaryData::simSpeed_png, BinaryData::simSpeed_pngSize));
    simulationOffsetImage.setImage(ImageCache::getFromMemory(BinaryData::simOffset_png, BinaryData::simOffset_pngSize));
    simulationAccuracyImage.setImage(ImageCache::getFromMemory(BinaryData::simAccuracy_png, BinaryData::simAccuracy_pngSize));
    sampleTypeImage.setImage(ImageCache::getFromMemory(BinaryData::filter_png, BinaryData::filter_pngSize));
    
    
    potentialTypeImage1.setImage(ImageCache::getFromMemory(BinaryData::potentialShape_png, BinaryData::potentialShape_pngSize));
    potentialShiftImage1.setImage(ImageCache::getFromMemory(BinaryData::potentialShift_png, BinaryData::potentialShift_pngSize));
    potentialScaleImage1.setImage(ImageCache::getFromMemory(BinaryData::potentialScale_png, BinaryData::potentialScale_pngSize));
    potentialHeightImage1.setImage(ImageCache::getFromMemory(BinaryData::potentialHeight_png, BinaryData::potentialHeight_pngSize));
    potentialTypeImage2.setImage(ImageCache::getFromMemory(BinaryData::potentialShape_png, BinaryData::potentialShape_pngSize));
    potentialShiftImage2.setImage(ImageCache::getFromMemory(BinaryData::potentialShift_png, BinaryData::potentialShift_pngSize));
    potentialScaleImage2.setImage(ImageCache::getFromMemory(BinaryData::potentialScale_png, BinaryData::potentialScale_pngSize));
    potentialHeightImage2.setImage(ImageCache::getFromMemory(BinaryData::potentialHeight_png, BinaryData::potentialHeight_pngSize));
    
    
    attackImage.setImage(ImageCache::getFromMemory(BinaryData::attack_png, BinaryData::attack_pngSize));
    decayImage.setImage(ImageCache::getFromMemory(BinaryData::decay_png, BinaryData::decay_pngSize));
    sustainImage.setImage(ImageCache::getFromMemory(BinaryData::sustain_png, BinaryData::sustain_pngSize));
    releaseImage.setImage(ImageCache::getFromMemory(BinaryData::release_png, BinaryData::release_pngSize));
    filterFrequencyImage.setImage(ImageCache::getFromMemory(BinaryData::filterFreq_png, BinaryData::filterFreq_pngSize));
    filterResonanceImage.setImage(ImageCache::getFromMemory(BinaryData::filterRes_png, BinaryData::filterRes_pngSize));
    filterEnvelopeImage.setImage(ImageCache::getFromMemory(BinaryData::filterEnv_png, BinaryData::filterEnv_pngSize));
    voiceCountImage.setImage(ImageCache::getFromMemory(BinaryData::voiceCount_png, BinaryData::voiceCount_pngSize));
    portamentoImage.setImage(ImageCache::getFromMemory(BinaryData::portamento_png, BinaryData::portamento_pngSize));
    stereoImage.setImage(ImageCache::getFromMemory(BinaryData::stereo_png, BinaryData::stereo_pngSize));
    reverbImage.setImage(ImageCache::getFromMemory(BinaryData::reverb_png, BinaryData::reverb_pngSize));
    gainImage.setImage(ImageCache::getFromMemory(BinaryData::gian_png, BinaryData::gian_pngSize));
    

    // Fixed tooltip label display
    tooltipLabel.setJustificationType(Justification::centredLeft);
    tooltipLabel.setColour(Label::textColourId, Colour(0xFFCCCCDD));
    addAndMakeVisible(tooltipLabel);
    
    // Bottom bar buttons
    Image linkButtonVersionImage = ImageCache::getFromMemory(BinaryData::buttonVersion_png,
        BinaryData::buttonVersion_pngSize);
    Image linkButtonHelpImage = ImageCache::getFromMemory(BinaryData::buttonHelp_png,
        BinaryData::buttonHelp_pngSize);
    Image linkButtonDonateImage = ImageCache::getFromMemory(BinaryData::buttonDonate_png,
        BinaryData::buttonDonate_pngSize);

    linkButtonVersion.setImages(false, true, true,
        linkButtonVersionImage, 0.0f, highlightFG,  // opaque tint
        linkButtonVersionImage, 1.0f, Colours::transparentBlack,
        linkButtonVersionImage, 0.6f, Colours::transparentBlack);
    linkButtonHelp.setImages(false, true, true,
        linkButtonHelpImage, 0.0f, highlightFG,
        linkButtonHelpImage, 1.0f, Colours::transparentBlack,
        linkButtonHelpImage, 0.6f, Colours::transparentBlack);
    linkButtonDonate.setImages(false, true, true,
        linkButtonDonateImage, 0.0f, highlightFG,
        linkButtonDonateImage, 1.0f, Colours::transparentBlack,
        linkButtonDonateImage, 0.6f, Colours::transparentBlack);

    linkButtonVersion.onClick = [this]() {
        URL(String("https://qsynthi.com/version-check?v=") + JucePlugin_VersionString).launchInDefaultBrowser();
        };
    linkButtonHelp.onClick = [this]() {
        URL(String("https://qsynthi.com/")).launchInDefaultBrowser();
        };
    linkButtonDonate.onClick = [this]() {
        URL(String("https://qsynthi.com/donate")).launchInDefaultBrowser();
        };


    setLookAndFeel(waveStyle);
    
    waveComponents.forEach([this](auto* c){ c->setLookAndFeel(this->waveStyle); });
    potentialComponents.forEach([this](auto* c){ c->setLookAndFeel(this->potentialStyle); });
    synthiComponents.forEach([this](auto* c){ c->setLookAndFeel(this->synthiStyle); });
    
    components.forEach([this](auto* c){ 
        c->addMouseListener(this, false);
        this->addAndMakeVisible(c);
    });
    
    setSize (800, 610);
    setResizable(true, true);
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

    // bottom area background
    //auto bounds = getLocalBounds();
    //int bottomHeight = bounds.getHeight() / bottomBarHeightFraction;
    //auto bottomArea = bounds.removeFromBottom(bottomHeight);
    //g.setColour(Colour(0xff141010));
    //g.fillRect(bottomArea);

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

    int bottomHeight = bounds.getHeight() / 20;
    auto bottomArea = bounds.removeFromBottom(bottomHeight);

    int width = bounds.getWidth();
    int height = bounds.getHeight();
    int border = (width+height) / 250;

    // Bottom Bar Area
    bottomArea = trim(bottomArea, border);
    int bottomButtonSize = bottomArea.getHeight();
    int bottomButtonSpaceW = bottomButtonSize + border;
    bottomArea.removeFromRight(border * 2);  // right padding
    // buttons are ordered right to left
    linkButtonVersion   .setBounds(bottomArea.removeFromRight(bottomButtonSpaceW).withWidth(bottomButtonSize));
    linkButtonHelp      .setBounds(bottomArea.removeFromRight(bottomButtonSpaceW).withWidth(bottomButtonSize));
    linkButtonDonate    .setBounds(bottomArea.removeFromRight(bottomButtonSpaceW).withWidth(bottomButtonSize));
    tooltipLabel.setBounds(bottomArea);
    
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
    
    synthiArea.removeFromTop(lineHeight/2);
    
    TEXT(filterText, synthiArea);
    LINE(4, synthiImages, synthiComponents, synthiArea);
    LINE(5, synthiImages, synthiComponents, synthiArea);
    LINE(6, synthiImages, synthiComponents, synthiArea);
    
    synthiArea.removeFromTop(lineHeight/2);
    
    TEXT(generalText, synthiArea);
    LINE(7, synthiImages, synthiComponents, synthiArea);
    LINE(8, synthiImages, synthiComponents, synthiArea);
    LINE(9, synthiImages, synthiComponents, synthiArea);
    LINE(10, synthiImages, synthiComponents, synthiArea);
    LINE(11, synthiImages, synthiComponents, synthiArea);

}

Rectangle<int> QSynthiAudioProcessorEditor::trim(Rectangle<int> rect, int amount)
{
    return rect.withTrimmedTop(amount).withTrimmedRight(amount).withTrimmedBottom(amount).withTrimmedLeft(amount);
}

void QSynthiAudioProcessorEditor::mouseEnter(const MouseEvent& e)
{
    // Bottom bar buttons
    if (e.eventComponent == &linkButtonVersion)
        showTooltip("Check for updates");
    else if (e.eventComponent == &linkButtonHelp)
        showTooltip("Open website for info and examples");
    else if (e.eventComponent == &linkButtonDonate)
        showTooltip("Donate to support us!");

    // Wave
    else if (e.eventComponent == &waveTypeImage || e.eventComponent == &waveType)
        showTooltip("Wave Shape");
    else if (e.eventComponent == &waveShiftImage || e.eventComponent == &waveShift)
        showTooltip("Wave x-Position");
    else if (e.eventComponent == &waveScaleImage || e.eventComponent == &waveScale)
        showTooltip("Wave Stretch");
    else if (e.eventComponent == &simulationSpeedImage || e.eventComponent == &simulationSpeed)
        showTooltip("Simulation speed");
    else if (e.eventComponent == &simulationOffsetImage || e.eventComponent == &simulationOffset)
        showTooltip("Skip into simulation on start (requires more CPU)");
    else if (e.eventComponent == &simulationAccuracyImage || e.eventComponent == &simulationAccuracy)
        showTooltip("Simulation accuracy (requires more CPU)");
    else if (e.eventComponent == &sampleTypeImage || e.eventComponent == &sampleType)
        showTooltip("How to sonify the complex valued simulation");

    // Potential
    else if (e.eventComponent == &potentialTypeImage1 || e.eventComponent == &potentialType1)
        showTooltip("Potential 1 Shape");
    else if (e.eventComponent == &potentialShiftImage1 || e.eventComponent == &potentialShift1)
        showTooltip("Potential 1 x-Position");
    else if (e.eventComponent == &potentialScaleImage1 || e.eventComponent == &potentialScale1)
        showTooltip("Potential 1 Stretch");
    else if (e.eventComponent == &potentialHeightImage1 || e.eventComponent == &potentialHeight1)
        showTooltip("Potential 1 Height");
    else if (e.eventComponent == &potentialTypeImage2 || e.eventComponent == &potentialType2)
        showTooltip("Potential 2 Shape");
    else if (e.eventComponent == &potentialShiftImage2 || e.eventComponent == &potentialShift2)
        showTooltip("Potential 2 x-Position");
    else if (e.eventComponent == &potentialScaleImage2 || e.eventComponent == &potentialScale2)
        showTooltip("Potential 2 Stretch");
    else if (e.eventComponent == &potentialHeightImage2 || e.eventComponent == &potentialHeight2)
        showTooltip("Potential 2 Height");

    // Synthi
    else if (e.eventComponent == &attackImage || e.eventComponent == &attack)
        showTooltip("Attack time");
    else if (e.eventComponent == &decayImage || e.eventComponent == &decay)
        showTooltip("Decay time");
    else if (e.eventComponent == &sustainImage || e.eventComponent == &sustain)
        showTooltip("Sustain level");
    else if (e.eventComponent == &releaseImage || e.eventComponent == &release)
        showTooltip("Release time");
    else if (e.eventComponent == &filterFrequencyImage || e.eventComponent == &filterFrequency)
        showTooltip("Low-pass Filter Frequency");
    else if (e.eventComponent == &filterResonanceImage || e.eventComponent == &filterResonance)
        showTooltip("Low-pass Filter Resonance");
    else if (e.eventComponent == &filterEnvelopeImage || e.eventComponent == &filterEnvelope)
        showTooltip("Filter Envelope amount");
    else if (e.eventComponent == &voiceCountImage || e.eventComponent == &voiceCount)
        showTooltip("Number of Voices");
    else if (e.eventComponent == &portamentoImage || e.eventComponent == &portamento)
            showTooltip("Portamento time (must set Number of Voices to 1)");
    else if (e.eventComponent == &stereoImage || e.eventComponent == &stereoize)
            showTooltip("Stereo amount");
    else if (e.eventComponent == &reverbImage || e.eventComponent == &reverbMix)
            showTooltip("Reverb amount");
    else if (e.eventComponent == &gainImage || e.eventComponent == &gain)
            showTooltip("Volume");

    // Headings
    else if (e.eventComponent == &waveText)
            showTooltip("Initial wave of the simulation");
    else if (e.eventComponent == &simulationText)
            showTooltip(String(L"Simulation governed by Schrödinger's Equation"));
    else if (e.eventComponent == &potentialText)
            showTooltip("Potential influences how the simulation behaves");
    else if (e.eventComponent == &envelopeText)
            showTooltip("ADSR Envelope for Volume and Filter");
    else if (e.eventComponent == &filterText)
            showTooltip("Low-Pass Filter (24db/Oct Ladder style)");
    else if (e.eventComponent == &generalText)
            showTooltip("Audio Output settings");

}

void QSynthiAudioProcessorEditor::mouseExit(const MouseEvent& e)
{
    hideTooltip();
}

void QSynthiAudioProcessorEditor::showTooltip(const String& text)
{
    tooltipLabel.setText(text, dontSendNotification);
}

void QSynthiAudioProcessorEditor::hideTooltip()
{
    tooltipLabel.setText("", dontSendNotification);
}
