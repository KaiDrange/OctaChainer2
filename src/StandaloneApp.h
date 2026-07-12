#pragma once

#include <JuceHeader.h>

#include "StandaloneAppMainWindow.h"

class OctaChainer2StandaloneApplication final : public juce::JUCEApplication
{
public:
    OctaChainer2StandaloneApplication() = default;

    const juce::String getApplicationName() override;
    const juce::String getApplicationVersion() override;

    bool moreThanOneInstanceAllowed() override;

    void initialise(const juce::String& commandLine) override;
    void shutdown() override;
    void systemRequestedQuit() override;
    void anotherInstanceStarted(const juce::String& commandLine) override;

private:
    std::unique_ptr<StandaloneAppMainWindow> mainWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OctaChainer2StandaloneApplication)
};
