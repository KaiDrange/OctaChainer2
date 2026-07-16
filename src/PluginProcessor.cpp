#include "PluginProcessor.h"
#include "PluginEditor.h"

OctaChainer2AudioProcessor::OctaChainer2AudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
      )
{
}

OctaChainer2AudioProcessor::~OctaChainer2AudioProcessor() = default;

StateHandler& OctaChainer2AudioProcessor::getStateHandler() noexcept
{
    return stateHandler;
}

const StateHandler& OctaChainer2AudioProcessor::getStateHandler() const noexcept
{
    return stateHandler;
}

AudioPlaybackEngine& OctaChainer2AudioProcessor::getAudioPlaybackEngine() noexcept
{
    return audioPlaybackEngine;
}

const juce::String OctaChainer2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OctaChainer2AudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool OctaChainer2AudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool OctaChainer2AudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double OctaChainer2AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OctaChainer2AudioProcessor::getNumPrograms()
{
    return 1;
}

int OctaChainer2AudioProcessor::getCurrentProgram()
{
    return 0;
}

void OctaChainer2AudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String OctaChainer2AudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void OctaChainer2AudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void OctaChainer2AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void OctaChainer2AudioProcessor::releaseResources()
{
}

bool OctaChainer2AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    {
        return false;
    }

#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    {
        return false;
    }
#endif

    return true;
#endif
}

void OctaChainer2AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;

    for (int channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
    {
        buffer.clear(channel, 0, buffer.getNumSamples());
    }

    // Unity gain pass-through as a minimal, stable starting point.
    constexpr float gain = 1.0f;
    buffer.applyGain(gain);
}

bool OctaChainer2AudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* OctaChainer2AudioProcessor::createEditor()
{
    return new OctaChainer2AudioProcessorEditor(*this);
}

void OctaChainer2AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    std::unique_ptr<juce::XmlElement> xml(stateHandler.createXml());

    if (xml != nullptr)
        copyXmlToBinary(*xml, destData);
}

void OctaChainer2AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml != nullptr)
        stateHandler.restoreFromXml(*xml);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OctaChainer2AudioProcessor();
}

