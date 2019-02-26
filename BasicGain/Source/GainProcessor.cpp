#include "GainProcessor.h"
#include "GainEditor.h"

// Instantiate this plugin
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GainProcessor();
}

// Instantiate this plugin's editor/GUI
AudioProcessorEditor* GainProcessor::createEditor()
{
    return new GainEditor(*this);
}

// Constructor: start off assuming stereo input, stereo output
GainProcessor::GainProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput  ("Input",  AudioChannelSet::stereo(), true)
                     .withOutput ("Output", AudioChannelSet::stereo(), true)
                     )
    , valueTreeState(*this, &undoManager)
    , parameters(valueTreeState)
{
    parameters.createAllParameters();
    valueTreeState.state = ValueTree(Identifier(JucePlugin_Name));
}

// Destructor
GainProcessor::~GainProcessor()
{
}

// Prepare to process audio (always called at least once before processBlock)
void GainProcessor::prepareToPlay (double /*sampleRate*/, int /*maxSamplesPerBlock*/)
{
}

// Audio processing finished; release any allocated memory
void GainProcessor::releaseResources()
{
}

// Process one buffer ("block") of data
void GainProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    // disable 
    ScopedNoDenormals noDenormals;

    // apply the same gain factor to all input channels for which there is an output channel
    int channelIndex = 0;
    for (; channelIndex < getTotalNumInputChannels(); channelIndex++)
    {
        const float* pIn = buffer.getReadPointer(channelIndex);
        float* pOut = buffer.getWritePointer(channelIndex);

        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            *pOut++ = *pIn++ * parameters.gain;
        }
    }
    // clear any remaining/excess output channels to zero
    for (; channelIndex < getTotalNumOutputChannels(); channelIndex++)
    {
        buffer.clear(channelIndex, 0, buffer.getNumSamples());
    }
}

// Called by the host when it needs to persist the current plugin state
void GainProcessor::getStateInformation (MemoryBlock& destData)
{
    ScopedPointer<XmlElement> xml = new XmlElement(JucePlugin_Name);
    parameters.putToXml(xml);
    copyXmlToBinary(*xml, destData);
}

// Called by the host before processing, when it needs to restore a saved plugin state
void GainProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    ScopedPointer<XmlElement> xml = getXmlFromBinary(data, sizeInBytes);
    if (xml && xml->hasTagName(JucePlugin_Name))
    {
        parameters.getFromXml(xml);
    }
}