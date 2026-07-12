#include "AudioPanelComponent.h"

AudioPanelComponent::AudioPanelComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                                         const juce::String& title)
    : PanelComponent(height, width, title)
{
    addAndMakeVisible(btnPlaySlice);
    addAndMakeVisible(btnPlayChain);
    addAndMakeVisible(masterVolumeLabel);
    addAndMakeVisible(masterVolumeSlider);

    masterVolumeLabel.setText("Master Volume", juce::dontSendNotification);
    masterVolumeLabel.setJustificationType(juce::Justification::centred);
    masterVolumeLabel.setInterceptsMouseClicks(false, false);

    masterVolumeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    masterVolumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    masterVolumeSlider.setRange(0.0, 1.0, 0.001);
    masterVolumeSlider.setValue(0.75, juce::dontSendNotification);
}

void AudioPanelComponent::resized()
{
    PanelComponent::resized();

    const auto height = innerBounds.getHeight();
    setVisible(height > 40);

    const auto hideLabels = height < 80;
    btnPlaySlice.showLabel = !hideLabels;
    btnPlayChain.showLabel = !hideLabels;
    masterVolumeLabel.setVisible(!hideLabels);

    const auto area = innerBounds.reduced(StyleSheet::controlGap);
    constexpr int labelHeight = static_cast<int>(StyleSheet::fontDefaultSize);

    auto columns = area;
    const auto columnWidth = columns.getWidth() / 3;
    const auto sliceColumn = columns.removeFromLeft(columnWidth);
    const auto chainColumn = columns.removeFromLeft(columnWidth);
    auto volumeColumn = columns;
    btnPlaySlice.setBounds(sliceColumn);
    btnPlayChain.setBounds(chainColumn);

    if (!hideLabels)
    {
        const auto volumeLabelArea = volumeColumn.removeFromTop(labelHeight);
        masterVolumeLabel.setBounds(volumeLabelArea);
        volumeColumn.removeFromTop(StyleSheet::controlGap);
    }
    const auto sliderSize = juce::jmax(0, juce::jmin(volumeColumn.getWidth(), volumeColumn.getHeight()));
    masterVolumeSlider.setBounds(volumeColumn.withSizeKeepingCentre(sliderSize, sliderSize));
}
