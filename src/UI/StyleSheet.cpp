#include "StyleSheet.h"

StyleSheet::StyleSheet()
{
    setColour(juce::Label::backgroundColourId, juce::Colour(controlBackgroundColour));
    setColour(juce::Label::textColourId, juce::Colour(controlTextColour));
    setColour(juce::Label::outlineColourId, juce::Colour(controlBorderColour));
    setColour(juce::Label::backgroundWhenEditingColourId, juce::Colour(controlBackgroundFocusedColour));
    setColour(juce::Label::textWhenEditingColourId, juce::Colour(controlTextColour));
    setColour(juce::Label::outlineWhenEditingColourId, juce::Colour(controlBorderFocusedColour));

    setColour(juce::TextEditor::backgroundColourId, juce::Colour(controlBackgroundColour));
    setColour(juce::TextEditor::textColourId, juce::Colour(controlTextColour));
    setColour(juce::TextEditor::outlineColourId, juce::Colour(controlBorderColour));
    setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(controlBorderFocusedColour));
    setColour(juce::TextEditor::highlightColourId, juce::Colour(controlHighlightColour));
    setColour(juce::TextEditor::shadowColourId, juce::Colours::transparentBlack);
    setColour(juce::TextButton::buttonColourId, juce::Colour(buttonBackgroundColour));
    setColour(juce::TextButton::buttonOnColourId, juce::Colour(buttonBackgroundDownColour));
    setColour(juce::TextButton::textColourOffId, juce::Colour(buttonTextColour));
    setColour(juce::TextButton::textColourOnId, juce::Colour(buttonTextColour));
}

juce::Font StyleSheet::getLabelFont(juce::Label&)
{
    return juce::FontOptions(13.0f);
}

void StyleSheet::drawLabel(juce::Graphics& g, juce::Label& label)
{
    const auto alpha = label.isEnabled() ? 1.0f : 0.5f;
    const auto font = getLabelFont(label);
    g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
    g.setFont(font);
    g.drawFittedText(label.getText(),
                     label.getBorderSize().subtractedFrom(label.getLocalBounds()).reduced(controlTextInsetX, controlTextInsetY),
                     label.getJustificationType(),
                     juce::jmax(1, static_cast<int>(static_cast<float>(label.getHeight()) / font.getHeight())),
                     label.getMinimumHorizontalScale());
}

juce::Font StyleSheet::getTextButtonFont(juce::TextButton&, const int buttonHeight)
{
    return juce::FontOptions(juce::jmin(15.0f, static_cast<float>(buttonHeight) * 0.55f), juce::Font::bold);
}

void StyleSheet::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                      const bool shouldDrawButtonAsHighlighted, const bool shouldDrawButtonAsDown)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);
    auto fill = backgroundColour;

    if (! button.isEnabled())
        fill = fill.withMultipliedAlpha(0.45f);
    else if (shouldDrawButtonAsDown)
        fill = juce::Colour(buttonBackgroundDownColour);
    else if (shouldDrawButtonAsHighlighted)
        fill = juce::Colour(buttonBackgroundHoveredColour);

    if (button.hasKeyboardFocus(true))
        fill = fill.brighter(0.05f);

    g.setColour(fill);
    g.fillRoundedRectangle(bounds, 6.0f);

    auto border = juce::Colour(buttonBorderColour);
    if (shouldDrawButtonAsDown)
        border = border.brighter(0.15f);
    else if (shouldDrawButtonAsHighlighted)
        border = border.brighter(0.08f);

    g.setColour(border.withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.55f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
}

void StyleSheet::fillTextEditorBackground(juce::Graphics& g, const int width, const int height, juce::TextEditor& editor)
{
    const auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(0.5f, 0.5f);
    auto fill = editor.hasKeyboardFocus(true) ? juce::Colour(controlBackgroundFocusedColour)
                                              : juce::Colour(controlBackgroundColour);

    if (! editor.isEnabled())
        fill = fill.withMultipliedAlpha(0.6f);

    g.setColour(fill);
    g.fillRoundedRectangle(bounds, 5.0f);
}

void StyleSheet::drawTextEditorOutline(juce::Graphics& g, const int width, const int height, juce::TextEditor& editor)
{
    const auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(0.5f, 0.5f);
    auto border = editor.hasKeyboardFocus(true) ? juce::Colour(controlBorderFocusedColour)
                                                 : juce::Colour(controlBorderColour);

    if (! editor.isEnabled())
        border = border.withMultipliedAlpha(0.5f);

    g.setColour(border);
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
}
