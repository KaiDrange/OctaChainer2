#pragma once

#include <JuceHeader.h>
#include <memory>

#include "AudioClip.h"

class AudioPlaybackEngine : public juce::ActionBroadcaster
{
public:
    inline static const juce::String playbackStoppedMessage{"PlaybackStopped"};

    void play(std::shared_ptr<const AudioClip> clipToUse);
    void stop();
    void ProcessBlock(juce::AudioBuffer<float>& buffer);

    void sendPlaybackStoppedAction() const;
    double getCurrentPlaybackPositionFactor() const;

    std::atomic<bool> isPlaying{false};
    std::atomic<int> writeIndex{0};
    double deviceSampleRate = -1;
    int deviceChannelCount = -1;

private:
    std::shared_ptr<const AudioClip> currentClip;
};
