#include "SettingsPanelComponent.h"
#include "../Core/OtFileFormat.h"

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

    bitDepthSection.setTooltip("Bit depth of exported audio file(s)");
    channelSection.setTooltip("Number of channels of exported audio file(s)");
    sampleRateSection.setTooltip("Sample rate of exported audio file(s)");
    otAttributesSection.setTooltip("Settings for Elektron Octatrack OT file(s)");
    chainExportSection.setTooltip("Settings for exported .wav chain");
    megabreakExportSection.setTooltip("Elektron Octatrack megabreak export");

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
    exportOtFile.setTooltip("A separate .ot file with metadata used by Elektron Octatrack");
    chainExportSection.addAndMakeVisible(exportEvenGrid);
    exportEvenGrid.setTooltip("Makes all slices the same length, based on the longest slice in the chain");
    chainExportSection.addAndMakeVisible(exportEmbedMarkers);
    exportEmbedMarkers.setTooltip("Embeds cue markers for slices in the exported wav file(s)");
    chainExportSection.addAndMakeVisible(createButton);
    createButton.addShortcut(KeyPress('S', ModifierKeys::ctrlModifier, 0));
    createButton.setTooltip("Combines slices into one or more .wav files (Hotkey: Ctrl + S)");

    megabreakExportSection.addAndMakeVisible(megabreakFadeinBox);
    megabreakExportSection.addAndMakeVisible(megabreakFadeoutBox);
    megabreakExportSection.addAndMakeVisible(megabreakFileCountBox);
    megabreakFadeinBox.setTooltip("Applied to each megabreak part");
    megabreakFadeoutBox.setTooltip("Applied to each megabreak part");
    megabreakFileCountBox.setTooltip("Each slice will be divided into this many parts and saved in separate files");
    megabreakExportSection.addAndMakeVisible(createMegabreakButton);
    createMegabreakButton.setTooltip("Creates a set of megabreak files for the Elektron Octatrack");

    createButton.setEnabled(false);
    createMegabreakButton.setEnabled(false);

    configureRadioButtons(stateHandler, StateHandler::bitDepthId, bitDepthGroupId, { &bitDepth16Bit, &bitDepth24Bit });
    configureRadioButtons(stateHandler, StateHandler::channelsId, channelGroupId, { &channelMono, &channelStereo });
    configureRadioButtons(stateHandler, StateHandler::samplerateId, sampleRateGroupId, { &sampleRate44k, &sampleRate48k });

    timestretchBox.onChange = [this]{ stateHandler.setStateValueFromItemId(StateHandler::timestretchId, timestretchBox.getSelectedId()); };
    loopBox.onChange = [this]{ stateHandler.setStateValueFromItemId(StateHandler::loopModeId, loopBox.getSelectedId()); };
    trigQuantBox.onChange = [this]{ stateHandler.setStateValueFromItemId(StateHandler::triqQuantId, trigQuantBox.getSelectedId()); };
    normalizationBox.onChange = [this]{ stateHandler.setStateValueFromItemId(StateHandler::normalizationId, normalizationBox.getSelectedId()); };
    fadeinBox.onChange = [this]{ stateHandler.setStateValueFromItemId(StateHandler::fadeinId, fadeinBox.getSelectedId()); };
    fadeoutBox.onChange = [this]{ stateHandler.setStateValueFromItemId(StateHandler::fadeoutId, fadeoutBox.getSelectedId()); };
    megabreakFadeinBox.onChange = [this]{ stateHandler.setStateValueFromItemId(StateHandler::megabreakFadeinId, megabreakFadeinBox.getSelectedId()); };
    megabreakFadeoutBox.onChange = [this]{ stateHandler.setStateValueFromItemId(StateHandler::megabreakFadeoutId, megabreakFadeoutBox.getSelectedId()); };
    exportOtFile.onClick = [this]{ stateHandler.setStateValue(StateHandler::otFileId, exportOtFile.getToggleState()); };
    exportEvenGrid.onClick = [this]{ stateHandler.setStateValue(StateHandler::evenGridId, exportEvenGrid.getToggleState()); };
    exportEmbedMarkers.onClick = [this]{ stateHandler.setStateValue(StateHandler::embedMarkersId, exportEmbedMarkers.getToggleState()); };
    megabreakFileCountBox.onChange = [this]{ stateHandler.setStateValueFromItemId(StateHandler::megabreakFileCountId, megabreakFileCountBox.getSelectedId()); };
    createButton.onClick = [this]{ sendChainExportRequested(); };
    createMegabreakButton.onClick = [this]{ sendMegabreakExportRequested(); };

    gainInput.addListener(this);
    bpmInput.addListener(this);

    SettingsPanelComponent::stateChanged({ StateHandler::StateChange::fullReload });
}

SettingsPanelComponent::~SettingsPanelComponent()
{
    gainInput.removeListener(this);
    bpmInput.removeListener(this);
    stateHandler.removeListener(this);
}

void SettingsPanelComponent::addListener(Listener* listener)
{
    listeners.add(listener);
}

void SettingsPanelComponent::removeListener(Listener* listenerToRemove)
{
    jassert(listeners.contains(listenerToRemove));
    listeners.remove(listenerToRemove);
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
    const auto fadeRow = megabreakExportControls.removeFromTop(StyleSheet::comboboxHeight);
    const auto fadeWidth = (fadeRow.getWidth() - StyleSheet::controlGap) / 2;
    megabreakFadeinBox.setBounds(fadeRow.getX(), fadeRow.getY(), fadeWidth, StyleSheet::comboboxHeight);
    megabreakFadeoutBox.setBounds(fadeRow.getX() + fadeWidth + StyleSheet::controlGap,
                                  fadeRow.getY(), fadeWidth, StyleSheet::comboboxHeight);

    megabreakExportControls.removeFromTop(StyleSheet::controlGap);
    const auto megabreakRow = megabreakExportControls.removeFromTop(StyleSheet::comboboxHeight);
    const auto megabreakWidth = (megabreakRow.getWidth() - StyleSheet::controlGap) / 2;
    megabreakFileCountBox.setBounds(megabreakRow.getX(), megabreakRow.getY(), megabreakWidth, StyleSheet::comboboxHeight);
    createMegabreakButton.setBounds(megabreakRow.getX() + megabreakWidth + StyleSheet::controlGap,
                                    megabreakRow.getY(), megabreakWidth, StyleSheet::comboboxHeight);
}

void SettingsPanelComponent::updateExportButtonState()
{
    const auto otCompatibleExport = isOtCompatibleExport(stateHandler, hasSlices);
    const auto megabreakCompatibleExport = isMegabreakCompatibleExport(stateHandler, hasSlices);
    createButton.setEnabled(hasSlices && (! exportOtFile.getToggleState() || otCompatibleExport));
    createMegabreakButton.setEnabled(megabreakCompatibleExport);
}

void SettingsPanelComponent::stateChanged(const StateHandler::StateChange& change)
{
    bool shouldUpdateExportButtons = false;

    if (change.has(StateHandler::StateChange::sliceList) || change.has(StateHandler::StateChange::fullReload))
    {
        hasSlices = stateHandler.getNumSlices() > 0;
        shouldUpdateExportButtons = true;
    }

    if (! change.has(StateHandler::StateChange::settings) && ! change.has(StateHandler::StateChange::fullReload))
    {
        if (shouldUpdateExportButtons)
            updateExportButtonState();
        return;
    }

    stateHandler.refreshRadioButtons(StateHandler::bitDepthId, { &bitDepth16Bit, &bitDepth24Bit });
    stateHandler.refreshRadioButtons(StateHandler::channelsId, { &channelMono, &channelStereo });
    stateHandler.refreshRadioButtons(StateHandler::samplerateId, { &sampleRate44k, &sampleRate48k });

    stateHandler.refreshComboBox(StateHandler::timestretchId, timestretchBox);
    stateHandler.refreshComboBox(StateHandler::loopModeId, loopBox);
    stateHandler.refreshComboBox(StateHandler::triqQuantId, trigQuantBox);
    stateHandler.refreshComboBox(StateHandler::normalizationId, normalizationBox);
    stateHandler.refreshComboBox(StateHandler::fadeinId, fadeinBox);
    stateHandler.refreshComboBox(StateHandler::fadeoutId, fadeoutBox);
    stateHandler.refreshComboBox(StateHandler::megabreakFadeinId, megabreakFadeinBox);
    stateHandler.refreshComboBox(StateHandler::megabreakFadeoutId, megabreakFadeoutBox);
    stateHandler.refreshComboBox(StateHandler::megabreakFileCountId, megabreakFileCountBox);

    gainInput.setValue(juce::var(stateHandler.getStateValue<double>(StateHandler::gainId, StateHandler::gainValue.defaultValue)));
    bpmInput.setValue(juce::var(stateHandler.getStateValue<double>(StateHandler::bpmId, StateHandler::bpmValue.defaultValue)));
    exportOtFile.setToggleState(juce::var(stateHandler.getStateValue<bool>(StateHandler::otFileId, StateHandler::otFileDefault)), juce::NotificationType::dontSendNotification);
    exportEvenGrid.setToggleState(juce::var(stateHandler.getStateValue<bool>(StateHandler::evenGridId, StateHandler::evenGridDefault)), juce::NotificationType::dontSendNotification);
    exportEmbedMarkers.setToggleState(juce::var(stateHandler.getStateValue<bool>(StateHandler::embedMarkersId, StateHandler::embedMarkersDefault)), juce::NotificationType::dontSendNotification);

    updateExportButtonState();
}

void SettingsPanelComponent::numberInputChanged(NumberInputComponent* numberInput)
{
    const auto value = numberInput->getValue();
    if (value.isVoid())
        return;

    if (numberInput == &gainInput)
    {
        stateHandler.setStateValue(StateHandler::gainId, value);
        return;
    }

    if (numberInput == &bpmInput)
        stateHandler.setStateValue(StateHandler::bpmId, value);
}

bool SettingsPanelComponent::isOtCompatibleExport(const StateHandler& stateHandler, const bool hasSlices)
{
    if (!hasSlices)
        return false;

    const auto maxSlicesPerChain = juce::jmax(1, stateHandler.getStateValue<int>(
        StateHandler::chainMaxLengthId,
        static_cast<int>(StateHandler::chainMaxLengthValue.defaultValue)));
    return maxSlicesPerChain <= static_cast<int>(OtFileFormat::maxSliceCount);
}

bool SettingsPanelComponent::isMegabreakCompatibleExport(const StateHandler& stateHandler, const bool hasSlices)
{
    const auto sliceCount = stateHandler.getNumSlices();
    juce::ignoreUnused(hasSlices);
    return sliceCount > 0 && sliceCount <= static_cast<int>(OtFileFormat::maxSliceCount);
}

void SettingsPanelComponent::sendChainExportRequested()
{
    listeners.call([](Listener& listener)
    {
        listener.chainExportRequested();
    });
}

void SettingsPanelComponent::sendMegabreakExportRequested()
{
    listeners.call([](Listener& listener)
    {
        listener.megabreakExportRequested();
    });
}
