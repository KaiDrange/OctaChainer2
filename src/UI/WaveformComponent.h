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
    void setPlayHeadPositionFactor(double newPlayHeadPositionFactor);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    class PlayHeadOverlayComponent : public juce::Component
    {
    public:
        PlayHeadOverlayComponent();

        void setPlayHeadPositionFactor(double newPlayHeadPositionFactor);
        void paint(juce::Graphics& g) override;

    private:
        double playHeadPositionFactor{0.0};
    };

    void drawWaveform(juce::Graphics& g);

    PlayHeadOverlayComponent playHeadOverlay;
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache{8};
    juce::AudioThumbnail thumbnail{512, formatManager, thumbnailCache};
    juce::AudioBuffer<float> thumbnailSourceBuffer;
};
