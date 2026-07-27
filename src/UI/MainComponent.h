#pragma once

#include <JuceHeader.h>

#include "../Core/AudioPlaybackEngine.h"
#include "../Core/Chain.h"
#include "../Core/OtReader.h"
#include "../Core/StateHandler.h"
#include "AudioPanelComponent.h"
#include "SliceListComponent.h"
#include "SettingsPanelComponent.h"
#include "StyleSheet.h"
#include "WaveformComponent.h"
#include "../Core/OtWriter.h"

class MainComponent : public juce::Component,
                      public juce::DragAndDropContainer,
                      public juce::FileDragAndDropTarget,
                      public juce::Timer,
                      StateHandler::Listener,
                      AudioPanelComponent::Listener,
                      SettingsPanelComponent::Listener,
                      WaveformComponent::Listener
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
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    void stateChanged(const StateHandler::StateChange& change) override;
    void transportButtonPressed(TransportButtonComponent::TransportEvent event) override;
    void chainExportRequested() override;
    void waveformSegmentClicked(int segmentIndex, int sliceIndex) override;
    void waveformSliceRangeChanged(int startSample, int endSample) override;
    void importOtFile(const juce::File& otFile);
    std::shared_ptr<const AudioClip> getChainAudioClip() const { return chain.getAudioClip(); }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)

    void updateSliceWaveform();
    void cancelChainRender();
    void requestChainRender(bool stopPlayback);
    void clearPlaybackChain();
    void saveChainToFile();
    static bool exportChainToFile(const juce::File& wavFile, const juce::ValueTree& exportState, double targetSampleRate,
                                  int bitDepth, juce::String* errorMessage);
    void finishChainRender(std::uint64_t requestId, const std::shared_ptr<Chain>& renderedChain, const juce::String& renderError = {});
    void chainRenderThreadLoop();
    void stopChainRenderThread();
    bool isSelectedSliceInCurrentChain() const;
    bool isChainRenderRelevantSetting(const StateHandler::StateChange& change) const;
    bool isOtFilePath(const juce::String& path) const;
    static void showChainRenderError(const juce::String& message);
    static OtFileFormat::Stretch_t getOtStretchSetting(const juce::ValueTree& settingsTree);
    static OtFileFormat::Loop_t getOtLoopSetting(const juce::ValueTree& settingsTree);
    static OtFileFormat::TrigQuant_t getOtTrigQuantSetting(const juce::ValueTree& settingsTree);
    static bool shouldWriteOtFile(const juce::ValueTree& exportState);
    static bool writeOtFile(const juce::File& wavFile, const juce::ValueTree& exportState, const Chain& exportChain,
                 juce::String* errorMessage);

    StyleSheet style;
    StateHandler& stateHandler;
    AudioPlaybackEngine& audioPlaybackEngine;
    std::shared_ptr<OtReader> otReader;
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
    std::unique_ptr<juce::FileChooser> chainExportChooser;

    void sendTransportEvent(TransportButtonComponent::TransportEvent event);
    juce::ListenerList<Listener> listeners;
};
