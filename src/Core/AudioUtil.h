#pragma once

#include "AudioClip.h"
#include "Chain.h"

class AudioUtil
{
public:
    static constexpr std::array<char, 4> riffChunkId{ 'R', 'I', 'F', 'F' };
    static constexpr std::array<char, 4> waveFormatId{ 'W', 'A', 'V', 'E' };
    static constexpr std::array<char, 4> fmtChunkId{ 'f', 'm', 't', ' ' };
    static constexpr std::array<char, 4> dataChunkId{ 'd', 'a', 't', 'a' };
    static constexpr std::array<char, 4> cueChunkId{ 'c', 'u', 'e', ' ' };

    static std::shared_ptr<AudioClip> renderPlaybackClip(AudioClip clip, double targetSamplerate, int targetChannelCount);
    static std::vector<juce::int64> buildCueOffsetsFromSegments(const std::vector<Chain::Segment>& segments);
    static bool writeWavFile(const juce::File& file, const juce::AudioBuffer<float>& audioData, double sampleRate,
                             int bitDepth, const std::vector<juce::int64>* cueOffsets = nullptr,
                             juce::String* errorMessage = nullptr);

    static bool readWaveCueOffsets(const juce::File& file, std::vector<juce::int64>& cueOffsets);
    static std::vector<juce::Range<juce::int64>> buildSliceRangesFromCueOffsets(const std::vector<juce::int64>& cueOffsets,
                                                                                juce::int64 lengthInSamples);

private:
    static constexpr std::uint16_t pcmFormat = 1;

    static std::uint32_t readU32LE(const std::uint8_t* data);
    static void writeFourCC(std::uint8_t*& dest, const std::array<char, 4>& id);
    static void writeU16LE(std::uint8_t*& dest, std::uint16_t value);
    static void writeU32LE(std::uint8_t*& dest, std::uint32_t value);
    static bool writePcmSample(std::uint8_t*& dest, float sample, int bitDepth);
};
