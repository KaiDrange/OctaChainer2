#pragma once

#include <JuceHeader.h>
#include <functional>
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

    bool create(const juce::ValueTree& stateTree, double targetSampleRate,
                 const std::function<bool()>& shouldAbort);
    void clear();
    static int getChainStartIndex(const StateHandler& stateHandler, int chainSliceCount);
    static int getChainStartIndex(const juce::ValueTree& stateTree, int chainSliceCount);

    [[nodiscard]] bool isValid() const noexcept;
    [[nodiscard]] std::shared_ptr<AudioClip> getAudioClip() const noexcept;
    [[nodiscard]] const juce::AudioBuffer<float>& getAudioData() const noexcept;
    [[nodiscard]] double getSampleRate() const noexcept;
    [[nodiscard]] const std::vector<Segment>& getSegments() const noexcept;

private:
    static bool loadSliceRange(const juce::ValueTree& sliceTree,
                               juce::AudioBuffer<float>& destination, double& sampleRate);
    static bool resampleSliceToTargetRate(const juce::AudioBuffer<float>& source, double sourceSampleRate,
                                          double targetSampleRate, juce::AudioBuffer<float>& destination);
    static void normalizeAudioBuffer(juce::AudioBuffer<float>& buffer);

    static void applyFadeInOut(juce::AudioBuffer<float>& buffer, double sampleRate, int fadeInMs, int fadeOutMs);

    std::shared_ptr<AudioClip> audioClip;
    std::vector<Segment> segments;
};
