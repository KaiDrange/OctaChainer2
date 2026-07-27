#pragma once

#include <JuceHeader.h>
#include <cstdint>
#include <memory>
#include <vector>

#include "AudioFileLoader.h"
#include "OtFileFormat.h"
#include "StateHandler.h"

class OtReader : public std::enable_shared_from_this<OtReader>
{
public:
    struct ImportSettings
    {
        juce::File otFile;
        juce::File audioFile;
        int sampleRate = 0;
        int bitDepth = 0;
        int audioChannels = 0;
        juce::int64 audioLengthInSamples = 0;
        double tempo = 0.0;
        double gain = 0.0;
        OtFileFormat::Loop_t loopSetting = OtFileFormat::NoLoop;
        OtFileFormat::Stretch_t stretchSetting = OtFileFormat::NoStretch;
        OtFileFormat::TrigQuant_t trigQuantSetting = OtFileFormat::Direct;
        std::vector<juce::Range<juce::int64>> sliceRanges;
    };

    OtReader();

    void importOtFile(const juce::File& otFile, StateHandler& stateHandler, juce::Component* associatedComponent);
    bool loadImportSettings(const juce::File& otFile, ImportSettings& settings, juce::Component* associatedComponent);
    bool performImport(const ImportSettings& settings, StateHandler& stateHandler, juce::Component* associatedComponent);

private:
    struct ParsedOtFile
    {
        double tempo = 0.0;
        double gain = 0.0;
        OtFileFormat::Loop_t loopSetting = OtFileFormat::NoLoop;
        OtFileFormat::Stretch_t stretchSetting = OtFileFormat::NoStretch;
        OtFileFormat::TrigQuant_t trigQuantSetting = OtFileFormat::Direct;
        std::uint32_t trimStart = 0;
        std::uint32_t trimEnd = 0;
        std::vector<OtFileFormat::Slice> slices;
    };

    static std::uint32_t checksumBytes(const std::vector<std::uint8_t>& bytes);
    static std::uint16_t readU16BE(const std::uint8_t* data, std::size_t offset);
    static std::uint32_t readU32BE(const std::uint8_t* data, std::size_t offset);
    static bool isSupportedBitDepth(int bitDepth);
    static void showErrorAsync(const juce::String& title, const juce::String& message, juce::Component* associatedComponent);
    static bool readOtFileBytes(const juce::File& otFile, std::vector<std::uint8_t>& bytes, juce::String* errorMessage);
    static bool parseOtBytes(const std::vector<std::uint8_t>& bytes, ParsedOtFile& parsed, juce::String* errorMessage);
    static bool readAudioFileMetadata(const juce::File& file, int& sampleRate, int& bitDepth, int& numChannels,
                                      juce::int64& lengthInSamples, juce::String* errorMessage);
    bool readImportSettingsImpl(const juce::File& otFile, ImportSettings& settings, juce::String* errorMessage) const;
    bool performImportImpl(const ImportSettings& settings, StateHandler& stateHandler, juce::String* errorMessage);

    AudioFileLoader audioFileLoader;
};
