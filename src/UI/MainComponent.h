#pragma once

#include <JuceHeader.h>

#include "../Core/AudioPlaybackEngine.h"
#include "../Core/StateHandler.h"
#include "AudioPanelComponent.h"
#include "SliceListComponent.h"
#include "SettingsPanelComponent.h"
#include "StyleSheet.h"
#include "WaveformComponent.h"

class MainComponent : public juce::Component,
                      public juce::DragAndDropContainer,
                      public juce::Timer,
                      StateHandler::Listener,
                      AudioPanelComponent::Listener
{
public:
    static constexpr int defaultWidth = 1536;
    static constexpr int defaultHeight = 880;
    static constexpr int maxWidth = 2560;
    static constexpr int maxHeight = 1440;
    static constexpr int minWidth = 900;
    static constexpr int minHeight = 600;

    static constexpr int sampleListWidthPercentage = 70;
    static constexpr int sampleListHeightPercentage = 75;
    static constexpr int sampleListMinHeight = minHeight;

    static constexpr int audioSectionFixedHeight = 100;

    static constexpr int panelMarginPixels = StyleSheet::panelMargins;
    static constexpr int settingsPanelFixedWidth = 300;
    static constexpr int settingsPanelFixedHeight = 534;

    static constexpr int sliceWaveformHeightPercentage = (100 - sampleListHeightPercentage) / 2;
    static constexpr int chainWaveformWidthPercentage = sliceWaveformHeightPercentage;

    MainComponent(StateHandler& stateHandlerToUse, AudioPlaybackEngine& audioPlaybackEngineToUse);
    ~MainComponent() override;

    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void transportButtonPressed(TransportButtonComponent::TransportEvent event) = 0;
    };

    void addListener(Listener* listener);
    void removeListener(Listener* listenerToRemove);
    void detachPlaybackListener();
    void timerCallback() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void stateChanged() override;
    void transportButtonPressed(TransportButtonComponent::TransportEvent event) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)

    void updateSliceWaveform();

    StyleSheet style;
    StateHandler& stateHandler;
    AudioPlaybackEngine& audioPlaybackEngine;
    SliceListComponent sampleListComponent;
    SettingsPanelComponent settingsPanelComponent;
    WaveformComponent sliceWaveformComponent;
    WaveformComponent chainWaveformComponent;
    AudioPanelComponent audioPanelComponent;

    void sendTransportEvent(TransportButtonComponent::TransportEvent event);
    juce::ListenerList<Listener> listeners;
};
