#pragma once

#include <JuceHeader.h>

class MegabreakExporter
{
public:
    static bool exportToFiles(const juce::File& baseFile,
                              const juce::ValueTree& baseState,
                              double targetSampleRate,
                              int bitDepth,
                              int partCount,
                              std::function<void(int completedParts, int totalParts)> progressCallback = {},
                              juce::String* errorMessage = nullptr);

private:
    static juce::Range<juce::int64> getPartRange(const juce::ValueTree& sliceTree, int partIndex, int partCount);
    static bool prepareExportState(const juce::ValueTree& exportState, int partIndex, int partCount);
};
