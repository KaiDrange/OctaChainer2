#pragma once

#include <JuceHeader.h>

#include "StyleSheet.h"

class PanelComponent : public juce::Component
{
public:
    struct Dimension
    {
        enum class Mode
        {
            Fixed,
            Percentage
        };

        Mode mode;
        int value;
        int minimum;

        static constexpr Dimension fixed(const int pixels, const int minimum = 0)
        {
            return { Mode::Fixed, pixels, minimum };
        }

        static constexpr Dimension percentage(const int percent, const int minimum = 0)
        {
            return { Mode::Percentage, percent, minimum };
        }
    };

    Dimension panelWidth;
    Dimension panelHeight;
    juce::String title;
    Rectangle<int> innerBounds;

    juce::Colour borderColour = juce::Colour(StyleSheet::panelBorderColour);
    juce::Colour backgroundColour = juce::Colour(StyleSheet::panelBackgroundColour);

    PanelComponent(Dimension height, Dimension width, juce::String title);

    static int resolveDimension(Dimension dimension, int availableSize);
    int getResolvedWidth(int availableSize) const;
    int getResolvedHeight(int availableSize) const;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    Rectangle<int> calculateInnerBounds() const;
    Rectangle<int> calculateTitleBounds() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelComponent)
};
