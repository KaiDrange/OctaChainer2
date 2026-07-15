#include "AudioUtil.h"

// Resamples and sums channels to mono if necessary. Note: if target channel count is greater than source channel count,
// the output will be the same as the source channel count. It will be cheaper to upmix at the latest stage possible. Either
// in the audio engine for playback or when writing to a file.
std::shared_ptr<AudioClip> AudioUtil::renderPlaybackClip(AudioClip clip, const double targetSamplerate, const int targetChannelCount)
{
    if (targetSamplerate <= 0 || targetChannelCount <= 0)
    {
        DBG("Invalid target sample rate or channel count for rendering audio clip");
        return nullptr;
    }

    const auto& source = clip.getAudioData();
    const int sourceChannelCount = source.getNumChannels();
    const int sourceSampleCount = source.getNumSamples();

    const int outputChannelCount = juce::jmin(sourceChannelCount, targetChannelCount);
    const bool needsResample = clip.getSampleRate() != targetSamplerate;
    const bool needsDownmixToMono = outputChannelCount < sourceChannelCount;

    if (!needsResample && !needsDownmixToMono)
        return std::make_shared<AudioClip>(std::move(clip));

    const int outputSampleCount = needsResample
        ? juce::jmax(1, static_cast<int>(static_cast<double>(sourceSampleCount) * targetSamplerate / clip.getSampleRate()))
        : sourceSampleCount;
    juce::AudioBuffer<float> output;
    output.setSize(outputChannelCount, outputSampleCount, false, false, true);
    output.clear();

    const double speedRatio = clip.getSampleRate() / targetSamplerate;

    // Resample only
    if (!needsDownmixToMono)
    {
        for (int ch = 0; ch < outputChannelCount; ++ch)
        {
            if (!needsResample)
            {
                output.copyFrom(ch, 0, source, ch, 0, sourceSampleCount);
                continue;
            }

            juce::LagrangeInterpolator interpolator;
            interpolator.reset();
            interpolator.process(speedRatio,
                                 source.getReadPointer(ch),
                                 output.getWritePointer(ch),
                                 outputSampleCount);
        }

        return std::make_shared<AudioClip>(std::move(output),
                                                   needsResample ? targetSamplerate : clip.getSampleRate());
    }

    // Downmix only or resample + downmix, both to mono.
    juce::AudioBuffer<float> monoScratch;
    monoScratch.setSize(1, outputSampleCount, false, false, true);
    monoScratch.clear();

    const float mixGain = 1.0f / static_cast<float>(sourceChannelCount);

    for (int ch = 0; ch < sourceChannelCount; ++ch)
    {
        if (!needsResample)
        {
            output.addFrom(0, 0, source, ch, 0, sourceSampleCount, mixGain);
            continue;
        }

        monoScratch.clear();

        juce::LagrangeInterpolator interpolator;
        interpolator.reset();
        interpolator.process(speedRatio,
                             source.getReadPointer(ch),
                             monoScratch.getWritePointer(0),
                             outputSampleCount);

        output.addFrom(0, 0, monoScratch, 0, 0, outputSampleCount, mixGain);
    }

    return std::make_shared<AudioClip>(std::move(output),
                                               needsResample ? targetSamplerate : clip.getSampleRate());
}
