#include "StandaloneAppMainWindow.h"

#include "Core/AudioUtil.h"
#include "UI/MainComponent.h"

StandaloneAppMainWindow::StandaloneAppMainWindow(const juce::String& name)
    : DocumentWindow(name,
                     juce::Colours::black,
                     juce::DocumentWindow::allButtons),
      menuBarModel(
          [] { juce::JUCEApplication::getInstance()->systemRequestedQuit(); },
          [this] { saveProject(); },
          [this] { loadProject(); },
          [this] { showAudioSettings(); }
      )
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
    audioDeviceManager.removeAudioCallback(this);

    if (mainComponent != nullptr)
        mainComponent->removeListener(this);

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
    audioDeviceManager.addAudioCallback(this);

    setUsingNativeTitleBar(true);
    setResizable(true, true);
    setResizeLimits(MainComponent::minWidth, MainComponent::minHeight, MainComponent::maxWidth,
                    MainComponent::maxHeight);

    setContentOwned(new MainComponent(stateHandler), false);
    mainComponent = dynamic_cast<MainComponent*>(getContentComponent());
    jassert(mainComponent != nullptr);
    mainComponent->addListener(this);

    centreWithSize(MainComponent::defaultWidth, MainComponent::defaultHeight);
    setVisible(true);
}

void StandaloneAppMainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

void StandaloneAppMainWindow::transportButtonPressed(const TransportButtonComponent::TransportEvent event)
{
    if (event == TransportButtonComponent::TransportEvent::PlayChain)
    {
        DBG("PlayChain transport is not implemented yet");
    }
    else if (event == TransportButtonComponent::TransportEvent::PlaySlice)
    {
        juce::AudioBuffer<float> audioData;
        double sampleRate = 0.0;

        if (stateHandler.loadSelectedSliceAudio(audioData, sampleRate))
        {
            const auto renderedClip = AudioUtil::renderPlaybackClip(
                    AudioClip(std::move(audioData), sampleRate),
                    audioPlaybackEngine.deviceSampleRate,
                    audioPlaybackEngine.deviceChannelCount);
            audioPlaybackEngine.play(renderedClip);
        }
        else
        {
            DBG("No selected slice audio is available for playback");
            audioPlaybackEngine.stop();
        }
    }
    else if (event == TransportButtonComponent::TransportEvent::Stop)
    {
        audioPlaybackEngine.stop();
    }
}

void StandaloneAppMainWindow::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    audioPlaybackEngine.deviceSampleRate = device->getCurrentSampleRate();
    audioPlaybackEngine.deviceChannelCount = device->getActiveOutputChannels().countNumberOfSetBits();
}

void StandaloneAppMainWindow::audioDeviceStopped()
{
    audioPlaybackEngine.stop();
}

void StandaloneAppMainWindow::audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels,
                                                               float* const* outputChannelData, const int numOutputChannels,
                                                               const int numSamples, const juce::AudioIODeviceCallbackContext& context)
{
    juce::ignoreUnused(inputChannelData, numInputChannels, context);

    if (outputChannelData == nullptr || numOutputChannels <= 0 || numSamples <= 0)
        return;

    juce::AudioBuffer<float> outputBuffer(outputChannelData, numOutputChannels, numSamples);
    audioPlaybackEngine.ProcessBlock(outputBuffer);
}

void StandaloneAppMainWindow::showAudioSettings()
{
    auto selector = std::make_unique<juce::AudioDeviceSelectorComponent>(
        audioDeviceManager,
        0, 0,
        0, 2,
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

        if (!settingsDirectory.createDirectory())
        {
            DBG("Failed to create audio settings directory");
            return;
        }

        const juce::File settingsFile = settingsDirectory.getChildFile("AudioSettings.xml");

        if (!xml->writeTo(settingsFile))
        DBG("Failed to write audio settings");
    }
}

void StandaloneAppMainWindow::loadAudioSettings()
{
    const juce::File settingsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                    .getChildFile(ProjectInfo::projectName)
                                    .getChildFile("AudioSettings.xml");

    if (settingsFile.existsAsFile())
    {
        const std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(settingsFile));

        if (xml != nullptr)
        {
            audioDeviceManager.initialise(0, 2, xml.get(), true);
            return;
        }
    }

    audioDeviceManager.initialiseWithDefaultDevices(0, 2);
}

void StandaloneAppMainWindow::saveProject()
{
    fileChooser = std::make_unique<juce::FileChooser>("Save current project", juce::File(), "*.xml");
    constexpr auto browserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles |
        juce::FileBrowserComponent::warnAboutOverwriting;
    fileChooser->launchAsync(browserFlags, [this](const juce::FileChooser& chooser)
    {
        auto xmlFile = chooser.getResult();
        if (xmlFile != juce::File{})
        {
            if (xmlFile.getFileExtension() != ".xml")
            {
                xmlFile = xmlFile.withFileExtension(".xml");
            }

            const std::unique_ptr<juce::XmlElement> xml(stateHandler.createXml());
            if (xml != nullptr)
            {
                if (!xml->writeTo(xmlFile))
                    DBG("Failed to save project");
            }
        }
    });
}

void StandaloneAppMainWindow::loadProject()
{
    fileChooser = std::make_unique<juce::FileChooser>("Select a project file to load", juce::File(), "*.xml");
    constexpr auto browserFlags = juce::FileBrowserComponent::openMode;
    fileChooser->launchAsync(browserFlags, [this](const juce::FileChooser& chooser)
    {
        const auto xmlFile = chooser.getResult();
        if (xmlFile.existsAsFile())
        {
            if (const auto xml = juce::XmlDocument::parse(xmlFile))
            {
                const juce::ValueTree newTree = juce::ValueTree::fromXml(*xml);
                stateHandler.setState(newTree);
            }
        }
    });
}
