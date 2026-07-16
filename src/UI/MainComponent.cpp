#include "MainComponent.h"

MainComponent::MainComponent(StateHandler& stateHandlerToUse, AudioPlaybackEngine& audioPlaybackEngineToUse)
    : stateHandler(stateHandlerToUse),
      audioPlaybackEngine(audioPlaybackEngineToUse),
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
                          stateHandler)
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
    updateSliceWaveform();

    startTimerHz(60);
}


MainComponent::~MainComponent()
{
    stopTimer();
    stateHandler.removeListener(this);
    audioPanelComponent.removeListener(this);
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

void MainComponent::stateChanged(const StateHandler::StateChange& change)
{
    if (change.has(StateHandler::StateChange::selectedSlice) || change.has(StateHandler::StateChange::fullReload))
        updateSliceWaveform();

    if ((change.has(StateHandler::StateChange::settings) && change.isSetting(stateHandler.masterVolumeId))
        || change.has(StateHandler::StateChange::fullReload))
    {
        audioPlaybackEngine.gain.store(stateHandler.getStateValue<float>(stateHandler.masterVolumeId, 0.5f));
    }
}

void MainComponent::transportButtonPressed(const TransportButtonComponent::TransportEvent event)
{
    sendTransportEvent(event);
}

void MainComponent::updateSliceWaveform()
{
    juce::AudioBuffer<float> audioData;
    double sampleRate = 0.0;

    if (stateHandler.loadSelectedSliceAudio(audioData, sampleRate))
        sliceWaveformComponent.setAudioData(audioData, sampleRate);
    else
        sliceWaveformComponent.clearAudioData();
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
