#include "SliceActionsDialogComponent.h"
#include "../Core/StateHandler.h"
#include "StyleSheet.h"

SliceActionsDialogComponent::SliceActionsDialogComponent(const double bpmDefault,
                                                        const bool canCropToRange,
                                                        const bool canMergeWithSliceAbove,
                                                        ActionCallback onActionSelected)
    : sliceCountInput("Slice count:", 1.0, 64.0, 1.0, 1.0, true),
      bpmInput("BPM:", StateHandler::bpmValue.min, StateHandler::bpmValue.max, bpmDefault, 0.01, true),
      sixteenthNotesInput("Sixteenth notes per slice:", 1.0, 64.0, 16.0, 1.0, true),
      onActionSelected(std::move(onActionSelected))
{
    setSize(dialogWidth, dialogHeight);

    sliceCountInput.setLabelColour(juce::Colour(StyleSheet::buttonTextColour));
    sliceCountInput.setValueColour(juce::Colour(StyleSheet::darkInputsValueColour));
    bpmInput.setLabelColour(juce::Colour(StyleSheet::buttonTextColour));
    bpmInput.setValueColour(juce::Colour(StyleSheet::darkInputsValueColour));
    sixteenthNotesInput.setLabelColour(juce::Colour(StyleSheet::buttonTextColour));
    sixteenthNotesInput.setValueColour(juce::Colour(StyleSheet::darkInputsValueColour));

    configureOptionButton(cropButton, "Crop to range", cropToRange);
    configureOptionButton(splitButton, "Divide evenly", splitSlice);
    configureOptionButton(mergeButton, "Merge with slice above", mergeWithSliceAbove);
    configureOptionButton(normalizeButton, "Normalize", normalizeSlice);
    configureOptionButton(divideByBpmButton, "Divide by BPM", divideByBpm);

    cropButton.setEnabled(canCropToRange);
    mergeButton.setEnabled(canMergeWithSliceAbove);

    addAndMakeVisible(cropButton);
    addAndMakeVisible(splitButton);
    addAndMakeVisible(mergeButton);
    addAndMakeVisible(normalizeButton);
    addAndMakeVisible(divideByBpmButton);
    addAndMakeVisible(sliceCountInput);
    addAndMakeVisible(bpmInput);
    addAndMakeVisible(sixteenthNotesInput);
}

void SliceActionsDialogComponent::resized()
{
    auto area = getLocalBounds().reduced(outerMargin);

    cropButton.setBounds(area.removeFromTop(optionButtonHeight));
    area.removeFromTop(rowGap);
    mergeButton.setBounds(area.removeFromTop(optionButtonHeight));
    area.removeFromTop(rowGap);
    normalizeButton.setBounds(area.removeFromTop(optionButtonHeight));

    auto divideEvenlyRow = area.removeFromTop(inputHeight);
    const auto inputWidth = juce::jmax(1, (divideEvenlyRow.getWidth() - (inputColumnGap * 2)) / 4);
    const auto splitButtonArea = divideEvenlyRow.removeFromLeft(inputWidth * 2);
    divideEvenlyRow.removeFromLeft(inputColumnGap);
    const auto sliceCountArea = divideEvenlyRow.removeFromLeft(inputWidth);
    splitButton.setBounds(splitButtonArea.withHeight(optionButtonHeight)
                                        .withY(splitButtonArea.getBottom() - optionButtonHeight));
    sliceCountInput.setBounds(sliceCountArea);

    auto divideBpmRow = area.removeFromTop(inputHeight);

    const auto divideButtonArea = divideBpmRow.removeFromLeft(inputWidth * 2);
    divideBpmRow.removeFromLeft(inputColumnGap);
    const auto bpmArea = divideBpmRow.removeFromLeft(inputWidth);
    divideBpmRow.removeFromLeft(inputColumnGap);
    const auto sixteenthArea = divideBpmRow.removeFromLeft(inputWidth);

    divideByBpmButton.setBounds(divideButtonArea.withHeight(optionButtonHeight)
                                              .withY(divideButtonArea.getBottom() - optionButtonHeight));
    bpmInput.setBounds(bpmArea);
    sixteenthNotesInput.setBounds(sixteenthArea);
}

void SliceActionsDialogComponent::configureOptionButton(juce::TextButton& button, const juce::String& text, const int resultId) const
{
    button.setButtonText(text);
    button.onClick = [this, resultId]
    {
        if (onActionSelected)
            onActionSelected(static_cast<ResultId>(resultId));

        closeDialog(resultId);
    };
}

void SliceActionsDialogComponent::closeDialog(const int result) const
{
    if (auto* dialog = findParentComponentOfClass<juce::DialogWindow>())
        dialog->exitModalState(result);
}
