#include "StandaloneApp.h"

#include "UI/MainComponent.h"

const juce::String OctaChainer2StandaloneApplication::getApplicationName()
{
    return ProjectInfo::projectName;
}

const juce::String OctaChainer2StandaloneApplication::getApplicationVersion()
{
    return ProjectInfo::versionString;
}

bool OctaChainer2StandaloneApplication::moreThanOneInstanceAllowed()
{
    return true;
}

void OctaChainer2StandaloneApplication::initialise(const juce::String& commandLine)
{
    juce::ignoreUnused(commandLine);

    mainWindow = std::make_unique<StandaloneAppMainWindow>(getApplicationName());
    mainWindow->initialise();
}

void OctaChainer2StandaloneApplication::shutdown()
{
    mainWindow = nullptr;
}

void OctaChainer2StandaloneApplication::systemRequestedQuit()
{
    mainWindow->saveAudioSettings();
    quit();
}

void OctaChainer2StandaloneApplication::anotherInstanceStarted(const juce::String& commandLine)
{
    juce::ignoreUnused(commandLine);
}

START_JUCE_APPLICATION(OctaChainer2StandaloneApplication)
