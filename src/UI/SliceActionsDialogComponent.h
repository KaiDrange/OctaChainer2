#pragma once

#include <JuceHeader.h>
#include "NumberInputComponent.h"

class SliceActionsDialogComponent final : public juce::Component
{
public:
    enum ResultId
    {
        cloneSlice = 1,
        cropToRange = 2,
        splitSlice = 3,
        mergeWithSliceAbove = 4,
        normalizeSlice = 5,
        divideByBpm = 6
    };

    using ActionCallback = std::function<void(ResultId, const SliceActionsDialogComponent&)>;
    using SliceNameCommitCallback = std::function<void(const juce::String&)>;

    SliceActionsDialogComponent(double bpmDefault,
                                const juce::String& currentSliceName,
                                bool canCropToRange,
                                bool canMergeWithSliceAbove,
                                ActionCallback onActionSelected = {},
                                SliceNameCommitCallback onSliceNameCommitted = {});

    void resized() override;
    [[nodiscard]] int getSliceCount() const;
    [[nodiscard]] double getDivideByBpmValue() const;
    [[nodiscard]] double getSixteenthNotesPerSlice() const;

private:
    void configureOptionButton(juce::TextButton& button, const juce::String& text, int resultId) const;
    void closeDialog(int result = 0) const;

    static constexpr int dialogWidth = 520;
    static constexpr int dialogHeight = 280;
    static constexpr int outerMargin = 16;
    static constexpr int rowGap = 12;
    static constexpr int optionButtonHeight = 28;
    static constexpr int sectionGap = 16;
    static constexpr int inputHeight = 48;
    static constexpr int inputColumnGap = 12;
    static constexpr int footerButtonWidth = 96;

    juce::TextEditor sliceNameInput;
    juce::TextButton cloneButton;
    juce::TextButton cropButton;
    juce::TextButton splitButton;
    juce::TextButton mergeButton;
    juce::TextButton normalizeButton;
    juce::TextButton divideByBpmButton;
    NumberInputComponent sliceCountInput;
    NumberInputComponent bpmInput;
    NumberInputComponent sixteenthNotesInput;
    ActionCallback onActionSelected;
    SliceNameCommitCallback onSliceNameCommitted;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliceActionsDialogComponent)
};
