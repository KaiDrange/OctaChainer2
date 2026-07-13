#pragma once

#include <JuceHeader.h>

#include "NumberInputComponent.h"
#include "PanelComponent.h"
#include "../Core/StateHandler.h"

class SampleListComponent : public PanelComponent, public juce::TableListBoxModel
{
public:
    SampleListComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                        const juce::String& title = "");
    void resized() override;

    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/,
                            bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

private:
    juce::TableListBox table{"Sample List", this};
    juce::TextButton btnAdd{"Add file"};
    juce::TextButton btnAddSilence{"Add blank"};
    juce::TextButton btnRemove{"Remove"};
    juce::TextButton btnRemoveAll{"Clear"};
    NumberInputComponent chainMaxLength{
        "Max slices:", StateHandler::chainMaxLengthValue.min, StateHandler::chainMaxLengthValue.max,
        StateHandler::chainMaxLengthValue.defaultValue, StateHandler::chainMaxLengthValue.stepSize, false
    };
};
