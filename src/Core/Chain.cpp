#include "Chain.h"


void Chain::clear()
{
    audioClip.reset();
    segments.clear();
}

int Chain::getChainStartIndex(const StateHandler& stateHandler, const int chainSliceCount)
{
    const auto selectedSliceIndex = stateHandler.getSelectedSliceIndex();
    if (selectedSliceIndex < 0)
        return 0;

    return (selectedSliceIndex / chainSliceCount) * chainSliceCount;
}

int Chain::getChainStartIndex(const juce::ValueTree& stateTree, const int chainSliceCount)
{
    const auto dataTree = stateTree.getChildWithName(StateHandler::dataId);
    if (! dataTree.isValid())
        return 0;

    const auto selectedSliceIndex = static_cast<int>(dataTree.getProperty(StateHandler::selectedSliceId, -1));
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

const std::vector<Chain::Segment>& Chain::getSegments() const noexcept
{
    return segments;
}

bool Chain::loadSliceRange(const juce::ValueTree& sliceTree,
                           juce::AudioBuffer<float>& destination, double& sampleRate)
{
    const auto numChannels = static_cast<int>(sliceTree.getProperty(StateHandler::sliceChannelsId, 0));
    sampleRate = static_cast<double>(sliceTree.getProperty(StateHandler::sliceSamplerateId, 0.0));
    const auto numSamples = static_cast<juce::int64>(sliceTree.getProperty(StateHandler::sliceNumSamplesId, 0));
    const auto* audioDataValue = sliceTree.getPropertyPointer(StateHandler::sliceAudioDataId);

    if (numChannels <= 0 || sampleRate <= 0.0 || numSamples <= 0 || audioDataValue == nullptr)
        return false;

    const auto* audioDataBlock = audioDataValue->getBinaryData();
    if (audioDataBlock == nullptr)
        return false;

    const auto expectedBytes = static_cast<size_t>(numChannels) * static_cast<size_t>(numSamples) * sizeof(float);
    if (audioDataBlock->getSize() < expectedBytes)
        return false;

    const auto rangeStart = juce::jlimit<juce::int64>(0, numSamples, sliceTree.getProperty(StateHandler::sliceStartSampleId, 0));
    const auto rangeEnd = juce::jlimit(rangeStart, numSamples, static_cast<juce::int64>(sliceTree.getProperty(StateHandler::sliceEndSampleId, numSamples)));
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

bool Chain::renderBufferToTargetChannelCount(const juce::AudioBuffer<float>& source, const int targetChannelCount,
                                             juce::AudioBuffer<float>& destination)
{
    if (source.getNumChannels() <= 0 || source.getNumSamples() <= 0 || targetChannelCount <= 0)
        return false;

    if (source.getNumChannels() == targetChannelCount)
    {
        destination.makeCopyOf(source);
        return true;
    }

    destination.setSize(targetChannelCount, source.getNumSamples(), false, false, true);
    destination.clear();

    if (targetChannelCount == 1)
    {
        const auto mixGain = 1.0f / static_cast<float>(source.getNumChannels());
        for (int channel = 0; channel < source.getNumChannels(); ++channel)
            destination.addFrom(0, 0, source, channel, 0, source.getNumSamples(), mixGain);

        return true;
    }

    if (source.getNumChannels() == 1)
    {
        for (int channel = 0; channel < targetChannelCount; ++channel)
            destination.copyFrom(channel, 0, source, 0, 0, source.getNumSamples());

        return true;
    }

    for (int channel = 0; channel < targetChannelCount; ++channel)
    {
        const auto sourceChannel = juce::jmin(channel, source.getNumChannels() - 1);
        destination.copyFrom(channel, 0, source, sourceChannel, 0, source.getNumSamples());
    }

    return true;
}

void Chain::normalizeAudioBuffer(juce::AudioBuffer<float>& buffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();
    if (numChannels <= 0 || numSamples <= 0)
        return;

    float peak = 0.0f;
    for (int channel = 0; channel < numChannels; ++channel)
        peak = juce::jmax(peak, buffer.getMagnitude(channel, 0, numSamples));

    if (! std::isfinite(peak) || peak <= 0.0f)
        return;

    buffer.applyGain(1.0f / peak);
}

void Chain::padBufferToLength(juce::AudioBuffer<float>& buffer, const int targetSampleCount)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();
    if (numChannels <= 0 || numSamples <= 0 || targetSampleCount <= numSamples)
        return;

    juce::AudioBuffer<float> paddedBuffer;
    paddedBuffer.setSize(numChannels, targetSampleCount, false, false, true);
    paddedBuffer.clear();
    for (int channel = 0; channel < numChannels; ++channel)
        paddedBuffer.copyFrom(channel, 0, buffer, channel, 0, numSamples);

    buffer = std::move(paddedBuffer);
}

void Chain::applyFadeInOut(juce::AudioBuffer<float>& buffer, const double sampleRate, const int fadeInMs, const int fadeOutMs)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();
    if (numChannels <= 0 || numSamples <= 0 || sampleRate <= 0.0)
        return;

    const auto minFadeFreeSamples = juce::roundToInt(sampleRate * 0.020);
    if (numSamples < minFadeFreeSamples)
        return;

    const auto fadeInSamples = juce::jlimit(0, numSamples, juce::roundToInt(sampleRate * (static_cast<double>(fadeInMs) / 1000.0)));
    const auto fadeOutSamples = juce::jlimit(0, numSamples, juce::roundToInt(sampleRate * (static_cast<double>(fadeOutMs) / 1000.0)));

    if (fadeInSamples > 0)
        buffer.applyGainRamp(0, fadeInSamples, 0.0f, 1.0f);

    if (fadeOutSamples > 0)
    {
        const auto fadeOutStart = juce::jmax(0, numSamples - fadeOutSamples);
        buffer.applyGainRamp(fadeOutStart,
                             numSamples - fadeOutStart,
                             1.0f,
                             0.0f);
    }
}

bool Chain::create(const juce::ValueTree& stateTree, const double targetSampleRate,
                   const std::function<bool()>& shouldAbort)
{
    clear();

    if (shouldAbort && shouldAbort())
        return false;
    const auto dataTree = stateTree.getChildWithName(StateHandler::dataId);
    const auto settingsTree = stateTree.getChildWithName(StateHandler::settingsId);
    if (! dataTree.isValid() || ! settingsTree.isValid())
        return false;

    const auto numSlices = dataTree.getNumChildren();
    if (numSlices <= 0)
        return false;

    const auto chainGroupSize = juce::jmax(1, static_cast<int>(settingsTree.getProperty(StateHandler::chainMaxLengthId,
                                                                                      static_cast<int>(StateHandler::chainMaxLengthValue.defaultValue))));
    const auto startIndex = getChainStartIndex(stateTree, chainGroupSize);
    const auto endIndex = juce::jmin(numSlices, startIndex + chainGroupSize);
    if (startIndex >= endIndex)
        return false;

    struct RenderedSlice
    {
        juce::AudioBuffer<float> audioData;
        int startSample = 0;
        int sampleCount = 0;
    };

    int totalSampleCount = 0;
    std::vector<RenderedSlice> renderedSlices;
    renderedSlices.reserve(static_cast<size_t>(endIndex - startIndex));

    const auto normalizationValue = settingsTree.getProperty(StateHandler::normalizationId, "none").toString();
    const bool shouldNormalizeSlice = normalizationValue == "slices";
    const bool shouldNormalizeChain = normalizationValue == "chain";
    const int outputChannelCount = settingsTree.getProperty(StateHandler::channelsId, "mono").toString() == "stereo" ? 2 : 1;
    const bool shouldPadSlices = settingsTree.getProperty(StateHandler::evenGridId, StateHandler::evenGridDefault);
    const auto fadeInInMs = settingsTree.getProperty(StateHandler::fadeinId, "none").toString().getIntValue();
    const auto fadeOutInMs = settingsTree.getProperty(StateHandler::fadeoutId, "none").toString().getIntValue();

    for (int sliceIndex = startIndex; sliceIndex < endIndex; ++sliceIndex)
    {
        if (shouldAbort && shouldAbort())
            return false;

        const auto sliceTree = dataTree.getChild(sliceIndex);
        if (!sliceTree.isValid())
            continue;

        juce::AudioBuffer<float> sliceBuffer;
        double sourceSampleRate = 0.0;
        if (!loadSliceRange(sliceTree, sliceBuffer, sourceSampleRate))
            continue;

        juce::AudioBuffer<float> renderedBuffer;
        if (!resampleSliceToTargetRate(sliceBuffer, sourceSampleRate, targetSampleRate, renderedBuffer))
            continue;

        juce::AudioBuffer<float> channelMatchedBuffer;
        if (!renderBufferToTargetChannelCount(renderedBuffer, outputChannelCount, channelMatchedBuffer))
            continue;

        if (shouldNormalizeSlice)
            normalizeAudioBuffer(channelMatchedBuffer);

        applyFadeInOut(channelMatchedBuffer, targetSampleRate, fadeInInMs, fadeOutInMs);

        const auto renderedSampleCount = channelMatchedBuffer.getNumSamples();
        if (renderedSampleCount <= 0)
            continue;

        const auto startSample = totalSampleCount;
        totalSampleCount += renderedSampleCount;
        renderedSlices.push_back({ std::move(channelMatchedBuffer), startSample, renderedSampleCount });
    }

    if (renderedSlices.empty() || outputChannelCount <= 0 || totalSampleCount <= 0)
    {
        clear();
        return false;
    }

    int largestSliceSampleCount = 0;
    if (shouldPadSlices)
    {
        for (const auto& renderedSlice : renderedSlices)
            largestSliceSampleCount = juce::jmax(largestSliceSampleCount, renderedSlice.sampleCount);
    }

    if (shouldPadSlices && largestSliceSampleCount <= 0)
    {
        clear();
        return false;
    }

    if (shouldPadSlices)
    {
        totalSampleCount = 0;
        for (auto& renderedSlice : renderedSlices)
        {
            padBufferToLength(renderedSlice.audioData, largestSliceSampleCount);
            renderedSlice.sampleCount = renderedSlice.audioData.getNumSamples();
            renderedSlice.startSample = totalSampleCount;
            totalSampleCount += renderedSlice.sampleCount;
        }
    }

    juce::AudioBuffer<float> output;
    output.setSize(outputChannelCount, totalSampleCount, false, false, true);
    output.clear();

    segments.reserve(renderedSlices.size());
    for (size_t i = 0; i < renderedSlices.size(); ++i)
    {
        const auto& renderedSlice = renderedSlices[i].audioData;
        const auto sourceSamples = renderedSlices[i].sampleCount;
        segments.push_back({ startIndex + static_cast<int>(i),
                             renderedSlices[i].startSample,
                             sourceSamples });

        for (int channel = 0; channel < outputChannelCount; ++channel)
        {
            output.copyFrom(channel, renderedSlices[i].startSample, renderedSlice, channel, 0, sourceSamples);
        }
    }

    if (shouldNormalizeChain)
        normalizeAudioBuffer(output);

    audioClip = std::make_shared<AudioClip>(std::move(output), targetSampleRate);
    return true;
}
