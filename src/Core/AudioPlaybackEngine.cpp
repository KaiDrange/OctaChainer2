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
}

void AudioPlaybackEngine::ProcessBlock(juce::AudioBuffer<float>& buffer)
{
    juce::ScopedNoDenormals noDenormals;
    const auto numSamples = buffer.getNumSamples();

    buffer.clear();

    if (! isPlaying.load(std::memory_order_acquire))
        return;

    const auto playback = std::atomic_load_explicit(&currentClip, std::memory_order_acquire);
    if (playback == nullptr || ! playback->isValid())
    {
        stop();
        return;
    }

    const auto& currentSample = playback->getAudioData();
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

        for (int ch = 0; ch < deviceChannelCount; ++ch)
        {
            const int sourceChannel = juce::jmin(ch, clipChannelCount - 1);
            buffer.copyFrom(ch, 0, currentSample, sourceChannel, currentIndex, samplesToProcess);
        }
        currentIndex += samplesToProcess;
        writeIndex.store(currentIndex, std::memory_order_release);
    }

    if (currentIndex >= sampleLength)
        stop();
}
