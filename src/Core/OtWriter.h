#pragma once

#include <JuceHeader.h>
#include "OtFileFormat.h"


class OtWriter
{
public:
    OtWriter(juce::File outputFileToWrite,
             int sampleRateToUse,
             OtFileFormat::Loop_t loopSettingToUse,
             OtFileFormat::Stretch_t stretchSettingToUse,
             OtFileFormat::TrigQuant_t trigQuantSettingToUse,
             double gainToUse,
             double bpmToUse);

    bool write(std::uint32_t totalSamples) const;
    void addSlice(std::uint32_t start, std::uint32_t end);

private:
    std::array<std::uint8_t, OtFileFormat::otFileSize> buildPayload(std::uint32_t totalSamples) const;
    static void writeU16BE(std::uint8_t*& dest, std::uint16_t value);
    static void writeU32BE(std::uint8_t*& dest, std::uint32_t value);
    static std::uint16_t checksum(const std::uint8_t* data);

    juce::File outputFile;
    int sampleRate = 0;
    double bpm = 0.0;
    double gain = 0.0;
    OtFileFormat::Loop_t loopSetting = OtFileFormat::NoLoop;
    OtFileFormat::Stretch_t stretchSetting = OtFileFormat::NoStretch;
    OtFileFormat::TrigQuant_t trigQuantSetting = OtFileFormat::Pattern;
    std::vector<OtFileFormat::Slice> slices;
};
