#include "SettingsPanelComponent.h"

#include <initializer_list>

namespace
{
    constexpr int topSectionHeight = 80;
    constexpr int sectionGap = 10;
    constexpr int controlGap = 6;
    constexpr int comboHeight = 27;
    constexpr int inputHeight = 30;
}

SettingsPanelComponent::SettingsPanelComponent(const int heightPercentage, const int widthPercentage,
                                               const juce::String& title)
    : PanelComponent(heightPercentage, widthPercentage, title)
{
    addAndMakeVisible(bitrateSection);
    addAndMakeVisible(channelSection);
    addAndMakeVisible(sampleRateSection);
    addAndMakeVisible(otAttributesSection);

    bitrateSection.addAndMakeVisible(bitrate16Bit);
    bitrateSection.addAndMakeVisible(bitrate24Bit);
    channelSection.addAndMakeVisible(channelMono);
    channelSection.addAndMakeVisible(channelStereo);
    sampleRateSection.addAndMakeVisible(sampleRate48k);
    sampleRateSection.addAndMakeVisible(sampleRate44k1);

    otAttributesSection.addAndMakeVisible(timestretchBox);
    otAttributesSection.addAndMakeVisible(loopBox);
    otAttributesSection.addAndMakeVisible(trigQuantBox);
    otAttributesSection.addAndMakeVisible(normalizationBox);
    otAttributesSection.addAndMakeVisible(fadeinBox);
    otAttributesSection.addAndMakeVisible(fadeoutBox);
    otAttributesSection.addAndMakeVisible(gainInput);
    otAttributesSection.addAndMakeVisible(bpmInput);
    otAttributesSection.addAndMakeVisible(normalizationBox);
    otAttributesSection.addAndMakeVisible(fadeinBox);
    otAttributesSection.addAndMakeVisible(fadeoutBox);

    auto configureCombo = [](juce::ComboBox& box, std::initializer_list<juce::String> items, int selectedId)
    {
        int itemId = 1;
        for (const auto& item : items)
            box.addItem(item, itemId++);

        box.setSelectedId(selectedId, juce::dontSendNotification);
    };

    configureRadioButton(bitrate16Bit, bitrateGroupId, true);
    configureRadioButton(bitrate24Bit, bitrateGroupId, false);

    configureRadioButton(channelMono, channelGroupId, false);
    configureRadioButton(channelStereo, channelGroupId, true);

    configureRadioButton(sampleRate48k, sampleRateGroupId, false);
    configureRadioButton(sampleRate44k1, sampleRateGroupId, true);

    configureCombo(timestretchBox, { "Timestretch off", "Timestretch on" }, 1);
    configureCombo(loopBox, { "Loop off", "Loop on" }, 1);
    configureCombo(trigQuantBox, { "Trig quant direct", "Trig quant quantized" }, 1);
    configureCombo(normalizationBox, { "No normalization", "Normalize on" }, 1);
    configureCombo(fadeinBox, { "No fade-in", "Fade in" }, 1);
    configureCombo(fadeoutBox, { "No fade-out", "Fade out" }, 1);

    gainInput.setValue(0);
    bpmInput.setValue(125);
}

void SettingsPanelComponent::configureRadioButton(juce::ToggleButton& button, const int groupId, const bool selected)
{
    button.setClickingTogglesState(true);
    button.setRadioGroupId(groupId, juce::dontSendNotification);
    button.setToggleState(selected, juce::dontSendNotification);
}

void SettingsPanelComponent::resized()
{
    PanelComponent::resized();

    auto topSections = innerBounds.withHeight(juce::jmin(innerBounds.getHeight(), topSectionHeight));
    const auto topSectionWidth = (topSections.getWidth() - sectionGap * 2) / 3;

    bitrateSection.setBounds(topSections.removeFromLeft(topSectionWidth));
    topSections.removeFromLeft(sectionGap);
    channelSection.setBounds(topSections.removeFromLeft(topSectionWidth));
    topSections.removeFromLeft(sectionGap);
    sampleRateSection.setBounds(topSections);

    auto otArea = innerBounds;
    otArea.removeFromTop(topSectionHeight + sectionGap);
    otAttributesSection.setBounds(otArea);

    auto placeButtons = [](const SectionComponent& section, const std::initializer_list<juce::ToggleButton*> buttons)
    {
        auto area = section.getContentBounds();
        for (auto* button : buttons)
        {
            button->setBounds(area.removeFromTop(22));
            area.removeFromTop(2);
        }
    };

    placeButtons(bitrateSection, { &bitrate16Bit, &bitrate24Bit });
    placeButtons(channelSection, { &channelMono, &channelStereo });
    placeButtons(sampleRateSection, { &sampleRate48k, &sampleRate44k1 });

    const auto otContent = otAttributesSection.getContentBounds();
    auto controls = otContent;

    timestretchBox.setBounds(controls.removeFromTop(comboHeight));
    controls.removeFromTop(controlGap);
    loopBox.setBounds(controls.removeFromTop(comboHeight));
    controls.removeFromTop(controlGap);
    trigQuantBox.setBounds(controls.removeFromTop(comboHeight));
    controls.removeFromTop(controlGap);
    normalizationBox.setBounds(controls.removeFromTop(comboHeight));
    controls.removeFromTop(controlGap);

    const auto fadeRow = controls;
    const auto fadeWidth = (fadeRow.getWidth() - controlGap) / 2;
    fadeinBox.setBounds(fadeRow.getX(), fadeRow.getY(), fadeWidth, comboHeight);
    fadeoutBox.setBounds(fadeRow.getX() + fadeWidth + controlGap, fadeRow.getY(), fadeWidth, comboHeight);

    controls.removeFromTop(comboHeight + controlGap);

    const auto inputRow = controls;
    const auto inputWidth = (inputRow.getWidth() - controlGap) / 2;

    gainInput.setBounds(inputRow.getX(), inputRow.getY(), inputWidth, inputHeight);
    bpmInput.setBounds(inputRow.getX() + inputWidth + controlGap, inputRow.getY(), inputWidth, inputHeight);
}
