#include "StandaloneAppMainWindow.h"
#include "UI/MainComponent.h"

StandaloneAppMainWindow::StandaloneAppMainWindow(const juce::String& name)
    : DocumentWindow(name,
                     juce::Colours::black,
                     juce::DocumentWindow::allButtons),
      menuBarModel(
          []
          {
              juce::JUCEApplication::getInstance()->systemRequestedQuit();
          },
          [this]
          {
              showAudioSettings();
          })
{
    setLookAndFeel(&style);

#if JUCE_MAC
    juce::MenuBarModel::setMacMainMenu(&menuBarModel);
#endif

#if ! JUCE_MAC
    setMenuBar(&menuBarModel);
#endif
}

StandaloneAppMainWindow::~StandaloneAppMainWindow()
{
#if JUCE_MAC
    juce::MenuBarModel::setMacMainMenu(nullptr);
#endif

    setMenuBar(nullptr);
    setLookAndFeel(nullptr);
}

void StandaloneAppMainWindow::initialise()
{
    const auto error = audioDeviceManager.initialise(0, 2, nullptr, true);
    if (error.isNotEmpty())
        DBG(error);

    loadAudioSettings();

    setUsingNativeTitleBar(true);
    setResizable(true, true);
    setResizeLimits(MainComponent::minWidth, MainComponent::minHeight, MainComponent::maxWidth,
                    MainComponent::maxHeight);

    setContentOwned(new MainComponent(stateHandler), false);

    centreWithSize(MainComponent::defaultWidth, MainComponent::defaultHeight);
    setVisible(true);
}

void StandaloneAppMainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

void StandaloneAppMainWindow::showAudioSettings()
{
    auto selector = std::make_unique<juce::AudioDeviceSelectorComponent>(
        audioDeviceManager,
        0, 0,
        0, 256,
        false,
        false,
        true,
        false
    );

    selector->setSize(500, 450);

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(selector.release());
    options.dialogTitle = "Audio settings";
    options.dialogBackgroundColour = juce::Colour(StyleSheet::dialogBackgroundColour);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    options.componentToCentreAround = this;

    options.launchAsync();
}

void StandaloneAppMainWindow::saveAudioSettings() const
{
    const std::unique_ptr<juce::XmlElement> xml(audioDeviceManager.createStateXml());

    if (xml != nullptr)
    {
        const juce::File settingsDirectory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                            .getChildFile(ProjectInfo::projectName);

        if (! settingsDirectory.createDirectory())
        {
            DBG("Failed to create audio settings directory");
            return;
        }

        const juce::File settingsFile = settingsDirectory.getChildFile("AudioSettings.xml");

        if (! xml->writeTo(settingsFile))
            DBG("Failed to write audio settings");
    }
}

void StandaloneAppMainWindow::loadAudioSettings()
{
    juce::File settingsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                  .getChildFile(ProjectInfo::projectName)
                                  .getChildFile("AudioSettings.xml");

    if (settingsFile.existsAsFile())
    {
        std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(settingsFile));

        if (xml != nullptr)
        {
            audioDeviceManager.initialise(0, 2, xml.get(), true);
            return;
        }
    }

    audioDeviceManager.initialiseWithDefaultDevices(0, 2);
}
