#pragma once

#include <JuceHeader.h>

#include "StyleSheet.h"

class PanelComponent : public juce::Component
{
public:
    int widthPercentage;
    int heightPercentage;
    juce::String title;
    Rectangle<int> innerBounds;

    juce::Colour borderColour = juce::Colour(StyleSheet::panelBorderColour);
    juce::Colour backgroundColour = juce::Colour(StyleSheet::panelBackgroundColour);

    PanelComponent(int heightPercentage, int widthPercentage, const juce::String& title);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelComponent)
};
