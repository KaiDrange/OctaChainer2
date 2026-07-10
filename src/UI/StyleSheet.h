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
    static constexpr uint32 toggleTickColour = 0xFF0A84FF;

    static constexpr uint32 controlBackgroundColour = 0xFFF7F2E9;
    static constexpr uint32 controlBackgroundFocusedColour = 0xFFFFFFFF;
    static constexpr uint32 controlBorderColour = 0x809DA79D;
    static constexpr uint32 controlBorderFocusedColour = 0xFF6D8376;
    static constexpr uint32 controlTextColour = 0xFF2E322F;
    static constexpr uint32 controlHighlightColour = 0x407D9284;
    static constexpr int controlTextInsetX = 6;
    static constexpr int controlTextInsetY = 2;
    static constexpr uint32 comboBackgroundColour = 0xFFE8E2D8;
    static constexpr uint32 comboButtonColour = textDefaultColour;
    static constexpr uint32 comboArrowColour = textDefaultColour;
    static constexpr float defaultButtonHeight = 40.0f;
    static constexpr float buttonMargins = 3.0f;
    static constexpr int sectionGap = 10;
    static constexpr int controlGap = 6;
    static constexpr int comboboxHeight = 27;
    static constexpr int inputHeight = 30;

    static juce::Font getTitleFont()
    {
        return juce::FontOptions(16.0f, juce::Font::bold);
    }

    static juce::Font getControlFont()
    {
        return juce::FontOptions(13.0f);
    }

    StyleSheet();

    juce::Font getLabelFont(juce::Label&) override;
    void drawLabel(juce::Graphics& g, juce::Label& label) override;
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawTickBox(juce::Graphics& g, juce::Component& component,
                     float x, float y, float w, float h,
                     bool ticked, bool isEnabled,
                     bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void changeToggleButtonWidthToFitText(juce::ToggleButton& button) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;
    void positionComboBoxText(juce::ComboBox& box, juce::Label& labelToPosition) override;
    void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& editor) override;
    void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& editor) override;
};
