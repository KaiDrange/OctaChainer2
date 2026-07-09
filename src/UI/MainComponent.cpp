#include "MainComponent.h"

MainComponent::MainComponent()
    : sampleListComponent(sampleListHeightPercentage, sampleListWidthPercentage),
      settingsPanelComponent(settingsPanelHeightPercentage, settingsPanelWidthPercentage, "Settings"),
      sliceWaveformComponent(sliceWaveformHeightPercentage, sliceWaveformHeightPercentage, "Slice waveform"),
      chainWaveformComponent(chainWaveformWidthPercentage, chainWaveformWidthPercentage, "Chain waveform")
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
        juce::Colour(StyleSheet::backgroundColour), 0, 0, juce::Colour(StyleSheet::backgroundGradientColour),
        bounds.getWidth(), bounds.getHeight(), false
    };
    g.setGradientFill(backgroundGradient);
    g.fillRect(bounds);
}

void MainComponent::resized()
{
    auto topArea = getLocalBounds().reduced(StyleSheet::panelMargins);
    sampleListComponent.setBounds(topArea.removeFromLeft(topArea.getWidth() * sampleListComponent.widthPercentage / 100).withHeight(topArea.getHeight() * sampleListComponent.heightPercentage / 100));
    settingsPanelComponent.setBounds(topArea.removeFromTop(topArea.getHeight() * settingsPanelComponent.heightPercentage / 100).withWidth(topArea.getWidth()));

    auto bottomArea = getLocalBounds().reduced(StyleSheet::panelMargins);
    chainWaveformComponent.setBounds(bottomArea.removeFromBottom(bottomArea.getHeight() * chainWaveformComponent.heightPercentage / 100).withWidth(bottomArea.getWidth()));
    sliceWaveformComponent.setBounds(bottomArea.removeFromBottom(bottomArea.getHeight() * sliceWaveformComponent.heightPercentage / 100).withWidth(bottomArea.getWidth()));

}
