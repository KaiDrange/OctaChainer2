#pragma once

#include "PanelComponent.h"


class WaveformComponent : public PanelComponent,
                          private juce::ChangeListener
{
public:
    WaveformComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                      const juce::String& title);
    ~WaveformComponent() override;

    void setAudioData(const juce::AudioBuffer<float>& audioData, double sampleRate);
    void clearAudioData();

    void paint(juce::Graphics& g) override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    void drawWaveform(juce::Graphics& g);

    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache{8};
    juce::AudioThumbnail thumbnail{512, formatManager, thumbnailCache};
    juce::AudioBuffer<float> thumbnailSourceBuffer;
};
