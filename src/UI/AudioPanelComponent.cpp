#include "AudioPanelComponent.h"

#include <memory>

namespace
{
    std::unique_ptr<juce::DrawablePath> createPlayIcon(const juce::Colour colour)
    {
        auto drawable = std::make_unique<juce::DrawablePath>();
        juce::Path path;
        path.addTriangle(28.0f, 20.0f, 28.0f, 80.0f, 74.0f, 50.0f);
        drawable->setPath(path);
        drawable->setFill(juce::FillType(colour));
        return drawable;
    }

    std::unique_ptr<juce::DrawablePath> createStopIcon(const juce::Colour colour)
    {
        auto drawable = std::make_unique<juce::DrawablePath>();
        juce::Path path;
        path.addRectangle(30.0f, 30.0f, 40.0f, 40.0f);
        drawable->setPath(path);
        drawable->setFill(juce::FillType(colour));
        return drawable;
    }
}

AudioPanelComponent::AudioPanelComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                                         const juce::String& title)
    : PanelComponent(height, width, title)
{
    addAndMakeVisible(btnPlaySlice);
    addAndMakeVisible(btnPlayChain);
    addAndMakeVisible(sliceLabel);
    addAndMakeVisible(chainLabel);
    addAndMakeVisible(masterVolumeLabel);
    addAndMakeVisible(masterVolumeSlider);

    auto configurePlayButton = [](juce::DrawableButton& button)
    {
        button.setClickingTogglesState(true);
        button.setToggleState(false, juce::dontSendNotification);
        button.setButtonStyle(juce::DrawableButton::ImageOnButtonBackground);
        button.setColour(juce::TextButton::buttonColourId, juce::Colour(StyleSheet::controlBackgroundColour));
        button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(StyleSheet::controlBackgroundColour));
        button.setImages(createPlayIcon(juce::Colour(0xFF2EAF58)).get(),
                         nullptr,
                         nullptr,
                         nullptr,
                         createStopIcon(juce::Colour(0xFFD94B4B)).get(),
                         nullptr,
                         nullptr,
                         nullptr);
        button.setEdgeIndent(8);
    };

    configurePlayButton(btnPlaySlice);
    configurePlayButton(btnPlayChain);

    sliceLabel.setText("Slice", juce::dontSendNotification);
    sliceLabel.setJustificationType(juce::Justification::centred);
    sliceLabel.setInterceptsMouseClicks(false, false);

    chainLabel.setText("Chain", juce::dontSendNotification);
    chainLabel.setJustificationType(juce::Justification::centred);
    chainLabel.setInterceptsMouseClicks(false, false);

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

    auto area = innerBounds.reduced(StyleSheet::controlGap);

    auto columns = area;
    const auto columnWidth = columns.getWidth() / 3;
    auto sliceColumn = columns.removeFromLeft(columnWidth);
    auto chainColumn = columns.removeFromLeft(columnWidth);
    auto volumeColumn = columns;

    constexpr int labelHeight = 18;
    auto layoutButtonColumn = [labelHeight](juce::Rectangle<int> column, juce::Label& label, juce::DrawableButton& button)
    {
        auto labelArea = column.removeFromTop(labelHeight);
        label.setBounds(labelArea);
        column.removeFromTop(StyleSheet::controlGap);
        const auto controlSize = juce::jmax(0, juce::jmin(column.getWidth(), column.getHeight()));
        button.setBounds(column.withSizeKeepingCentre(controlSize, controlSize));
    };

    layoutButtonColumn(sliceColumn, sliceLabel, btnPlaySlice);
    layoutButtonColumn(chainColumn, chainLabel, btnPlayChain);

    auto volumeLabelArea = volumeColumn.removeFromTop(labelHeight);
    masterVolumeLabel.setBounds(volumeLabelArea);
    volumeColumn.removeFromTop(StyleSheet::controlGap);
    const auto sliderSize = juce::jmax(0, juce::jmin(volumeColumn.getWidth(), volumeColumn.getHeight()));
    masterVolumeSlider.setBounds(volumeColumn.withSizeKeepingCentre(sliderSize, sliderSize));
}
