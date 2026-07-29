#include "MainComponent.h"
#include "../Core/ChainExporter.h"
#include "../Core/MegabreakExporter.h"

MainComponent::MainComponent(StateHandler& stateHandlerToUse, AudioPlaybackEngine& audioPlaybackEngineToUse)
    : stateHandler(stateHandlerToUse),
      audioPlaybackEngine(audioPlaybackEngineToUse),
      otReader(std::make_shared<OtReader>()),
      sampleListComponent(PanelComponent::Dimension::percentage(sampleListHeightPercentage, sampleListMinHeight),
                          PanelComponent::Dimension::percentage(sampleListWidthPercentage),
                          stateHandler),
      settingsPanelComponent(PanelComponent::Dimension::fixed(settingsPanelFixedHeight),
                             PanelComponent::Dimension::fixed(settingsPanelFixedWidth),
                             stateHandler),
      sliceWaveformComponent(PanelComponent::Dimension::percentage(sliceWaveformHeightPercentage),
                             PanelComponent::Dimension::percentage(sliceWaveformHeightPercentage),
                             "Slice waveform"),
      chainWaveformComponent(PanelComponent::Dimension::percentage(chainWaveformWidthPercentage),
                             PanelComponent::Dimension::percentage(chainWaveformWidthPercentage),
                             "Chain waveform"),
      audioPanelComponent(PanelComponent::Dimension::fixed(audioSectionFixedHeight),
                          PanelComponent::Dimension::fixed(settingsPanelFixedWidth),
                          stateHandler),
      tooltipWindow(this)
{
    setLookAndFeel(&style);
    addAndMakeVisible(sampleListComponent);
    addAndMakeVisible(settingsPanelComponent);
    addAndMakeVisible(sliceWaveformComponent);
    addAndMakeVisible(chainWaveformComponent);
    addAndMakeVisible(audioPanelComponent);

    stateHandler.addListener(this);
    audioPlaybackEngine.addActionListener(&audioPanelComponent);
    audioPanelComponent.addListener(this);
    settingsPanelComponent.addListener(this);
    sliceWaveformComponent.addListener(this);
    chainWaveformComponent.addListener(this);
    updateSliceWaveform();
    chainRenderThread = std::thread([this]
    {
        chainRenderThreadLoop();
    });
    updateChainWaveform();

    startTimerHz(60);
}


MainComponent::~MainComponent()
{
    stopChainRenderThread();
    stopTimer();
    stateHandler.removeListener(this);
    audioPanelComponent.removeListener(this);
    settingsPanelComponent.removeListener(this);
    sliceWaveformComponent.removeListener(this);
    chainWaveformComponent.removeListener(this);
    setLookAndFeel(nullptr);
}

void MainComponent::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const juce::ColourGradient backgroundGradient{
        juce::Colour(StyleSheet::defaultBackgroundColour), 0, 0, juce::Colour(StyleSheet::backgroundGradientColour),
        bounds.getWidth(), bounds.getHeight(), false
    };
    g.setGradientFill(backgroundGradient);
    g.fillRect(bounds);
}

void MainComponent::resized()
{
    auto contentArea = getLocalBounds().reduced(juce::roundToInt(StyleSheet::panelMargins));
    const auto availableWidth = contentArea.getWidth();
    const auto availableHeight = contentArea.getHeight();

    const auto settingsWidth = settingsPanelComponent.getResolvedWidth(availableWidth);
    const auto settingsHeight = settingsPanelComponent.getResolvedHeight(availableHeight);
    const auto audioPanelHeight = audioPanelComponent.getResolvedHeight(availableHeight);
    const auto sampleListWidth = juce::jmax(0, availableWidth - settingsWidth);

    const auto sliceHeight = juce::jmin(sliceWaveformComponent.getResolvedHeight(availableHeight), availableHeight);
    auto topBand = contentArea.removeFromTop(sampleListComponent.getResolvedHeight(availableHeight));
    auto bottomBand = contentArea;

    sampleListComponent.setBounds(topBand.removeFromLeft(sampleListWidth));
    settingsPanelComponent.setBounds(topBand.removeFromTop(settingsHeight).withWidth(settingsWidth));
    audioPanelComponent.setBounds(topBand.removeFromTop(audioPanelHeight));

    sliceWaveformComponent.setBounds(bottomBand.removeFromTop(sliceHeight));
    chainWaveformComponent.setBounds(bottomBand);
}

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress('p', juce::ModifierKeys::ctrlModifier, 0)
        && audioPanelComponent.btnPlayChain.getButton().isEnabled())
    {
        audioPanelComponent.btnPlayChain.getButton().triggerClick();
        return true;
    }

    return false;
}

bool MainComponent::isOtFilePath(const juce::String& path) const
{
    return juce::File(path).getFileExtension().equalsIgnoreCase(".ot");
}

bool MainComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (int i = 0; i < files.size(); ++i)
    {
        if (isOtFilePath(files[i]))
            return true;
    }

    return false;
}

void MainComponent::filesDropped(const juce::StringArray& files, int, int)
{
    for (int i = 0; i < files.size(); ++i)
    {
        if (isOtFilePath(files[i]))
        {
            importOtFile(juce::File(files[i]));
            break;
        }
    }
}

void MainComponent::importOtFile(const juce::File& otFile)
{
    if (otReader != nullptr)
        otReader->importOtFile(otFile, stateHandler, this);
}

void MainComponent::stateChanged(const StateHandler::StateChange& change)
{
    if (change.has(StateHandler::StateChange::sliceList)
        || change.has(StateHandler::StateChange::selectedSlice)
        || change.has(StateHandler::StateChange::fullReload))
        updateSliceWaveform();

    if (change.has(StateHandler::StateChange::sliceList)
        || isChainRenderRelevantSetting(change)
        || change.has(StateHandler::StateChange::fullReload))
    {
        updateChainWaveform();
    }
    else if (change.has(StateHandler::StateChange::selectedSlice))
    {
        if (isSelectedSliceInCurrentChain())
            refreshChainWaveformSelection();
        else
            updateChainWaveform();
    }

    if ((change.has(StateHandler::StateChange::settings) && change.isSetting(stateHandler.masterVolumeId))
        || change.has(StateHandler::StateChange::fullReload))
    {
        audioPlaybackEngine.gain.store(stateHandler.getStateValue<float>(stateHandler.masterVolumeId, 0.5f));
    }
}

bool MainComponent::isChainRenderRelevantSetting(const StateHandler::StateChange& change) const
{
    if (! change.has(StateHandler::StateChange::settings))
        return false;

    return change.isSetting(StateHandler::normalizationId)
        || change.isSetting(StateHandler::channelsId)
        || change.isSetting(StateHandler::fadeinId)
        || change.isSetting(StateHandler::fadeoutId)
        || change.isSetting(StateHandler::evenGridId);
}

void MainComponent::transportButtonPressed(const TransportButtonComponent::TransportEvent event)
{
    sendTransportEvent(event);
}

void MainComponent::chainExportRequested()
{
    saveChainToFile();
}

void MainComponent::megabreakExportRequested()
{
    saveMegabreakToFile();
}

void MainComponent::waveformSegmentClicked(const int segmentIndex, const int sliceIndex)
{
    juce::ignoreUnused(segmentIndex);

    if (sliceIndex < 0)
        return;

    stateHandler.selectSlice(sliceIndex);
}

void MainComponent::waveformSliceRangeChanged(const int startSample, const int endSample)
{
    stateHandler.setSelectedSliceRange(startSample, endSample);
}

void MainComponent::updateSliceWaveform()
{
    juce::AudioBuffer<float> audioData;
    double sampleRate = 0.0;
    const auto sliceTree = stateHandler.getSelectedSliceTree();

    if (stateHandler.loadFullSelectedSliceAudio(audioData, sampleRate))
    {
        sliceWaveformComponent.setAudioData(audioData, sampleRate);

        if (sliceTree.isValid())
        {
            const auto totalSamples = audioData.getNumSamples();
            const auto startSample = juce::jlimit(0, totalSamples,
                                                 static_cast<int>(sliceTree.getProperty(StateHandler::sliceStartSampleId, 0)));
            auto endSample = juce::jlimit(0, totalSamples,
                                          static_cast<int>(sliceTree.getProperty(StateHandler::sliceEndSampleId, totalSamples)));

            if (endSample <= startSample)
                endSample = juce::jmin(totalSamples, startSample + 1);

            sliceWaveformComponent.setSliceRange(startSample, endSample);
        }
        else
        {
            sliceWaveformComponent.setSliceRange(0, audioData.getNumSamples());
        }
    }
    else
    {
        sliceWaveformComponent.clearAudioData();
    }
}

void MainComponent::updateChainWaveform()
{
    requestChainRender(true);
}

void MainComponent::saveChainToFile()
{
    const juce::File defaultFolder(stateHandler.getStateValue(
        StateHandler::defaultExportFolderId,
        juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName()));

    chainExportChooser = std::make_unique<juce::FileChooser>("Save chain as", defaultFolder.getChildFile("chain.wav"), "*.wav");
    constexpr auto browserFlags = juce::FileBrowserComponent::saveMode
                                  | juce::FileBrowserComponent::canSelectFiles
                                  | juce::FileBrowserComponent::warnAboutOverwriting;

    const juce::Component::SafePointer<MainComponent> safeThis(this);
    chainExportChooser->launchAsync(browserFlags, [safeThis](const juce::FileChooser& chooser)
    {
        if (safeThis == nullptr)
            return;

        auto file = chooser.getResult();
        if (file == juce::File{})
            return;

        file = file.withFileExtension(".wav");

        auto stateXml = std::unique_ptr<juce::XmlElement>(safeThis->stateHandler.createXml());
        const auto targetSampleRate = static_cast<double>(safeThis->stateHandler.getStateValue<int>(
            StateHandler::samplerateId,
            44100));
        const auto bitDepth = safeThis->stateHandler.getStateValue<int>(StateHandler::bitDepthId, 16);
        const auto playbackClip = safeThis->audioPlaybackEngine.getCurrentClip();
        const auto chainClip = safeThis->chain.getAudioClip();

        if (playbackClip != nullptr && playbackClip == chainClip)
            safeThis->audioPlaybackEngine.stop();

        safeThis->cancelChainRender();
        safeThis->clearPlaybackChain();

        std::thread([safeThis, file, exportStateXml = std::move(stateXml), targetSampleRate, bitDepth]() mutable
        {
            juce::String errorMessage;
            bool success = false;

            if (safeThis != nullptr)
            {
                const auto baseState = exportStateXml != nullptr ? juce::ValueTree::fromXml(*exportStateXml) : juce::ValueTree{};
                const auto settingsTree = baseState.getChildWithName(StateHandler::settingsId);
                const auto dataTree = baseState.getChildWithName(StateHandler::dataId);
                const auto numSlices = dataTree.isValid() ? dataTree.getNumChildren() : 0;
                const auto maxSlicesPerChain = juce::jmax(1, static_cast<int>(settingsTree.getProperty(
                    StateHandler::chainMaxLengthId,
                    static_cast<int>(StateHandler::chainMaxLengthValue.defaultValue))));
                const auto chainCount = juce::jmax(1, (numSlices + maxSlicesPerChain - 1) / maxSlicesPerChain);

                success = true;
                for (int chainIndex = 0; chainIndex < chainCount; ++chainIndex)
                {
                    auto exportState = exportStateXml != nullptr ? juce::ValueTree::fromXml(*exportStateXml) : juce::ValueTree{};
                    auto exportDataTree = exportState.getChildWithName(StateHandler::dataId);
                    if (exportDataTree.isValid())
                    {
                        const auto startSlice = chainIndex * maxSlicesPerChain;
                        exportDataTree.setProperty(StateHandler::selectedSliceId, startSlice, nullptr);
                    }

                    const auto exportFile = chainCount > 1
                        ? file.getSiblingFile(file.getFileNameWithoutExtension() + "_" + juce::String(chainIndex + 1)
                                              + file.getFileExtension())
                        : file;

                    if (! ChainExporter::exportToFile(exportFile, exportState, targetSampleRate, bitDepth, &errorMessage))
                    {
                        success = false;
                        if (chainCount > 1 && errorMessage.isNotEmpty())
                        {
                            errorMessage = "Failed to save " + exportFile.getFileName() + ": " + errorMessage;
                        }
                        break;
                    }
                }
            }

            juce::MessageManager::callAsync([safeThis, success, asyncErrorMessage = std::move(errorMessage)]() mutable
            {
                if (! success && safeThis != nullptr)
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                           "Could not save chain",
                                                           asyncErrorMessage.isNotEmpty() ? asyncErrorMessage : "The chain could not be rendered.");
                }

                if (safeThis != nullptr)
                    safeThis->requestChainRender(false);
            });
        }).detach();
    });
}

void MainComponent::saveMegabreakToFile()
{
    const juce::File defaultFolder(stateHandler.getStateValue(
        StateHandler::defaultExportFolderId,
        juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName()));

    chainExportChooser = std::make_unique<juce::FileChooser>("Save megabreak as", defaultFolder.getChildFile("megabreak.wav"), "*.wav");
    constexpr auto browserFlags = juce::FileBrowserComponent::saveMode
                                  | juce::FileBrowserComponent::canSelectFiles
                                  | juce::FileBrowserComponent::warnAboutOverwriting;

    const juce::Component::SafePointer<MainComponent> safeThis(this);
    chainExportChooser->launchAsync(browserFlags, [safeThis](const juce::FileChooser& chooser)
    {
        if (safeThis == nullptr)
            return;

        auto file = chooser.getResult();
        if (file == juce::File{})
            return;

        file = file.withFileExtension(".wav");

        auto stateXml = std::unique_ptr<juce::XmlElement>(safeThis->stateHandler.createXml());
        const auto targetSampleRate = static_cast<double>(safeThis->stateHandler.getStateValue<int>(
            StateHandler::samplerateId,
            44100));
        const auto bitDepth = safeThis->stateHandler.getStateValue<int>(StateHandler::bitDepthId, 16);
        const auto partCount = juce::jmax(1, safeThis->stateHandler.getStateValue<int>(
            StateHandler::megabreakFileCountId,
            1));
        const auto playbackClip = safeThis->audioPlaybackEngine.getCurrentClip();
        const auto chainClip = safeThis->chain.getAudioClip();

        if (playbackClip != nullptr && playbackClip == chainClip)
            safeThis->audioPlaybackEngine.stop();

        safeThis->cancelChainRender();
        safeThis->clearPlaybackChain();

        std::thread([safeThis, file, exportStateXml = std::move(stateXml), targetSampleRate, bitDepth, partCount]() mutable
        {
            juce::String errorMessage;
            bool success = false;

            if (safeThis != nullptr)
            {
                const auto baseState = exportStateXml != nullptr ? juce::ValueTree::fromXml(*exportStateXml) : juce::ValueTree{};
                success = MegabreakExporter::exportToFiles(file, baseState, targetSampleRate, bitDepth, partCount, &errorMessage);
            }

            juce::MessageManager::callAsync([safeThis, success, asyncErrorMessage = std::move(errorMessage)]() mutable
            {
                if (! success && safeThis != nullptr)
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                           "Could not save megabreak",
                                                           asyncErrorMessage.isNotEmpty() ? asyncErrorMessage : "The megabreak could not be rendered.");
                }

                if (safeThis != nullptr)
                    safeThis->requestChainRender(false);
            });
        }).detach();
    });
}

void MainComponent::clearPlaybackChain()
{
    chain.clear();
    chainWaveformComponent.clearAudioData();
    audioPanelComponent.setChainReady(false);
}

void MainComponent::cancelChainRender()
{
    chainRenderLatestRequestId.fetch_add(1, std::memory_order_acq_rel);
    {
        const std::scoped_lock lock(chainRenderMutex);
        chainRenderHasPending = false;
        chainRenderPendingState = {};
        chainRenderPendingSampleRate = 0.0;
        chainRenderPendingRequestId = chainRenderLatestRequestId.load(std::memory_order_acquire);
    }

    chainRenderCondition.notify_all();
}

void MainComponent::refreshChainWaveformSelection()
{
    chainWaveformComponent.setSelectedSegmentIndex(stateHandler.getSelectedSliceIndex());
}

void MainComponent::requestChainRender(const bool stopPlayback)
{
    if (stopPlayback)
        audioPlaybackEngine.stop();

    const auto targetSampleRate = audioPlaybackEngine.deviceSampleRate;
    const auto numSlices = stateHandler.getNumSlices();

    if (targetSampleRate <= 0.0 || numSlices <= 0)
    {
        cancelChainRender();
        clearPlaybackChain();
        return;
    }

    {
        const auto requestId = chainRenderLatestRequestId.fetch_add(1, std::memory_order_acq_rel) + 1;
        const std::scoped_lock lock(chainRenderMutex);
        chainRenderPendingState = stateHandler.getState();
        chainRenderPendingSampleRate = targetSampleRate;
        chainRenderPendingRequestId = requestId;
        chainRenderHasPending = true;
    }

    clearPlaybackChain();
    chainWaveformComponent.setProcessingState(true);
    audioPanelComponent.setChainReady(false);
    chainRenderCondition.notify_one();
}

void MainComponent::finishChainRender(const std::uint64_t requestId, const std::shared_ptr<Chain>& renderedChain,
                                      const juce::String& renderError)
{
    if (requestId != chainRenderLatestRequestId.load(std::memory_order_acquire))
        return;

    if (renderedChain == nullptr || ! renderedChain->isValid())
    {
        chain.clear();
        chainWaveformComponent.clearAudioData();
        audioPanelComponent.setChainReady(false);

        if (renderError.isNotEmpty())
            showChainRenderError(renderError);

        return;
    }

    chain = *renderedChain;

    if (chain.isValid())
    {
        const auto selectedSliceIndex = stateHandler.getSelectedSliceIndex();
        chainWaveformComponent.setAudioClip(chain.getAudioClip(),
                                            chain.getSegments(),
                                            selectedSliceIndex);
        audioPanelComponent.setChainReady(true);
    }
    else
    {
        chainWaveformComponent.clearAudioData();
        audioPanelComponent.setChainReady(false);
    }
}

void MainComponent::showChainRenderError(const juce::String& message)
{
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                           "Could not render chain",
                                           message);
}

bool MainComponent::isSelectedSliceInCurrentChain() const
{
    const auto selectedSliceIndex = stateHandler.getSelectedSliceIndex();
    if (selectedSliceIndex < 0 || ! chain.isValid())
        return false;

    const auto& segments = chain.getSegments();
    return std::any_of(segments.begin(), segments.end(),
                       [selectedSliceIndex](const Chain::Segment& segment)
                       {
                           return segment.sliceIndex == selectedSliceIndex;
                       });
}

void MainComponent::chainRenderThreadLoop()
{
    for (;;)
    {
        juce::ValueTree pendingState;
        double targetSampleRate = 0.0;
        std::uint64_t requestId = 0;

        {
            std::unique_lock lock(chainRenderMutex);
            chainRenderCondition.wait(lock, [this]
            {
                return chainRenderExitRequested.load(std::memory_order_acquire) || chainRenderHasPending;
            });

            if (chainRenderExitRequested.load(std::memory_order_acquire))
                break;

            pendingState = chainRenderPendingState;
            targetSampleRate = chainRenderPendingSampleRate;
            requestId = chainRenderPendingRequestId;
            chainRenderHasPending = false;
        }

        auto renderedChain = std::make_shared<Chain>();
        const auto shouldAbort = [this, requestId]
        {
            return chainRenderExitRequested.load(std::memory_order_acquire)
                || chainRenderLatestRequestId.load(std::memory_order_acquire) != requestId;
        };

        juce::String renderError;
        const auto completed = renderedChain->create(pendingState, targetSampleRate, shouldAbort, &renderError);

        const juce::Component::SafePointer<MainComponent> safeThis(this);
        if (completed)
        {
            juce::MessageManager::callAsync([safeThis, requestId, renderedChain, asyncRenderError = std::move(renderError)]() mutable
            {
                if (safeThis == nullptr)
                    return;

                safeThis->finishChainRender(requestId, renderedChain, asyncRenderError);
            });
        }
        else if (! shouldAbort())
        {
            juce::MessageManager::callAsync([safeThis, requestId, asyncRenderError = std::move(renderError)]() mutable
            {
                if (safeThis == nullptr)
                    return;

                safeThis->finishChainRender(requestId, nullptr, asyncRenderError);
            });
        }
    }
}

void MainComponent::stopChainRenderThread()
{
    chainRenderExitRequested.store(true, std::memory_order_release);
    chainRenderCondition.notify_all();

    if (chainRenderThread.joinable())
        chainRenderThread.join();
}

void MainComponent::sendTransportEvent(TransportButtonComponent::TransportEvent event)
{
    listeners.call([event](Listener& l)
    {
        l.transportButtonPressed(event);
    });
}

void MainComponent::addListener(Listener* listener) {
    listeners.add(listener);
}

void MainComponent::removeListener(Listener* listenerToRemove) {
    jassert(listeners.contains(listenerToRemove));
    listeners.remove(listenerToRemove);
}

void MainComponent::detachPlaybackListener()
{
    audioPlaybackEngine.removeActionListener(&audioPanelComponent);
}

void MainComponent::timerCallback()
{
    const auto playHeadPositionFactor = audioPlaybackEngine.getCurrentPlaybackPositionFactor();
    const bool sliceIsPlaying = audioPanelComponent.btnPlaySlice.getButton().getToggleState();
    const bool chainIsPlaying = audioPanelComponent.btnPlayChain.getButton().getToggleState();

    sliceWaveformComponent.setPlayHeadPositionFactor(sliceIsPlaying ? playHeadPositionFactor : 0.0);
    chainWaveformComponent.setPlayHeadPositionFactor(chainIsPlaying ? playHeadPositionFactor : 0.0);
}
