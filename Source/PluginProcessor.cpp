/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
QSynthiAudioProcessor::QSynthiAudioProcessor() 
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    parameter = new Parameter(0.002f, 0.001f, 0.9995f, 0.5f);
    synth = new QSynthi(parameter);
}

QSynthiAudioProcessor::~QSynthiAudioProcessor()
{
    delete synth;       synth = nullptr;
    delete parameter;   parameter = nullptr;
}

//==============================================================================
const String QSynthiAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool QSynthiAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool QSynthiAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool QSynthiAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double QSynthiAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int QSynthiAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int QSynthiAudioProcessor::getCurrentProgram()
{
    return 0;
}

void QSynthiAudioProcessor::setCurrentProgram (int index)
{
}

const String QSynthiAudioProcessor::getProgramName (int index)
{
    return {};
}

void QSynthiAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void QSynthiAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

    synth->prepareToPlay((float) sampleRate);

}

void QSynthiAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool QSynthiAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void QSynthiAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    /*ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    */
    
    buffer.clear();
    
    synth->processBlock(buffer, midiMessages);
}

//==============================================================================
bool QSynthiAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* QSynthiAudioProcessor::createEditor()
{
    // TODO: Entferne den Generischen
    // return new QSynthiAudioProcessorEditor (*this);
    return new GenericAudioProcessorEditor(*this);
}

//==============================================================================
void QSynthiAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void QSynthiAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new QSynthiAudioProcessor();
}

AudioProcessorValueTreeState::ParameterLayout QSynthiAudioProcessor::createParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;
    
    list<AudioProcessorParameter> parameterList = parameter->processorParameters();
    
    parameterList.forEach([layout](auto p) {layout.add(p);});
    
    layout.add(std::make_unique<AudioParameterFloat>(
                                                     ParameterID { "Test", 1 },
                                                     "TEST",
                                                     NormalisableRange<float>(20.f, 20000.f, 0.1f, 0.5f),
                                                     20.f));

    return layout;

}
