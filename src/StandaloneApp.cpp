#include "StandaloneApp.h"
#include "Core/HeadlessBatchRunner.h"
#include "UI/MainComponent.h"

#include <iostream>

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
    const auto headlessResult = HeadlessBatchRunner::runFromCommandLine(commandLine);
    if (headlessResult.handled)
    {
        if (headlessResult.output.isNotEmpty())
        {
            if (headlessResult.success)
                std::cout << headlessResult.output << std::endl;
            else
                std::cerr << headlessResult.output << std::endl;
        }

        juce::JUCEApplicationBase::setApplicationReturnValue(headlessResult.exitCode);
        quit();
        return;
    }

    mainWindow = std::make_unique<StandaloneAppMainWindow>(getApplicationName());
    mainWindow->initialise();
}

void OctaChainer2StandaloneApplication::shutdown()
{
    mainWindow = nullptr;
}

void OctaChainer2StandaloneApplication::systemRequestedQuit()
{
    if (mainWindow != nullptr)
    {
        mainWindow->saveAudioSettings();
        mainWindow->saveDefaultSettings();
    }

    quit();
}

void OctaChainer2StandaloneApplication::anotherInstanceStarted(const juce::String& commandLine)
{
    juce::ignoreUnused(commandLine);
}

START_JUCE_APPLICATION(OctaChainer2StandaloneApplication)
