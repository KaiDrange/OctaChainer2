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
    juce::ignoreUnused(samplesPerBlock);
    audioPlaybackEngine.deviceSampleRate = sampleRate;
    audioPlaybackEngine.deviceChannelCount = getTotalNumOutputChannels();
}

void OctaChainer2AudioProcessor::releaseResources()
{
    audioPlaybackEngine.stop();
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

    const auto numInputChannels = getTotalNumInputChannels();
    const auto numOutputChannels = getTotalNumOutputChannels();
    const auto numSamples = buffer.getNumSamples();

    for (int channel = numInputChannels; channel < numOutputChannels; ++channel)
        buffer.clear(channel, 0, numSamples);

    juce::AudioBuffer<float> playbackBuffer(buffer.getNumChannels(), numSamples);
    playbackBuffer.clear();

    audioPlaybackEngine.ProcessBlock(playbackBuffer);

    const auto channelsToMix = juce::jmin(buffer.getNumChannels(), playbackBuffer.getNumChannels());
    for (int channel = 0; channel < channelsToMix; ++channel)
        buffer.addFrom(channel, 0, playbackBuffer, channel, 0, numSamples);
}

bool OctaChainer2AudioProcessor::hasEditor() const
{
    return true;
}

void OctaChainer2AudioProcessor::editorDestroyed()
{
    audioPlaybackEngine.stop();
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

