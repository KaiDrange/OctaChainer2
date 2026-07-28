#pragma once

#include <JuceHeader.h>
#include "NumberInputComponent.h"

class SliceActionsDialogComponent final : public juce::Component
{
public:
    enum ResultId
    {
        cropToRange = 1,
        splitSlice = 2,
        mergeWithSliceAbove = 3,
        normalizeSlice = 4,
        divideByBpm = 5
    };

    SliceActionsDialogComponent(double bpmDefault, bool canCropToRange, bool canMergeWithSliceAbove);

    void resized() override;

private:
    void configureOptionButton(juce::TextButton& button, const juce::String& text, int resultId) const;
    void closeDialog(int result = 0) const;

    static constexpr int dialogWidth = 520;
    static constexpr int dialogHeight = 230;
    static constexpr int outerMargin = 16;
    static constexpr int rowGap = 12;
    static constexpr int optionButtonHeight = 28;
    static constexpr int sectionGap = 16;
    static constexpr int inputHeight = 48;
    static constexpr int inputColumnGap = 12;
    static constexpr int footerButtonWidth = 96;

    juce::TextButton cropButton;
    juce::TextButton splitButton;
    juce::TextButton mergeButton;
    juce::TextButton normalizeButton;
    juce::TextButton divideByBpmButton;
    NumberInputComponent sliceCountInput;
    NumberInputComponent bpmInput;
    NumberInputComponent sixteenthNotesInput;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliceActionsDialogComponent)
};
