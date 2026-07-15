#pragma once

#include <JuceHeader.h>

#include "Core/AudioPlaybackEngine.h"
#include "Core/StateHandler.h"
#include "UI/MainMenuBarModel.h"
#include "UI/StyleSheet.h"

class StandaloneAppMainWindow final : public juce::DocumentWindow
{
public:
    explicit StandaloneAppMainWindow(const juce::String& name);
    ~StandaloneAppMainWindow() override;

    void initialise();
    void closeButtonPressed() override;

    void showAudioSettings();
    void saveAudioSettings() const;

    void saveProject();
    void loadProject();
private:
    void loadAudioSettings();

    StyleSheet style;
    juce::AudioDeviceManager audioDeviceManager;
    AudioPlaybackEngine audioPlaybackEngine;
    MainMenuBarModel menuBarModel;
    StateHandler stateHandler;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandaloneAppMainWindow)
};
