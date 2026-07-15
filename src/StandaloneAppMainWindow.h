#pragma once

#include <JuceHeader.h>

#include "Core/AudioPlaybackEngine.h"
#include "Core/StateHandler.h"
#include "UI/MainMenuBarModel.h"
#include "UI/StyleSheet.h"
#include "UI/MainComponent.h"

class StandaloneAppMainWindow final : public juce::DocumentWindow,
                                      public MainComponent::Listener,
                                      public juce::AudioIODeviceCallback
{
public:
    explicit StandaloneAppMainWindow(const juce::String& name);
    ~StandaloneAppMainWindow() override;

    void initialise();
    void closeButtonPressed() override;
    void transportButtonPressed(TransportButtonComponent::TransportEvent event) override;
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels,
                                          float* const* outputChannelData, int numOutputChannels,
                                          int numSamples, const juce::AudioIODeviceCallbackContext& context) override;

    void showAudioSettings();
    void saveAudioSettings() const;

    void saveProject();
    void loadProject();
private:
    void loadAudioSettings();

    MainComponent* mainComponent = nullptr;
    StyleSheet style;
    juce::AudioDeviceManager audioDeviceManager;
    AudioPlaybackEngine audioPlaybackEngine;
    MainMenuBarModel menuBarModel;
    StateHandler stateHandler;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandaloneAppMainWindow)
};
