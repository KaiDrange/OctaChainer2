#pragma once

#include <JuceHeader.h>
#include <vector>

#include "AudioClip.h"
#include "StateHandler.h"

class Chain
{
public:
    struct Segment
    {
        int sliceIndex = -1;
        int startSample = 0;
        int sampleCount = 0;
    };

    void rebuild(const StateHandler& stateHandler);
    void clear();
    static int getChainStartIndex(const StateHandler& stateHandler, int chainSliceCount);

    [[nodiscard]] bool isValid() const noexcept;
    [[nodiscard]] const AudioClip& getAudioClip() const noexcept;
    [[nodiscard]] const juce::AudioBuffer<float>& getAudioData() const noexcept;
    [[nodiscard]] double getSampleRate() const noexcept;
    [[nodiscard]] int getBitDepth() const noexcept;
    [[nodiscard]] const std::vector<Segment>& getSegments() const noexcept;

private:
    static bool loadSliceRange(const StateHandler& stateHandler, const juce::ValueTree& sliceTree,
                               juce::AudioBuffer<float>& destination, double& sampleRate);
    static bool resampleSliceToTargetRate(const juce::AudioBuffer<float>& source, double sourceSampleRate,
                                          double targetSampleRate, juce::AudioBuffer<float>& destination);

    std::shared_ptr<AudioClip> audioClip;
    std::vector<Segment> segments;
    int bitDepth = 0;
};
