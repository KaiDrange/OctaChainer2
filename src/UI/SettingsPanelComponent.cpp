#include "SettingsPanelComponent.h"

#include <algorithm>
#include <initializer_list>


SettingsPanelComponent::SettingsPanelComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                                               StateHandler& stateHandlerToUse,
                                               const juce::String& title)
    : PanelComponent(height, width, title),
      stateHandler(stateHandlerToUse)
{
    stateHandler.addListener(this);

    addAndMakeVisible(bitDepthSection);
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

    bitDepthSection.addAndMakeVisible(bitDepth16Bit);
    bitDepthSection.addAndMakeVisible(bitDepth24Bit);
    channelSection.addAndMakeVisible(channelMono);
    channelSection.addAndMakeVisible(channelStereo);
    sampleRateSection.addAndMakeVisible(sampleRate44k);
    sampleRateSection.addAndMakeVisible(sampleRate48k);

    otAttributesSection.addAndMakeVisible(timestretchBox);
    otAttributesSection.addAndMakeVisible(loopBox);
    otAttributesSection.addAndMakeVisible(trigQuantBox);
    otAttributesSection.addAndMakeVisible(gainInput);
    otAttributesSection.addAndMakeVisible(bpmInput);

    chainExportSection.addAndMakeVisible(normalizationBox);
    chainExportSection.addAndMakeVisible(fadeinBox);
    chainExportSection.addAndMakeVisible(fadeoutBox);
    chainExportSection.addAndMakeVisible(exportOtFile);
    chainExportSection.addAndMakeVisible(exportEvenGrid);
    chainExportSection.addAndMakeVisible(exportEmbedMarkers);
    chainExportSection.addAndMakeVisible(createButton);

    megabreakExportSection.addAndMakeVisible(megabreakFileCountBox);
    megabreakExportSection.addAndMakeVisible(createMegabreakButton);

    configureRadioButtons(stateHandler, stateHandler.bitDepthId, bitDepthGroupId, { &bitDepth16Bit, &bitDepth24Bit });
    configureRadioButtons(stateHandler, stateHandler.channelsId, channelGroupId, { &channelMono, &channelStereo });
    configureRadioButtons(stateHandler, stateHandler.samplerateId, sampleRateGroupId, { &sampleRate44k, &sampleRate48k });

    timestretchBox.onChange = [this]{ stateHandler.setStateValueFromItemId(stateHandler.timestretchId, timestretchBox.getSelectedId()); };
    loopBox.onChange = [this]{ stateHandler.setStateValueFromItemId(stateHandler.loopModeId, loopBox.getSelectedId()); };
    trigQuantBox.onChange = [this]{ stateHandler.setStateValueFromItemId(stateHandler.triqQuantId, trigQuantBox.getSelectedId()); };
    normalizationBox.onChange = [this]{ stateHandler.setStateValueFromItemId(stateHandler.normalizationId, normalizationBox.getSelectedId()); };
    fadeinBox.onChange = [this]{ stateHandler.setStateValueFromItemId(stateHandler.fadeinId, fadeinBox.getSelectedId()); };
    fadeoutBox.onChange = [this]{ stateHandler.setStateValueFromItemId(stateHandler.fadeoutId, fadeoutBox.getSelectedId()); };
    megabreakFileCountBox.onChange = [this]{ stateHandler.setStateValueFromItemId(stateHandler.megabreakFileCountId, megabreakFileCountBox.getSelectedId()); };

    gainInput.addListener(this);
    bpmInput.addListener(this);

    SettingsPanelComponent::stateChanged();
}

SettingsPanelComponent::~SettingsPanelComponent()
{
    gainInput.removeListener(this);
    bpmInput.removeListener(this);
    stateHandler.removeListener(this);
}

void SettingsPanelComponent::configureRadioButton(juce::ToggleButton& button, const int groupId)
{
    button.setClickingTogglesState(true);
    button.setRadioGroupId(groupId, juce::dontSendNotification);
}

void SettingsPanelComponent::configureRadioButtons(StateHandler& stateHandler, const juce::Identifier& identifier,
                                                   const int groupId, const std::initializer_list<juce::ToggleButton*> buttons)
{
    auto options = stateHandler.getOptions(identifier);
    std::sort(options.begin(), options.end(),
              [](const StateHandler::Option& lhs, const StateHandler::Option& rhs)
              {
                  return lhs.itemId < rhs.itemId;
              });
    auto optionIt = options.begin();

    for (auto* button : buttons)
    {
        if (button == nullptr)
            continue;

        configureRadioButton(*button, groupId);

        if (optionIt == options.end())
        {
            button->onClick = {};
            continue;
        }

        const auto itemId = optionIt->itemId;
        button->setButtonText(optionIt->name);
        button->onClick = [button, &stateHandler, identifier, itemId]
        {
            if (button->getToggleState())
                stateHandler.setStateValueFromItemId(identifier, itemId);
        };

        ++optionIt;
    }
}

void SettingsPanelComponent::resized()
{
    PanelComponent::resized();

    layoutTopSections();
    layoutOtAttributesSection();
    layoutChainExportSection();
    layoutMegabreakExportSection();
}

void SettingsPanelComponent::layoutTopSections()
{
    auto topSections = innerBounds.withHeight(juce::jmin(innerBounds.getHeight(), topSectionHeight));
    const auto topSectionWidth = (topSections.getWidth() - StyleSheet::sectionGap * 2) / 3;

    bitDepthSection.setBounds(topSections.removeFromLeft(topSectionWidth));
    topSections.removeFromLeft(StyleSheet::sectionGap);
    channelSection.setBounds(topSections.removeFromLeft(topSectionWidth));
    topSections.removeFromLeft(StyleSheet::sectionGap);
    sampleRateSection.setBounds(topSections);

    auto placeButtons = [](const SectionComponent& section, const std::initializer_list<juce::ToggleButton*> buttons)
    {
        auto area = section.getContentBounds();
        for (auto* button : buttons)
        {
            button->setBounds(area.removeFromTop(22));
            area.removeFromTop(2);
        }
    };

    placeButtons(bitDepthSection, { &bitDepth16Bit, &bitDepth24Bit });
    placeButtons(channelSection, { &channelMono, &channelStereo });
    placeButtons(sampleRateSection, { &sampleRate44k, &sampleRate48k });
}

void SettingsPanelComponent::layoutOtAttributesSection()
{
    auto otArea = innerBounds;
    otArea.removeFromTop(topSectionHeight + StyleSheet::sectionGap);
    otArea = otArea.removeFromTop(otSectionHeight);
    otAttributesSection.setBounds(otArea);

    const auto otContent = otAttributesSection.getContentBounds();
    auto controls = otContent;

    timestretchBox.setBounds(controls.removeFromTop(StyleSheet::comboboxHeight));
    controls.removeFromTop(StyleSheet::controlGap);
    loopBox.setBounds(controls.removeFromTop(StyleSheet::comboboxHeight));
    controls.removeFromTop(StyleSheet::controlGap);
    trigQuantBox.setBounds(controls.removeFromTop(StyleSheet::comboboxHeight));
    controls.removeFromTop(StyleSheet::controlGap);

    const auto inputRow = controls;
    const auto inputWidth = (inputRow.getWidth() - StyleSheet::controlGap) / 2;

    gainInput.setBounds(inputRow.getX(), inputRow.getY(), inputWidth, StyleSheet::inputHeight);
    bpmInput.setBounds(inputRow.getX() + inputWidth + StyleSheet::controlGap, inputRow.getY(), inputWidth, StyleSheet::inputHeight);
}

void SettingsPanelComponent::layoutChainExportSection()
{
    auto exportArea = innerBounds;
    exportArea.removeFromTop(topSectionHeight + otSectionHeight + StyleSheet::sectionGap * 2);
    const auto chainExportArea = exportArea.removeFromTop(chainExportSectionHeight);
    chainExportSection.setBounds(chainExportArea);

    auto chainExportControls = chainExportSection.getContentBounds();
    normalizationBox.setBounds(chainExportControls.removeFromTop(StyleSheet::comboboxHeight));
    chainExportControls.removeFromTop(StyleSheet::controlGap);

    const auto fadeRow = chainExportControls;
    const auto fadeWidth = (fadeRow.getWidth() - StyleSheet::controlGap) / 2;
    fadeinBox.setBounds(fadeRow.getX(), fadeRow.getY(), fadeWidth, StyleSheet::comboboxHeight);
    fadeoutBox.setBounds(fadeRow.getX() + fadeWidth + StyleSheet::controlGap, fadeRow.getY(), fadeWidth, StyleSheet::comboboxHeight);

    chainExportControls.removeFromTop(StyleSheet::comboboxHeight + StyleSheet::controlGap);

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
}

void SettingsPanelComponent::layoutMegabreakExportSection()
{
    auto exportArea = innerBounds;
    exportArea.removeFromTop(topSectionHeight + otSectionHeight + StyleSheet::sectionGap * 2);
    exportArea.removeFromTop(chainExportSectionHeight + StyleSheet::sectionGap);
    megabreakExportSection.setBounds(exportArea.removeFromTop(megabreakExportSectionHeight));

    auto megabreakExportControls = megabreakExportSection.getContentBounds();
    const auto megabreakRow = megabreakExportControls.removeFromTop(StyleSheet::comboboxHeight);
    const auto megabreakWidth = (megabreakRow.getWidth() - StyleSheet::controlGap) / 2;
    megabreakFileCountBox.setBounds(megabreakRow.getX(), megabreakRow.getY(), megabreakWidth, StyleSheet::comboboxHeight);
    createMegabreakButton.setBounds(megabreakRow.getX() + megabreakWidth + StyleSheet::controlGap,
                                    megabreakRow.getY(), megabreakWidth, StyleSheet::comboboxHeight);
}

void SettingsPanelComponent::stateChanged()
{
    stateHandler.refreshRadioButtons(stateHandler.bitDepthId, { &bitDepth16Bit, &bitDepth24Bit });
    stateHandler.refreshRadioButtons(stateHandler.channelsId, { &channelMono, &channelStereo });
    stateHandler.refreshRadioButtons(stateHandler.samplerateId, { &sampleRate44k, &sampleRate48k });

    stateHandler.refreshComboBox(stateHandler.timestretchId, timestretchBox);
    stateHandler.refreshComboBox(stateHandler.loopModeId, loopBox);
    stateHandler.refreshComboBox(stateHandler.triqQuantId, trigQuantBox);
    stateHandler.refreshComboBox(stateHandler.normalizationId, normalizationBox);
    stateHandler.refreshComboBox(stateHandler.fadeinId, fadeinBox);
    stateHandler.refreshComboBox(stateHandler.fadeoutId, fadeoutBox);
    stateHandler.refreshComboBox(stateHandler.megabreakFileCountId, megabreakFileCountBox);

    gainInput.setValue(juce::var(stateHandler.getStateValue<double>(stateHandler.gainId, StateHandler::gainValue.defaultValue)));
    bpmInput.setValue(juce::var(stateHandler.getStateValue<double>(stateHandler.bpmId, StateHandler::bpmValue.defaultValue)));
    exportOtFile.setToggleState(juce::var(stateHandler.getStateValue<bool>(stateHandler.otFileId, StateHandler::otFileDefault)), juce::NotificationType::dontSendNotification);
    exportEvenGrid.setToggleState(juce::var(stateHandler.getStateValue<bool>(stateHandler.evenGridId, StateHandler::evenGridDefault)), juce::NotificationType::dontSendNotification);
    exportEmbedMarkers.setToggleState(juce::var(stateHandler.getStateValue<bool>(stateHandler.embedMarkersId, StateHandler::embedMarkersDefault)), juce::NotificationType::dontSendNotification);
}

void SettingsPanelComponent::numberInputChanged(NumberInputComponent* numberInput)
{
    const auto value = numberInput->getValue();
    if (value.isVoid())
        return;

    if (numberInput == &gainInput)
    {
        stateHandler.setStateValue(stateHandler.gainId, value);
        return;
    }

    if (numberInput == &bpmInput)
        stateHandler.setStateValue(stateHandler.bpmId, value);
}
