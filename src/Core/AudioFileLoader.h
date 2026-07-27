#pragma once

#include <JuceHeader.h>
#include "Slice.h"

class AudioFileLoader
{
public:
    static constexpr std::array<char, 4> riffChunkId{ 'R', 'I', 'F', 'F' };
    static constexpr std::array<char, 4> waveFormatId{ 'W', 'A', 'V', 'E' };
    static constexpr std::array<char, 4> cueChunkId{ 'c', 'u', 'e', ' ' };

    static constexpr juce::int64 maxLoadedAudioDataBytes = 16ll * 1024ll * 1024ll;

    AudioFileLoader();

    std::unique_ptr<Slice> loadFile(const juce::File& file, juce::String* errorMessage = nullptr);
    bool loadFile(const juce::File& file, Slice& destination, juce::String* errorMessage = nullptr);
    std::vector<std::unique_ptr<Slice>> loadFileSlices(const juce::File& file, juce::String* errorMessage = nullptr);
    std::unique_ptr<Slice> loadFileRegion(const juce::File& file, juce::Range<juce::int64> sampleRange,
                                          juce::String* errorMessage = nullptr);
    bool loadFileRegion(const juce::File& file, const juce::Range<juce::int64>& sampleRange, Slice& destination,
                        juce::String* errorMessage = nullptr);

    [[nodiscard]] juce::String getSupportedFilePatterns() const;

private:
    static void setError(juce::String* errorMessage, const juce::String& message);
    static std::uint32_t readU32LE(const std::uint8_t* data);
    static bool readBytes(juce::InputStream& stream, void* destination, const int bytesToRead);
    static bool readWaveCueOffsets(const juce::File& file, std::vector<juce::int64>& cueOffsets);
    static std::vector<juce::Range<juce::int64>> buildSliceRangesFromCueOffsets(const std::vector<juce::int64>& cueOffsets,
                                                                         const juce::int64 lengthInSamples);
    static bool loadSliceFromReader(juce::AudioFormatReader& reader, const juce::File& file,
                             const juce::Range<juce::int64>& sampleRange, Slice& destination,
                             juce::String* errorMessage);

    juce::AudioFormatManager formatManager;
};
