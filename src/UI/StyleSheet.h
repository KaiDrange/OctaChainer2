#pragma once

#include <JuceHeader.h>


class StyleSheet : public juce::LookAndFeel_V4
{
public:
    static constexpr uint32 backgroundColour = 0xFF102038;
    static constexpr uint32 backgroundGradientColour = 0xFFAA77BB;
    static constexpr uint32 panelBackgroundColour = 0x50FFFFFF;
    static constexpr uint32 panelBorderColour = 0x40000000;
    static constexpr float panelMargins = 5.0f;
    static constexpr float panelPadding = 10.0f;
    static constexpr float panelBorderThickness = 3.0f;
    static constexpr float panelBorderCornerSize = 6.0f;
    static constexpr float panelBorderMargin = 10.0f;

    static constexpr uint32 textDefaultColour = 0xFFFFFFFF;

    inline static const juce::Font titleFont { juce::FontOptions(20.0f, juce::Font::bold) };

    StyleSheet()
    {
        setColour(juce::Label::textColourId, juce::Colour(textDefaultColour));
    }
};
