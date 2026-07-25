#pragma once

#include <vector>

#include "../Core/Chain.h"
#include "PanelComponent.h"


class WaveformComponent : public PanelComponent
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void waveformSegmentClicked(int segmentIndex, int sliceIndex) = 0;
    };

    WaveformComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                      const juce::String& title);
    ~WaveformComponent() override;

    void setAudioData(const juce::AudioBuffer<float>& audioData, double sampleRate);
    void setAudioData(const juce::AudioBuffer<float>& audioData, double sampleRate,
                      const std::vector<Chain::Segment>& segments, int selectedSegmentIndex = -1);
    void clearAudioData();
    void setProcessingState(bool isProcessing, juce::String message = "Processing...");
    void setSelectedSegmentIndex(int newSelectedSegmentIndex);
    void setPlayHeadPositionFactor(double newPlayHeadPositionFactor);
    void addListener(Listener* listener);
    void removeListener(Listener* listenerToRemove);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

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

    void drawWaveform(juce::Graphics& g) const;
    void drawSegmentWaveform(juce::Graphics& g, const juce::Rectangle<int>& waveformArea, int segmentIndex,
                             int segmentStartSample, int segmentSampleCount, bool isSelectedSegment) const;
    void drawBufferWaveform(juce::Graphics& g, const juce::Rectangle<int>& waveformArea, int startSample,
                            int sampleCount, juce::Colour waveformColour) const;
    void drawChannelCenterlines(juce::Graphics& g, const juce::Rectangle<int>& waveformArea) const;
    int getSegmentIndexAtPoint(juce::Point<int> position) const;
    void sendWaveformSegmentClicked(int segmentIndex, int sliceIndex);

    PlayHeadOverlayComponent playHeadOverlay;
    juce::AudioBuffer<float> waveformSourceBuffer;
    std::vector<Chain::Segment> waveformSegments;
    double waveformSampleRate = 0.0;
    int selectedSegmentIndex = -1;
    bool processing = false;
    juce::String processingMessage;
    juce::ListenerList<Listener> listeners;
};
