#pragma once

#include <JuceHeader.h>

class Slice
{
public:
    juce::AudioBuffer<float>* getAudioData() { return &audioData; }
    [[nodiscard]] const juce::AudioBuffer<float>* getAudioData() const { return &audioData; }

    double samplerate = 0.0f;
    unsigned int bitDepth = 0;
    int channels = 0;
    int64 lengthInSamples = 0;
    int64 start = 0;
    int64 end = 0;
    int64 loopStart = 0;
    int64 loopEnd = 0;
    juce::String name;
    juce::String sourcePath;

private:
    juce::AudioBuffer<float> audioData;
};
