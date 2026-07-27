#pragma once

#include <JuceHeader.h>
#include "Slice.h"

class AudioFileLoader
{
public:
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
    static bool loadSliceFromReader(juce::AudioFormatReader& reader, const juce::File& file,
                             const juce::Range<juce::int64>& sampleRange, Slice& destination,
                             juce::String* errorMessage);

    juce::AudioFormatManager formatManager;
};
