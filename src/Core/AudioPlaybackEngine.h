#pragma once

#include <JuceHeader.h>

#include "Slice.h"

class AudioPlaybackEngine
{
public:
    // void prepare(double sampleRate, int maxBlockSize);
    // void playSlice(const juce::AudioBuffer<float>& buffer, double sourceSampleRate);
    // void stop();
    // void renderNextBlock(juce::AudioBuffer<float>& output);

    void ProcessBlock(juce::AudioBuffer<float>& buffer);

    std::atomic<bool> isPlaying{false};
    std::atomic<int> writeIndex{0};

    Slice currentSlice;
};
