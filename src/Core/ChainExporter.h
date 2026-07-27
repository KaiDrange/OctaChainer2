#pragma once

#include <JuceHeader.h>

#include "Chain.h"
#include "OtFileFormat.h"

class ChainExporter
{
public:
    static bool exportToFile(const juce::File& wavFile,
                             const juce::ValueTree& exportState,
                             double targetSampleRate,
                             int bitDepth,
                             juce::String* errorMessage = nullptr);

private:
    static OtFileFormat::Stretch_t getOtStretchSetting(const juce::ValueTree& settingsTree);
    static OtFileFormat::Loop_t getOtLoopSetting(const juce::ValueTree& settingsTree);
    static OtFileFormat::TrigQuant_t getOtTrigQuantSetting(const juce::ValueTree& settingsTree);
    static bool shouldWriteOtFile(const juce::ValueTree& exportState);
    static bool writeOtFile(const juce::File& wavFile,
                            const juce::ValueTree& exportState,
                            const Chain& exportChain,
                            juce::String* errorMessage);
};
