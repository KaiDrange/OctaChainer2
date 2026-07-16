#pragma once

#include "AudioClip.h"

class AudioUtil
{
public:
    static std::shared_ptr<AudioClip> renderPlaybackClip(AudioClip clip, double targetSamplerate, int targetChannelCount);
};
