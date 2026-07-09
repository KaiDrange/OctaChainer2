#include "SettingsPanelComponent.h"

#include <initializer_list>

namespace
{
    constexpr int sectionGap = 10;
    constexpr int rowHeight = 22;
    constexpr int rowGap = 2;
    constexpr int maxSectionHeight = 108;
}

SettingsPanelComponent::SettingsPanelComponent(const int heightPercentage, const int widthPercentage,
                                               const juce::String& title)
    : PanelComponent(heightPercentage, widthPercentage, title)
{
    addAndMakeVisible(bitrateSection);
    addAndMakeVisible(channelSection);
    addAndMakeVisible(sampleRateSection);

    bitrateSection.addAndMakeVisible(bitrate16Bit);
    bitrateSection.addAndMakeVisible(bitrate24Bit);
    channelSection.addAndMakeVisible(channelMono);
    channelSection.addAndMakeVisible(channelStereo);
    sampleRateSection.addAndMakeVisible(sampleRate48k);
    sampleRateSection.addAndMakeVisible(sampleRate44k1);
    sampleRateSection.addAndMakeVisible(sampleRate32k);
    sampleRateSection.addAndMakeVisible(sampleRate22k05);

    configureRadioButton(bitrate16Bit, bitrateGroupId, true);
    configureRadioButton(bitrate24Bit, bitrateGroupId, false);

    configureRadioButton(channelMono, channelGroupId, false);
    configureRadioButton(channelStereo, channelGroupId, true);

    configureRadioButton(sampleRate48k, sampleRateGroupId, false);
    configureRadioButton(sampleRate44k1, sampleRateGroupId, true);
    configureRadioButton(sampleRate32k, sampleRateGroupId, false);
    configureRadioButton(sampleRate22k05, sampleRateGroupId, false);
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

    auto sections = innerBounds.withHeight(juce::jmin(innerBounds.getHeight(), maxSectionHeight));
    const auto sectionWidth = (sections.getWidth() - sectionGap * 2) / 3;

    bitrateSection.setBounds(sections.removeFromLeft(sectionWidth));
    sections.removeFromLeft(sectionGap);
    channelSection.setBounds(sections.removeFromLeft(sectionWidth));
    sections.removeFromLeft(sectionGap);
    sampleRateSection.setBounds(sections);

    auto placeButtons = [](const SectionComponent& section, const std::initializer_list<juce::ToggleButton*> buttons)
    {
        auto area = section.getContentBounds();

        for (auto* button : buttons)
        {
            button->setBounds(area.removeFromTop(rowHeight));
            area.removeFromTop(rowGap);
        }
    };

    placeButtons(bitrateSection, { &bitrate16Bit, &bitrate24Bit });
    placeButtons(channelSection, { &channelMono, &channelStereo });
    placeButtons(sampleRateSection, { &sampleRate48k, &sampleRate44k1, &sampleRate32k, &sampleRate22k05 });
}
