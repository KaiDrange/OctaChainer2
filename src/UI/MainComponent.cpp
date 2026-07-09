#include "MainComponent.h"

MainComponent::MainComponent()
    : sampleListComponent(sampleListHeightPercentage, sampleListWidthPercentage),
      settingsPanelComponent(settingsPanelHeightPercentage, settingsPanelWidthPercentage, "Settings"),
      sliceWaveformComponent(sliceWaveformHeightPercentage, sliceWaveformHeightPercentage, "Slice waveform"),
      chainWaveformComponent(chainWaveformWidthPercentage, chainWaveformWidthPercentage, "Chain waveforms")
{
    setLookAndFeel(&style);
    addAndMakeVisible(sampleListComponent);
    addAndMakeVisible(settingsPanelComponent);
    addAndMakeVisible(sliceWaveformComponent);
    addAndMakeVisible(chainWaveformComponent);
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

    const auto topHeight = contentArea.getHeight() * sampleListHeightPercentage / 100;
    const auto sliceHeight = contentArea.getHeight() * sliceWaveformComponent.heightPercentage / 100;
    auto topBand = contentArea.removeFromTop(topHeight);
    auto bottomBand = contentArea;

    const auto sampleListWidth = topBand.getWidth() * sampleListWidthPercentage / 100;
    sampleListComponent.setBounds(topBand.removeFromLeft(sampleListWidth));
    settingsPanelComponent.setBounds(topBand);

    sliceWaveformComponent.setBounds(bottomBand.removeFromTop(sliceHeight));
    chainWaveformComponent.setBounds(bottomBand); // Remaining space. If the layout changes, use bottomBand.removeFromTop(chainHeight) instead.
}
