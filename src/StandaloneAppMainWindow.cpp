#include "StandaloneAppMainWindow.h"
#include "Core/AudioUtil.h"
#include "UI/AboutDialogComponent.h"
#include "UI/DefaultSettingsDialogComponent.h"

StandaloneAppMainWindow::StandaloneAppMainWindow(const juce::String& name)
    : DocumentWindow(name,
                     juce::Colours::black,
                     juce::DocumentWindow::allButtons),
      menuBarModel(
          [] { juce::JUCEApplication::getInstance()->systemRequestedQuit(); },
          [this] { saveProject(); },
          [this] { loadProject(); },
          [this] { importOtFile(); },
          [this] { showAudioSettings(); },
          [this] { showDefaultSettings(); },
          [this] { showAboutDialog(); }
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
    if (mainComponent != nullptr)
        mainComponent->detachPlaybackListener();

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
    loadDefaultSettings();
    loadCurrentSettingsDefaults();
    audioDeviceManager.addAudioCallback(this);

    setUsingNativeTitleBar(true);
    setResizable(true, true);
    setResizeLimits(MainComponent::minWidth, MainComponent::minHeight, MainComponent::maxWidth,
                    MainComponent::maxHeight);

    setContentOwned(new MainComponent(stateHandler, audioPlaybackEngine), false);
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
        const auto clip = mainComponent->getChainAudioClip();
        if (clip != nullptr && clip->isValid())
            audioPlaybackEngine.play(clip);
        else
        {
            DBG("No rendered chain is available for playback");
            audioPlaybackEngine.stop();
        }
    }
    else if (event == TransportButtonComponent::TransportEvent::PlaySlice)
    {
        juce::AudioBuffer<float> audioData;
        double sampleRate = 0.0;

        if (stateHandler.loadSelectedSliceRangeAudio(audioData, sampleRate))
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

    if (mainComponent != nullptr)
    {
        const juce::Component::SafePointer<MainComponent> safeMainComponent(mainComponent);
        juce::MessageManager::callAsync([safeMainComponent]() mutable
        {
            if (safeMainComponent != nullptr)
                safeMainComponent->updateChainWaveform();
        });
    }
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

    juce::AudioBuffer outputBuffer(outputChannelData, numOutputChannels, numSamples);
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

void StandaloneAppMainWindow::showDefaultSettings()
{
    auto defaults = std::make_unique<DefaultSettingsDialogComponent>(
        stateHandler.getStateValue(StateHandler::defaultAudioFolderId,
                                   juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName()),
        stateHandler.getStateValue(StateHandler::defaultExportFolderId,
                                   juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName()),
        stateHandler.getStateValue(StateHandler::defaultProjectFolderId,
                                   juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName()),
        [this](const juce::String& defaultAudioFolder, const juce::String& defaultExportFolder,
               const juce::String& defaultProjectFolder)
        {
            stateHandler.setStateValue(StateHandler::defaultAudioFolderId, defaultAudioFolder);
            stateHandler.setStateValue(StateHandler::defaultExportFolderId, defaultExportFolder);
            stateHandler.setStateValue(StateHandler::defaultProjectFolderId, defaultProjectFolder);
        },
        [this]
        {
            saveCurrentSettingsAsDefaults();
        });

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(defaults.release());
    options.dialogTitle = "Application default settings";
    options.dialogBackgroundColour = juce::Colour(StyleSheet::dialogBackgroundColour);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    options.componentToCentreAround = this;

    options.launchAsync();
}

void StandaloneAppMainWindow::showAboutDialog()
{
    auto about = std::make_unique<AboutDialogComponent>(
        "OctaChainer 2",
        ProjectInfo::versionString
    );

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(about.release());
    options.dialogTitle = "About OctaChainer 2";
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

void StandaloneAppMainWindow::saveDefaultSettings() const
{
    const juce::File settingsDirectory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(ProjectInfo::projectName);

    if (! settingsDirectory.createDirectory())
    {
        DBG("Failed to create default folder settings directory");
        return;
    }

    juce::XmlElement xml("DefaultFolders");
    xml.setAttribute("audioFolder", stateHandler.getStateValue(
        StateHandler::defaultAudioFolderId,
        juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName()));
    xml.setAttribute("exportFolder", stateHandler.getStateValue(
        StateHandler::defaultExportFolderId,
        juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName()));
    xml.setAttribute("projectFolder", stateHandler.getStateValue(
        StateHandler::defaultProjectFolderId,
        juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName()));

    const juce::File settingsFile = settingsDirectory.getChildFile("DefaultFolders.xml");
    if (! xml.writeTo(settingsFile))
        DBG("Failed to write default folder settings");
}

void StandaloneAppMainWindow::saveCurrentSettingsAsDefaults() const
{
    const juce::File settingsDirectory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(ProjectInfo::projectName);

    if (! settingsDirectory.createDirectory())
    {
        DBG("Failed to create default settings directory");
        return;
    }

    juce::XmlElement xml("DefaultSettings");
    xml.setAttribute("bitDepth", stateHandler.getCurrentOption(StateHandler::bitDepthId).itemId);
    xml.setAttribute("channels", stateHandler.getCurrentOption(StateHandler::channelsId).itemId);
    xml.setAttribute("samplerate", stateHandler.getCurrentOption(StateHandler::samplerateId).itemId);
    xml.setAttribute("timestretch", stateHandler.getCurrentOption(StateHandler::timestretchId).itemId);
    xml.setAttribute("loopMode", stateHandler.getCurrentOption(StateHandler::loopModeId).itemId);
    xml.setAttribute("triqQuant", stateHandler.getCurrentOption(StateHandler::triqQuantId).itemId);
    xml.setAttribute("gain", stateHandler.getStateValue<double>(StateHandler::gainId, StateHandler::gainValue.defaultValue));
    xml.setAttribute("bpm", stateHandler.getStateValue<double>(StateHandler::bpmId, StateHandler::bpmValue.defaultValue));
    xml.setAttribute("normalization", stateHandler.getCurrentOption(StateHandler::normalizationId).itemId);
    xml.setAttribute("fadein", stateHandler.getCurrentOption(StateHandler::fadeinId).itemId);
    xml.setAttribute("fadeout", stateHandler.getCurrentOption(StateHandler::fadeoutId).itemId);
    xml.setAttribute("otFile", stateHandler.getStateValue<bool>(StateHandler::otFileId, StateHandler::otFileDefault));
    xml.setAttribute("evenGrid", stateHandler.getStateValue<bool>(StateHandler::evenGridId, StateHandler::evenGridDefault));
    xml.setAttribute("embedMarkers", stateHandler.getStateValue<bool>(StateHandler::embedMarkersId, StateHandler::embedMarkersDefault));
    xml.setAttribute("megabreakFileCount", stateHandler.getCurrentOption(StateHandler::megabreakFileCountId).itemId);
    xml.setAttribute("chainMaxLength", stateHandler.getStateValue<int>(StateHandler::chainMaxLengthId,
                                                                       static_cast<int>(StateHandler::chainMaxLengthValue.defaultValue)));
    xml.setAttribute("masterVolume", stateHandler.getStateValue<float>(StateHandler::masterVolumeId, 0.5f));

    const juce::File settingsFile = settingsDirectory.getChildFile("DefaultSettings.xml");
    if (! xml.writeTo(settingsFile))
        DBG("Failed to write default settings");
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

void StandaloneAppMainWindow::loadDefaultSettings()
{
    const juce::File settingsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                    .getChildFile(ProjectInfo::projectName)
                                    .getChildFile("DefaultFolders.xml");

    if (! settingsFile.existsAsFile())
        return;

    const std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(settingsFile));
    if (xml == nullptr || ! xml->hasTagName("DefaultFolders"))
        return;

    const auto audioFolder = xml->getStringAttribute("audioFolder");
    const auto exportFolder = xml->getStringAttribute("exportFolder");
    const auto projectFolder = xml->getStringAttribute("projectFolder");

    if (audioFolder.isNotEmpty())
        stateHandler.setStateValue(StateHandler::defaultAudioFolderId, audioFolder);

    if (exportFolder.isNotEmpty())
        stateHandler.setStateValue(StateHandler::defaultExportFolderId, exportFolder);

    if (projectFolder.isNotEmpty())
        stateHandler.setStateValue(StateHandler::defaultProjectFolderId, projectFolder);
}

void StandaloneAppMainWindow::loadCurrentSettingsDefaults()
{
    const juce::File settingsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                    .getChildFile(ProjectInfo::projectName)
                                    .getChildFile("DefaultSettings.xml");

    if (! settingsFile.existsAsFile())
        return;

    const std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(settingsFile));
    if (xml == nullptr || ! xml->hasTagName("DefaultSettings"))
        return;

    const auto applyItemId = [this, xmlPtr = xml.get()](const juce::Identifier& id, const char* attributeName)
    {
        if (xmlPtr->hasAttribute(attributeName))
            stateHandler.setStateValueFromItemId(id, xmlPtr->getIntAttribute(attributeName));
    };

    applyItemId(StateHandler::bitDepthId, "bitDepth");
    applyItemId(StateHandler::channelsId, "channels");
    applyItemId(StateHandler::samplerateId, "samplerate");
    applyItemId(StateHandler::timestretchId, "timestretch");
    applyItemId(StateHandler::loopModeId, "loopMode");
    applyItemId(StateHandler::triqQuantId, "triqQuant");
    applyItemId(StateHandler::normalizationId, "normalization");
    applyItemId(StateHandler::fadeinId, "fadein");
    applyItemId(StateHandler::fadeoutId, "fadeout");
    applyItemId(StateHandler::megabreakFileCountId, "megabreakFileCount");

    if (xml->hasAttribute("gain"))
        stateHandler.setStateValue(StateHandler::gainId, xml->getDoubleAttribute("gain"));

    if (xml->hasAttribute("bpm"))
        stateHandler.setStateValue(StateHandler::bpmId, xml->getDoubleAttribute("bpm"));

    if (xml->hasAttribute("chainMaxLength"))
        stateHandler.setStateValue(StateHandler::chainMaxLengthId, xml->getIntAttribute("chainMaxLength"));

    if (xml->hasAttribute("masterVolume"))
        stateHandler.setStateValue(StateHandler::masterVolumeId, static_cast<float>(xml->getDoubleAttribute("masterVolume")));

    if (xml->hasAttribute("otFile"))
        stateHandler.setStateValue(StateHandler::otFileId, xml->getBoolAttribute("otFile"));

    if (xml->hasAttribute("evenGrid"))
        stateHandler.setStateValue(StateHandler::evenGridId, xml->getBoolAttribute("evenGrid"));

    if (xml->hasAttribute("embedMarkers"))
        stateHandler.setStateValue(StateHandler::embedMarkersId, xml->getBoolAttribute("embedMarkers"));
}

void StandaloneAppMainWindow::saveProject()
{
    const juce::File initialFolder(stateHandler.getStateValue(
        StateHandler::defaultProjectFolderId,
        juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName()));
    fileChooser = std::make_unique<juce::FileChooser>("Save current project", initialFolder, "*.xml");
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
    const juce::File initialFolder(stateHandler.getStateValue(
        StateHandler::defaultProjectFolderId,
        juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName()));
    fileChooser = std::make_unique<juce::FileChooser>("Select a project file to load", initialFolder, "*.xml");
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
                loadDefaultSettings();
            }
        }
    });
}

void StandaloneAppMainWindow::importOtFile()
{
    const auto initialFolder = stateHandler.getStateValue(
        StateHandler::defaultExportFolderId,
        juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName());
    fileChooser = std::make_unique<juce::FileChooser>("Import OT file", juce::File(initialFolder), "*.ot");
    constexpr auto browserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    const juce::Component::SafePointer<StandaloneAppMainWindow> safeThis(this);
    fileChooser->launchAsync(browserFlags, [safeThis](const juce::FileChooser& chooser)
    {
        if (safeThis == nullptr)
            return;

        const auto otFile = chooser.getResult();
        if (otFile == juce::File{})
            return;

        if (safeThis->mainComponent != nullptr)
            safeThis->mainComponent->importOtFile(otFile);
    });
}
