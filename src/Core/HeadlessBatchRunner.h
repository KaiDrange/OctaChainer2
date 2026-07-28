#pragma once

#include <JuceHeader.h>
#include "AudioFileLoader.h"
#include "StateHandler.h"

class HeadlessBatchRunner
{
public:
    struct HeadlessOptions
    {
        bool headless = false;
        bool help = false;
        bool evenGrid = false;
        bool embedMarkers = false;
        bool otFile = false;
        bool normalizeSlice = false;
        bool normalizeChain = false;
        double targetSampleRate = 0.0;
        int targetBitDepth = 0;
        int channels = 0;
        juce::String sourceFolder;
        juce::String targetFolder;
        juce::String targetBaseName;
        int msSliceFadeIn = -1;
        int msSliceFadeOut = -1;
        int chainSliceCount = -1;
    };

    struct Result
    {
        bool handled = false;
        bool success = false;
        int exitCode = 0;
        juce::String output;
    };

    static constexpr int maxSliceCount = 1000;
    static Result runFromCommandLine(const juce::String& commandLine);

private:
    static juce::String buildHelpText();
    static bool isSupportedFadeMs(const int fadeMs);
    static std::vector<juce::String> tokenizeCommandLine(const juce::String& commandLine);
    static bool tryParseInt(const juce::String& text, int& value);
    static bool parseOptions(const juce::String& commandLine, HeadlessOptions& options, juce::String& errorMessage);
    static bool validateOptions(const HeadlessOptions& options, juce::String& errorMessage);
    static std::vector<juce::File> collectAudioFiles(const juce::File& sourceFolder, const AudioFileLoader& loader);
    static bool loadFolderIntoState(const juce::File& sourceFolder, StateHandler& stateHandler, juce::String& errorMessage);
    static void applyHeadlessSettings(const HeadlessOptions& options, StateHandler& stateHandler);
    static bool exportAllChains(const juce::File& targetFile, const StateHandler& stateHandler, juce::String& errorMessage);

};
