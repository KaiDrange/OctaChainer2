#include "StyleSheet.h"

StyleSheet::StyleSheet()
{
    setColour(juce::Label::textColourId, juce::Colour(textDefaultColour));
    setColour(juce::TextButton::buttonColourId, juce::Colour(buttonBackgroundColour));
    setColour(juce::TextButton::buttonOnColourId, juce::Colour(buttonBackgroundDownColour));
    setColour(juce::TextButton::textColourOffId, juce::Colour(buttonTextColour));
    setColour(juce::TextButton::textColourOnId, juce::Colour(buttonTextColour));
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
