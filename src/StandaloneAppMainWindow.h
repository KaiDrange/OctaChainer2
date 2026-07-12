#pragma once

#include <JuceHeader.h>

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
private:
    void loadAudioSettings();

    StyleSheet style;
    juce::AudioDeviceManager audioDeviceManager;
    MainMenuBarModel menuBarModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandaloneAppMainWindow)
};
