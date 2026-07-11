#include "MainComponent.h"

MainComponent::MainComponent()
    : sampleListComponent(PanelComponent::Dimension::percentage(sampleListHeightPercentage, sampleListMinHeight),
                          PanelComponent::Dimension::percentage(sampleListWidthPercentage)),
      settingsPanelComponent(PanelComponent::Dimension::fixed(settingsPanelFixedHeight),
                             PanelComponent::Dimension::fixed(settingsPanelFixedWidth)),
      audioPanelComponent(PanelComponent::Dimension::fixed(audioSectionFixedHeight),
                       PanelComponent::Dimension::fixed(settingsPanelFixedWidth)),
      sliceWaveformComponent(PanelComponent::Dimension::percentage(sliceWaveformHeightPercentage),
                             PanelComponent::Dimension::percentage(sliceWaveformHeightPercentage),
                             "Slice waveform"),
      chainWaveformComponent(PanelComponent::Dimension::percentage(chainWaveformWidthPercentage),
                             PanelComponent::Dimension::percentage(chainWaveformWidthPercentage),
                             "Chain waveforms")
{
    setLookAndFeel(&style);
    addAndMakeVisible(sampleListComponent);
    addAndMakeVisible(settingsPanelComponent);
    addAndMakeVisible(sliceWaveformComponent);
    addAndMakeVisible(chainWaveformComponent);
    addAndMakeVisible(audioPanelComponent);
}


MainComponent::~MainComponent()
{
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
