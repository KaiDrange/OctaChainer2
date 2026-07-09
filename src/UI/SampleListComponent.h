#pragma once

#include <JuceHeader.h>

#include "PanelComponent.h"

class SampleListComponent : public PanelComponent, public juce::TableListBoxModel
{
public:
    SampleListComponent(int heightPercentage, int widthPercentage, const juce::String& title = "");
    void resized() override;

    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/,
                            bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

private:
    juce::TableListBox table{"Sample List", this};
};
