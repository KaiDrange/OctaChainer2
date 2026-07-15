#include "AudioClip.h"

AudioClip::AudioClip(juce::AudioBuffer<float> audioDataToUse, const double sampleRateToUse)
    : audioData(std::move(audioDataToUse)),
      sampleRate(sampleRateToUse)
{
}

void AudioClip::setAudioData(juce::AudioBuffer<float> audioDataToUse, const double sampleRateToUse)
{
    audioData = std::move(audioDataToUse);
    sampleRate = sampleRateToUse;
}

void AudioClip::clear()
{
    audioData.setSize(0, 0);
    sampleRate = 0.0;
}
