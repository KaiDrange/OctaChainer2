#include "AudioPlaybackEngine.h"

void AudioPlaybackEngine::ProcessBlock(juce::AudioBuffer<float>& buffer)
{
    juce::ScopedNoDenormals noDenormals;
    const auto numOutputChannels = 2; // TODO: Make this configurable based on the output device's channel count
    const auto numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numOutputChannels; ++ch)
        buffer.clear(ch, 0, numSamples);

    if (!isPlaying.load())
        return;

    const auto& currentSample = currentSlice.getAudioData();
    const int sampleLength = currentSample->getNumSamples();
    int currentIndex = writeIndex.load();

    const int samplesToProcess = std::min(numSamples, sampleLength - currentIndex);
    if (samplesToProcess > 0)
    {
        const int numChannelsToCopy = std::min(numOutputChannels, currentSample->getNumChannels());
        for (int ch = 0; ch < numChannelsToCopy; ++ch)
        {
            buffer.copyFrom(ch, 0, *currentSample, ch, currentIndex, samplesToProcess);
        }
        currentIndex += samplesToProcess;
        writeIndex.store(currentIndex);
    }

    if (currentIndex >= sampleLength)
    {
        isPlaying.store(false);
        writeIndex.store(0);
    }
}
