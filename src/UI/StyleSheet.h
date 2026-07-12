#pragma once

#include <JuceHeader.h>


class StyleSheet : public juce::LookAndFeel_V4
{
public:
    static constexpr uint32 defaultBackgroundColour = 0xFFF4EBDD;
    static constexpr uint32 backgroundGradientColour = 0xFFB9B6B0;
    static constexpr uint32 panelBackgroundColour = 0xFFF4EBDD;
    static constexpr uint32 panelBorderColour = 0x809DA79D;
    static constexpr uint32 dialogBackgroundColour = 0xFF555555;

    static constexpr int panelMargins = 3;
    static constexpr int panelPadding = 5;
    static constexpr int panelBorderThickness = 2;
    static constexpr int panelBorderCornerSize = 4;
    static constexpr int panelTitleHeight = 20;

    static constexpr int sectionHeaderHeight = 16;
    static constexpr int sectionContentPadding = 8;
    static constexpr int sectionCornerSize = 4;
    static constexpr int sectionBorderThickness = 1;

    static constexpr uint32 textDefaultColour = 0xFF2E322F;
    static constexpr float fontDefaultSize  = 13.0f;

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
    static constexpr uint32 comboBackgroundColour = controlBackgroundColour;
    static constexpr uint32 comboButtonColour = textDefaultColour;
    static constexpr uint32 comboArrowColour = textDefaultColour;
    static constexpr int comboboxHeight = 27;
    static constexpr int defaultButtonHeight = 40;
    static constexpr int buttonMargins = 3;
    static constexpr int sectionGap = 10;
    static constexpr int controlGap = 6;
    static constexpr int inputHeight = 30;

    static juce::Font getTitleFont()
    {
        return juce::FontOptions(16.0f, juce::Font::bold);
    }

    static juce::Font getControlFont()
    {
        return juce::FontOptions(13.0f);
    }

    static juce::Colour getDefaultSectionBackgroundColour()
    {
        return juce::Colour(StyleSheet::panelBackgroundColour).darker(0.03f);
    }

    StyleSheet();

    juce::Font getLabelFont(juce::Label&) override;
    void drawLabel(juce::Graphics& g, juce::Label& label) override;
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;
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
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
                           const juce::String& text, const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override;
    void drawMenuBarBackground(juce::Graphics& g, int width, int height, bool isMouseOverBar,
                               juce::MenuBarComponent& menuBar) override;
    void drawMenuBarItem(juce::Graphics& g, int width, int height,
                         int itemIndex, const juce::String& itemText,
                         bool isMouseOverItem, bool isMenuOpen, bool isMouseOverBar,
                         juce::MenuBarComponent& menuBar) override;
    void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& editor) override;
    void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& editor) override;
};
