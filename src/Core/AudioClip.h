#pragma once

#include <JuceHeader.h>

class AudioClip
{
public:
    AudioClip() = default;
    AudioClip(juce::AudioBuffer<float> audioDataToUse, double sampleRateToUse);

    juce::AudioBuffer<float>& getAudioData() noexcept { return audioData; }
    [[nodiscard]] const juce::AudioBuffer<float>& getAudioData() const noexcept { return audioData; }

    void setAudioData(juce::AudioBuffer<float> audioDataToUse, double sampleRateToUse);
    void clear();
    [[nodiscard]] int getChannelCount() const noexcept { return audioData.getNumChannels(); }

    [[nodiscard]] double getSampleRate() const noexcept { return sampleRate; }
    [[nodiscard]] bool isValid() const noexcept { return audioData.getNumSamples() > 0 && audioData.getNumChannels() > 0; }

private:
    juce::AudioBuffer<float> audioData;
    double sampleRate = 0.0;
};
