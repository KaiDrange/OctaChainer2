#pragma once

#include <JuceHeader.h>


class StyleSheet : public juce::LookAndFeel_V4
{
public:
    static constexpr uint32 defaultBackgroundColour = 0xFFF4EBDD;
    static constexpr uint32 backgroundGradientColour = 0xFFC9c6C0;
    static constexpr uint32 panelBackgroundColour = 0xFFF4EBDD;
    static constexpr uint32 panelBorderColour = 0x809DA79D;
    static constexpr float panelMargins = 3.0f;
    static constexpr float panelPadding = 5.0f;
    static constexpr float panelBorderThickness = 2.0f;
    static constexpr float panelBorderCornerSize = 4.0f;
    static constexpr float panelBorderMargin = 7.0f;

    static constexpr uint32 textDefaultColour = 0xFF2E322F;

    static constexpr uint32 buttonBackgroundColour = 0xFF6D8376;
    static constexpr uint32 buttonBackgroundHoveredColour = 0xFF7D9284;
    static constexpr uint32 buttonBackgroundDownColour = 0xFF55695D;
    static constexpr uint32 buttonBorderColour = 0x80545F58;
    static constexpr uint32 buttonTextColour = 0xFFF7F5EF;

    static constexpr float defaultButtonHeight = 40.0f;
    static constexpr float buttonMargins = 3.0f;

    static juce::Font getTitleFont()
    {
        return juce::FontOptions(16.0f, juce::Font::bold);
    }

    StyleSheet();

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};
