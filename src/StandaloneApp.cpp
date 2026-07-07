#include "StandaloneApp.h"
#include "UI/MainComponent.h"

const juce::String OctaChainer2StandaloneApplication::getApplicationName()
{
    return "OctaChainer 2";
}

const juce::String OctaChainer2StandaloneApplication::getApplicationVersion()
{
    return "2.0.0";
}

bool OctaChainer2StandaloneApplication::moreThanOneInstanceAllowed()
{
    return true;
}

void OctaChainer2StandaloneApplication::initialise(const juce::String& commandLine)
{
    juce::ignoreUnused(commandLine);

    mainWindow = std::make_unique<MainWindow>(getApplicationName());
    mainWindow->initialise();
}

void OctaChainer2StandaloneApplication::shutdown()
{
    mainWindow = nullptr;
}

void OctaChainer2StandaloneApplication::systemRequestedQuit()
{
    quit();
}

void OctaChainer2StandaloneApplication::anotherInstanceStarted(const juce::String& commandLine)
{
    juce::ignoreUnused(commandLine);
}

OctaChainer2StandaloneApplication::MainWindow::MainWindow(const juce::String& name)
    : DocumentWindow(name,
                     juce::Colours::black,
                     juce::DocumentWindow::allButtons)
{
}

void OctaChainer2StandaloneApplication::MainWindow::initialise()
{
    setUsingNativeTitleBar(true);
    setResizable(true, true);
    setResizeLimits(MainComponent::minWidth, MainComponent::minHeight, MainComponent::maxWidth,
                    MainComponent::maxHeight);

    setContentOwned(new MainComponent(), false);

    centreWithSize(MainComponent::defaultWidth, MainComponent::defaultHeight);
    setVisible(true);
}

void OctaChainer2StandaloneApplication::MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

START_JUCE_APPLICATION(OctaChainer2StandaloneApplication)