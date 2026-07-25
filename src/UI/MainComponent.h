#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <memory>
#include <thread>

#include "../Core/AudioPlaybackEngine.h"
#include "../Core/Chain.h"
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
    void updateChainWaveform();
    void refreshChainWaveformSelection();
    void timerCallback() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void stateChanged(const StateHandler::StateChange& change) override;
    void transportButtonPressed(TransportButtonComponent::TransportEvent event) override;
    std::shared_ptr<const AudioClip> getChainAudioClip() const { return chain.getAudioClip(); }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)

    void updateSliceWaveform();
    void requestChainRender();
    void finishChainRender(std::uint64_t requestId, const std::shared_ptr<Chain>& renderedChain);
    void chainRenderThreadLoop();
    void stopChainRenderThread();
    bool isSelectedSliceInCurrentChain() const;

    StyleSheet style;
    StateHandler& stateHandler;
    AudioPlaybackEngine& audioPlaybackEngine;
    Chain chain;
    SliceListComponent sampleListComponent;
    SettingsPanelComponent settingsPanelComponent;
    WaveformComponent sliceWaveformComponent;
    WaveformComponent chainWaveformComponent;
    AudioPanelComponent audioPanelComponent;
    std::thread chainRenderThread;
    std::mutex chainRenderMutex;
    std::condition_variable chainRenderCondition;
    std::atomic<bool> chainRenderExitRequested{false};
    bool chainRenderHasPending = false;
    juce::ValueTree chainRenderPendingState;
    double chainRenderPendingSampleRate = 0.0;
    std::uint64_t chainRenderPendingRequestId = 0;
    std::atomic<std::uint64_t> chainRenderLatestRequestId{0};

    void sendTransportEvent(TransportButtonComponent::TransportEvent event);
    juce::ListenerList<Listener> listeners;
};
