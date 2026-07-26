#pragma once

#include <JuceHeader.h>
#include <memory>

#include "Slice.h"

class AudioFileLoader
{
public:
    static constexpr juce::int64 maxLoadedAudioDataBytes = 16ll * 1024ll * 1024ll;

    AudioFileLoader();

    std::unique_ptr<Slice> loadFile(const juce::File& file, juce::String* errorMessage = nullptr);
    bool loadFile(const juce::File& file, Slice& destination, juce::String* errorMessage = nullptr);

    [[nodiscard]] juce::String getSupportedFilePatterns() const;

private:
    static void setError(juce::String* errorMessage, const juce::String& message);

    juce::AudioFormatManager formatManager;
};
