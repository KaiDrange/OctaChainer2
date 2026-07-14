#pragma once

#include <JuceHeader.h>

class AudioPlaybackEngine
{
public:
    void prepare(double sampleRate, int maxBlockSize);
    void playSlice(const juce::AudioBuffer<float>& buffer, double sourceSampleRate);
    void stop();
    void renderNextBlock(juce::AudioBuffer<float>& output);
};
