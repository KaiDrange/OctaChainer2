#include "MainComponent.h"

MainComponent::MainComponent(StateHandler& stateHandlerToUse)
    : stateHandler(stateHandlerToUse),
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
                             "Chain waveforms"),
      audioPanelComponent(PanelComponent::Dimension::fixed(audioSectionFixedHeight),
                          PanelComponent::Dimension::fixed(settingsPanelFixedWidth))
{
    setLookAndFeel(&style);
    addAndMakeVisible(sampleListComponent);
    addAndMakeVisible(settingsPanelComponent);
    addAndMakeVisible(sliceWaveformComponent);
    addAndMakeVisible(chainWaveformComponent);
    addAndMakeVisible(audioPanelComponent);

    stateHandler.addListener(this);
    audioPanelComponent.addListener(this);
    updateSliceWaveform();
}


MainComponent::~MainComponent()
{
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

void MainComponent::stateChanged()
{
    updateSliceWaveform();
}

void MainComponent::transportButtonPressed(TransportButtonComponent::TransportEvent event)
{
    juce::String cmd;
    if (event == TransportButtonComponent::TransportEvent::PlayChain)
        cmd = "PlayChain";
    else if (event == TransportButtonComponent::TransportEvent::PlaySlice)
        cmd = "PlaySlice";
    else if (event == TransportButtonComponent::TransportEvent::Stop)
        cmd = "Stop";
    DBG("Transport button pressed: " << cmd);
}

void MainComponent::updateSliceWaveform()
{
    juce::AudioBuffer<float> audioData;
    double sampleRate = 0.0;

    if (loadSelectedSliceAudio(audioData, sampleRate))
        sliceWaveformComponent.setAudioData(audioData, sampleRate);
    else
        sliceWaveformComponent.clearAudioData();
}

bool MainComponent::loadSelectedSliceAudio(juce::AudioBuffer<float>& destination, double& sampleRate) const
{
    const auto sliceTree = stateHandler.getSelectedSliceTree();
    if (! sliceTree.isValid())
        return false;

    const auto numChannels = static_cast<int>(sliceTree.getProperty(stateHandler.sliceChannelsId, 0));
    sampleRate = static_cast<double>(sliceTree.getProperty(stateHandler.sliceSamplerateId, 0.0));
    const auto numSamples = static_cast<juce::int64>(sliceTree.getProperty(stateHandler.sliceNumSamplesId, static_cast<juce::int64>(0)));
    const auto* audioDataValue = sliceTree.getPropertyPointer(stateHandler.sliceAudioDataId);

    if (numChannels <= 0
        || sampleRate <= 0.0
        || numSamples <= 0
        || numSamples > static_cast<juce::int64>(std::numeric_limits<int>::max())
        || audioDataValue == nullptr)
    {
        return false;
    }

    const auto* audioDataBlock = audioDataValue->getBinaryData();
    if (audioDataBlock == nullptr)
        return false;

    const auto samplesPerChannel = static_cast<size_t>(numSamples);
    const auto expectedBytes = static_cast<size_t>(numChannels) * samplesPerChannel * sizeof(float);
    if (audioDataBlock->getSize() < expectedBytes)
        return false;

    const auto numSamplesAsInt = static_cast<int>(numSamples);
    const auto* samples = static_cast<const float*>(audioDataBlock->getData());

    destination.setSize(numChannels, numSamplesAsInt, false, false, false);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        destination.copyFrom(channel,
                             0,
                             samples + static_cast<size_t>(channel) * samplesPerChannel,
                             numSamplesAsInt);
    }

    return true;
}
