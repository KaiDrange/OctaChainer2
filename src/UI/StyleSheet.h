#pragma once

#include <JuceHeader.h>


class StyleSheet : public juce::LookAndFeel_V4
{
public:
    static constexpr uint32 backgroundColour = 0xFFF4EBDD;
    static constexpr uint32 backgroundGradientColour = 0xFFC9c6C0;
    static constexpr uint32 panelBackgroundColour = 0xFFF4EBDD;
    static constexpr uint32 panelBorderColour = 0x809DA79D;
    static constexpr float panelMargins = 3.0f;
    static constexpr float panelPadding = 5.0f;
    static constexpr float panelBorderThickness = 2.0f;
    static constexpr float panelBorderCornerSize = 4.0f;
    static constexpr float panelBorderMargin = 7.0f;

    static constexpr uint32 textDefaultColour = 0xFF2E322F;

    static juce::Font getTitleFont()
    {
        return juce::FontOptions(16.0f, juce::Font::bold);
    }

    StyleSheet()
    {
        setColour(juce::Label::textColourId, juce::Colour(textDefaultColour));
    }
};
