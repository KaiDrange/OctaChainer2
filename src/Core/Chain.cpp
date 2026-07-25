#include "Chain.h"

#include <algorithm>

void Chain::clear()
{
    audioClip.reset();
    segments.clear();
    bitDepth = 0;
}

int Chain::getChainStartIndex(const StateHandler& stateHandler, const int chainSliceCount)
{
    const auto selectedSliceIndex = stateHandler.getSelectedSliceIndex();
    if (selectedSliceIndex < 0)
        return 0;

    return (selectedSliceIndex / chainSliceCount) * chainSliceCount;
}

bool Chain::isValid() const noexcept
{
    return audioClip != nullptr && audioClip->isValid();
}

std::shared_ptr<AudioClip> Chain::getAudioClip() const noexcept
{
    static AudioClip emptyClip;
    return audioClip != nullptr ? audioClip : std::make_shared<AudioClip>(emptyClip);
}

const juce::AudioBuffer<float>& Chain::getAudioData() const noexcept
{
    return getAudioClip()->getAudioData();
}

double Chain::getSampleRate() const noexcept
{
    return audioClip != nullptr ? audioClip->getSampleRate() : 0.0;
}

int Chain::getBitDepth() const noexcept
{
    return bitDepth;
}

const std::vector<Chain::Segment>& Chain::getSegments() const noexcept
{
    return segments;
}

bool Chain::loadSliceRange(const StateHandler& stateHandler, const juce::ValueTree& sliceTree,
                           juce::AudioBuffer<float>& destination, double& sampleRate)
{
    const auto numChannels = static_cast<int>(sliceTree.getProperty(stateHandler.sliceChannelsId, 0));
    sampleRate = static_cast<double>(sliceTree.getProperty(stateHandler.sliceSamplerateId, 0.0));
    const auto numSamples = static_cast<juce::int64>(sliceTree.getProperty(stateHandler.sliceNumSamplesId, 0));
    const auto* audioDataValue = sliceTree.getPropertyPointer(stateHandler.sliceAudioDataId);

    if (numChannels <= 0 || sampleRate <= 0.0 || numSamples <= 0 || audioDataValue == nullptr)
        return false;

    const auto* audioDataBlock = audioDataValue->getBinaryData();
    if (audioDataBlock == nullptr)
        return false;

    const auto expectedBytes = static_cast<size_t>(numChannels) * static_cast<size_t>(numSamples) * sizeof(float);
    if (audioDataBlock->getSize() < expectedBytes)
        return false;

    const auto rangeStart = juce::jlimit<juce::int64>(0, numSamples, sliceTree.getProperty(stateHandler.sliceStartSampleId, 0));
    const auto rangeEnd = juce::jlimit(rangeStart, numSamples, static_cast<juce::int64>(sliceTree.getProperty(stateHandler.sliceEndSampleId, numSamples)));
    const auto rangeLength = rangeEnd - rangeStart;
    if (rangeLength <= 0)
        return false;

    const auto sourceSamplesPerChannel = static_cast<size_t>(numSamples);
    const auto segmentSamplesPerChannel = static_cast<int>(rangeLength);
    const auto* samples = static_cast<const float*>(audioDataBlock->getData());

    destination.setSize(numChannels, segmentSamplesPerChannel, false, false, true);
    destination.clear();

    for (int channel = 0; channel < numChannels; ++channel)
    {
        const auto* source = samples + static_cast<size_t>(channel) * sourceSamplesPerChannel + static_cast<size_t>(rangeStart);
        destination.copyFrom(channel, 0, source, segmentSamplesPerChannel);
    }

    return true;
}

bool Chain::resampleSliceToTargetRate(const juce::AudioBuffer<float>& source, const double sourceSampleRate,
                                      const double targetSampleRate, juce::AudioBuffer<float>& destination)
{
    if (source.getNumChannels() <= 0 || source.getNumSamples() <= 0 || sourceSampleRate <= 0.0 || targetSampleRate <= 0.0)
        return false;

    if (sourceSampleRate == targetSampleRate)
    {
        destination.makeCopyOf(source);
        return true;
    }

    const auto outputSampleCount = juce::jmax(1, juce::roundToInt(static_cast<double>(source.getNumSamples())
                                                                   * targetSampleRate / sourceSampleRate));
    destination.setSize(source.getNumChannels(), outputSampleCount, false, false, true);
    destination.clear();

    const auto speedRatio = sourceSampleRate / targetSampleRate;
    for (int channel = 0; channel < source.getNumChannels(); ++channel)
    {
        juce::LagrangeInterpolator interpolator;
        interpolator.reset();
        interpolator.process(speedRatio,
                             source.getReadPointer(channel),
                             destination.getWritePointer(channel),
                             outputSampleCount);
    }

    return true;
}

void Chain::rebuild(const StateHandler& stateHandler, const double targetSampleRate)
{
    clear();

    const auto numSlices = stateHandler.getNumSlices();
    if (numSlices <= 0)
        return;

    const auto chainGroupSize = juce::jmax(1, stateHandler.getStateValue<int>(stateHandler.chainMaxLengthId,
                                                                              static_cast<int>(StateHandler::chainMaxLengthValue.defaultValue)));
    const auto startIndex = getChainStartIndex(stateHandler, chainGroupSize);
    const auto endIndex = juce::jmin(numSlices, startIndex + chainGroupSize);
    if (startIndex >= endIndex)
        return;

    struct RenderedSlice
    {
        juce::AudioBuffer<float> audioData;
        int startSample = 0;
        int sampleCount = 0;
    };

    int outputChannelCount = 0;
    int totalSampleCount = 0;
    std::vector<RenderedSlice> renderedSlices;
    renderedSlices.reserve(static_cast<size_t>(endIndex - startIndex));

    for (int sliceIndex = startIndex; sliceIndex < endIndex; ++sliceIndex)
    {
        const auto sliceTree = stateHandler.getSliceTree(sliceIndex);
        if (! sliceTree.isValid())
            continue;

        juce::AudioBuffer<float> sliceBuffer;
        double sourceSampleRate = 0.0;
        if (! loadSliceRange(stateHandler, sliceTree, sliceBuffer, sourceSampleRate))
            continue;

        juce::AudioBuffer<float> renderedBuffer;
        if (! resampleSliceToTargetRate(sliceBuffer, sourceSampleRate, targetSampleRate, renderedBuffer))
            continue;

        const auto renderedSampleCount = renderedBuffer.getNumSamples();
        if (renderedSampleCount <= 0)
            continue;

        const auto startSample = totalSampleCount;
        totalSampleCount += renderedSampleCount;
        outputChannelCount = juce::jmax(outputChannelCount, renderedBuffer.getNumChannels());
        renderedSlices.push_back({ std::move(renderedBuffer), startSample, renderedSampleCount });
    }

    if (renderedSlices.empty() || outputChannelCount <= 0 || totalSampleCount <= 0)
    {
        clear();
        return;
    }

    juce::AudioBuffer<float> output;
    output.setSize(outputChannelCount, totalSampleCount, false, false, true);
    output.clear();

    segments.reserve(renderedSlices.size());
    for (size_t i = 0; i < renderedSlices.size(); ++i)
    {
        const auto& renderedSlice = renderedSlices[i].audioData;
        const auto sourceChannels = renderedSlice.getNumChannels();
        const auto sourceSamples = renderedSlices[i].sampleCount;
        segments.push_back({ static_cast<int>(startIndex + static_cast<int>(i)),
                             renderedSlices[i].startSample,
                             sourceSamples });

        for (int channel = 0; channel < outputChannelCount; ++channel)
        {
            const auto sourceChannel = juce::jlimit(0, sourceChannels - 1, channel);
            output.copyFrom(channel, renderedSlices[i].startSample, renderedSlice, sourceChannel, 0, sourceSamples);
        }
    }

    audioClip = std::make_shared<AudioClip>(std::move(output), targetSampleRate);
}
