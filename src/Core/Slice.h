#pragma once

#include <JuceHeader.h>

class Slice
{
public:
    juce::AudioBuffer<float>* getAudioData() { return &audioData; }
    [[nodiscard]] const juce::AudioBuffer<float>* getAudioData() const { return &audioData; }
    void createBlankAudioData(const int numChannels, const int64 numSamples)
    {
        audioData.setSize(numChannels, static_cast<int>(numSamples));
        audioData.clear();
    }


    double samplerate = 0.0f;
    unsigned int bitDepth = 0;
    int channels = 0;
    int64 lengthInSamples = 0;
    int64 start = 0;
    int64 end = 0;
    int64 loopStart = 0;
    juce::String name;
    juce::String sourcePath;

private:
    juce::AudioBuffer<float> audioData;
};
