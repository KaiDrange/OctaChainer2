#pragma once

#include "AudioClip.h"

class AudioUtil
{
public:
    static std::shared_ptr<AudioClip> renderPlaybackClip(AudioClip clip, double targetSamplerate, int targetChannelCount);
    static bool writeWavFile(const juce::File& file, const juce::AudioBuffer<float>& audioData, double sampleRate,
                             int bitDepth, juce::String* errorMessage = nullptr);
};
