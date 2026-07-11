#include "SettingsPanelComponent.h"

#include <initializer_list>


SettingsPanelComponent::SettingsPanelComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                                               const juce::String& title)
    : PanelComponent(height, width, title)
{
    addAndMakeVisible(bitrateSection);
    addAndMakeVisible(channelSection);
    addAndMakeVisible(sampleRateSection);
    addAndMakeVisible(otAttributesSection);
    addAndMakeVisible(chainExportSection);
    addAndMakeVisible(megabreakExportSection);

    otAttributesSection.backgroundColour = otAttributesSection.backgroundColour.darker(0.02f);
    chainExportSection.backgroundColour =
        chainExportSection.backgroundColour.withHue(chainExportSection.backgroundColour.getHue() + 0.03f);
    megabreakExportSection.backgroundColour =
        megabreakExportSection.backgroundColour.withHue(megabreakExportSection.backgroundColour.getHue() - 0.05f);

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

    chainExportSection.addAndMakeVisible(exportOtFile);
    chainExportSection.addAndMakeVisible(exportEvenGrid);
    chainExportSection.addAndMakeVisible(exportEmbedMarkers);
    chainExportSection.addAndMakeVisible(createButton);

    megabreakExportSection.addAndMakeVisible(megabreakFileCountBox);
    megabreakExportSection.addAndMakeVisible(createMegabreakButton);

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
    configureCombo(megabreakFileCountBox, { "File count: 16" }, 1);
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
    const auto topSectionWidth = (topSections.getWidth() - StyleSheet::sectionGap * 2) / 3;

    bitrateSection.setBounds(topSections.removeFromLeft(topSectionWidth));
    topSections.removeFromLeft(StyleSheet::sectionGap);
    channelSection.setBounds(topSections.removeFromLeft(topSectionWidth));
    topSections.removeFromLeft(StyleSheet::sectionGap);
    sampleRateSection.setBounds(topSections);

    auto otArea = innerBounds;
    otArea.removeFromTop(topSectionHeight + StyleSheet::sectionGap);
    otArea = otArea.removeFromTop(otSectionHeight);
    otAttributesSection.setBounds(otArea);

    auto exportArea = innerBounds;
    exportArea.removeFromTop(topSectionHeight + otSectionHeight + StyleSheet::sectionGap * 2);
    auto chainExportArea = exportArea.removeFromTop(chainExportSectionHeight);
    chainExportSection.setBounds(chainExportArea);
    exportArea.removeFromTop(StyleSheet::sectionGap);
    megabreakExportSection.setBounds(exportArea.removeFromTop(megabreakExportSectionHeight));

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

    timestretchBox.setBounds(controls.removeFromTop(StyleSheet::comboboxHeight));
    controls.removeFromTop(StyleSheet::controlGap);
    loopBox.setBounds(controls.removeFromTop(StyleSheet::comboboxHeight));
    controls.removeFromTop(StyleSheet::controlGap);
    trigQuantBox.setBounds(controls.removeFromTop(StyleSheet::comboboxHeight));
    controls.removeFromTop(StyleSheet::controlGap);
    normalizationBox.setBounds(controls.removeFromTop(StyleSheet::comboboxHeight));
    controls.removeFromTop(StyleSheet::controlGap);

    const auto fadeRow = controls;
    const auto fadeWidth = (fadeRow.getWidth() - StyleSheet::controlGap) / 2;
    fadeinBox.setBounds(fadeRow.getX(), fadeRow.getY(), fadeWidth, StyleSheet::comboboxHeight);
    fadeoutBox.setBounds(fadeRow.getX() + fadeWidth + StyleSheet::controlGap, fadeRow.getY(), fadeWidth, StyleSheet::comboboxHeight);

    controls.removeFromTop(StyleSheet::comboboxHeight + StyleSheet::controlGap);

    const auto inputRow = controls;
    const auto inputWidth = (inputRow.getWidth() - StyleSheet::controlGap) / 2;

    gainInput.setBounds(inputRow.getX(), inputRow.getY(), inputWidth, StyleSheet::inputHeight);
    bpmInput.setBounds(inputRow.getX() + inputWidth + StyleSheet::controlGap, inputRow.getY(), inputWidth, StyleSheet::inputHeight);

    auto chainExportControls = chainExportSection.getContentBounds();

    const auto exportToggleRow = chainExportControls.removeFromTop(StyleSheet::comboboxHeight);
    const auto exportToggleWidth = (exportToggleRow.getWidth() - StyleSheet::controlGap) / 2;
    exportOtFile.setBounds(exportToggleRow.getX(), exportToggleRow.getY(), exportToggleWidth, StyleSheet::comboboxHeight);
    exportEvenGrid.setBounds(exportToggleRow.getX() + exportToggleWidth + StyleSheet::controlGap,
                             exportToggleRow.getY(), exportToggleWidth, StyleSheet::comboboxHeight);

    chainExportControls.removeFromTop(StyleSheet::controlGap);
    const auto chainActionRow = chainExportControls.removeFromTop(StyleSheet::comboboxHeight);
    const auto chainActionWidth = (chainActionRow.getWidth() - StyleSheet::controlGap) / 2;
    exportEmbedMarkers.setBounds(chainActionRow.getX(), chainActionRow.getY(), chainActionWidth, StyleSheet::comboboxHeight);
    createButton.setBounds(chainActionRow.getX() + chainActionWidth + StyleSheet::controlGap,
                           chainActionRow.getY(), chainActionWidth, StyleSheet::comboboxHeight);

    auto megabreakExportControls = megabreakExportSection.getContentBounds();
    const auto megabreakRow = megabreakExportControls.removeFromTop(StyleSheet::comboboxHeight);
    const auto megabreakWidth = (megabreakRow.getWidth() - StyleSheet::controlGap) / 2;
    megabreakFileCountBox.setBounds(megabreakRow.getX(), megabreakRow.getY(), megabreakWidth, StyleSheet::comboboxHeight);
    createMegabreakButton.setBounds(megabreakRow.getX() + megabreakWidth + StyleSheet::controlGap,
                                    megabreakRow.getY(), megabreakWidth, StyleSheet::comboboxHeight);
}
