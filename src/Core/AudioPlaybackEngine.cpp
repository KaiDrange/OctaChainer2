#include "AudioPlaybackEngine.h"

void AudioPlaybackEngine::play(std::shared_ptr<const AudioClip> clipToUse)
{
    const bool hasPlayback = clipToUse != nullptr;
    std::atomic_store_explicit(&currentClip, std::move(clipToUse), std::memory_order_release);
    writeIndex.store(0, std::memory_order_release);
    isPlaying.store(hasPlayback, std::memory_order_release);
}

void AudioPlaybackEngine::stop()
{
    isPlaying.store(false, std::memory_order_release);
    writeIndex.store(0, std::memory_order_release);
    std::atomic_store_explicit(&currentClip, std::shared_ptr<const AudioClip>{}, std::memory_order_release);
    sendPlaybackStoppedAction();
}

void AudioPlaybackEngine::ProcessBlock(juce::AudioBuffer<float>& buffer)
{
    juce::ScopedNoDenormals noDenormals;
    const auto numSamples = buffer.getNumSamples();

    buffer.clear();

    if (! isPlaying.load(std::memory_order_acquire))
        return;

    const auto clip = std::atomic_load_explicit(&currentClip, std::memory_order_acquire);
    if (clip == nullptr || ! clip->isValid())
    {
        stop();
        return;
    }

    const auto& currentSample = clip->getAudioData();
    const int sampleLength = currentSample.getNumSamples();
    int currentIndex = writeIndex.load(std::memory_order_acquire);

    if (currentIndex >= sampleLength)
    {
        stop();
        return;
    }

    const int samplesToProcess = std::min(numSamples, sampleLength - currentIndex);
    if (samplesToProcess > 0)
    {
        const int clipChannelCount = currentSample.getNumChannels();
        const float currentGain = juce::jmin(gain.load(std::memory_order_acquire), 1.0f);

        for (int ch = 0; ch < deviceChannelCount; ++ch)
        {
            const int sourceChannel = juce::jmin(ch, clipChannelCount - 1);
            buffer.copyFrom(ch, 0, currentSample, sourceChannel, currentIndex, samplesToProcess);
            buffer.applyGain(ch, 0, samplesToProcess, currentGain);
        }
        currentIndex += samplesToProcess;
        writeIndex.store(currentIndex, std::memory_order_release);
    }

    if (currentIndex >= sampleLength)
        stop();
}

void AudioPlaybackEngine::sendPlaybackStoppedAction() const
{
    sendActionMessage(playbackStoppedMessage);
}

double AudioPlaybackEngine::getCurrentPlaybackPositionFactor() const
{
    const auto clip = std::atomic_load_explicit(&currentClip, std::memory_order_acquire);
    if (!isPlaying.load(std::memory_order_acquire) || clip == nullptr || !clip->isValid())
        return 0.0;

    const double currentPosition = writeIndex.load(std::memory_order_acquire);
    const double sampleLength = currentClip->getAudioData().getNumSamples();
    return currentPosition / sampleLength;
}
