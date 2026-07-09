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
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(comboBackgroundColour));
    setColour(juce::ComboBox::textColourId, juce::Colour(controlTextColour));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(controlBorderColour));
    setColour(juce::ComboBox::buttonColourId, juce::Colour(comboButtonColour));
    setColour(juce::ComboBox::arrowColourId, juce::Colour(comboArrowColour));
    setColour(juce::ComboBox::focusedOutlineColourId, juce::Colour(controlBorderFocusedColour));
    setColour(juce::ToggleButton::textColourId, juce::Colour(controlTextColour));
    setColour(juce::ToggleButton::tickColourId, juce::Colour(toggleTickColour));
    setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(controlBorderColour));
}

juce::Font StyleSheet::getLabelFont(juce::Label&)
{
    return juce::FontOptions(13.0f);
}

void StyleSheet::drawLabel(juce::Graphics& g, juce::Label& label)
{
    const auto alpha = label.isEnabled() ? 1.0f : 0.5f;
    const auto font = getLabelFont(label);
    const auto textArea = label.getBorderSize().subtractedFrom(label.getLocalBounds()).reduced(controlTextInsetX, controlTextInsetY);

    if (dynamic_cast<juce::ComboBox*> (label.getParentComponent()) != nullptr)
    {
        g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
        g.setFont(font);
        g.drawFittedText(label.getText(),
                         textArea,
                         label.getJustificationType(),
                         juce::jmax(1, static_cast<int>(static_cast<float>(label.getHeight()) / font.getHeight())),
                         label.getMinimumHorizontalScale());
        return;
    }

    g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
    g.setFont(font);
    g.drawFittedText(label.getText(),
                     textArea,
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

void StyleSheet::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                  const bool shouldDrawButtonAsHighlighted, const bool shouldDrawButtonAsDown)
{
    const auto fontSize = juce::jmin(15.0f, static_cast<float>(button.getHeight()) * 0.72f);
    const auto tickWidth = juce::jmax(14.0f, fontSize * 1.05f);

    drawTickBox(g, button,
                4.0f,
                (static_cast<float>(button.getHeight()) - tickWidth) * 0.5f,
                tickWidth, tickWidth,
                button.getToggleState(),
                button.isEnabled(),
                shouldDrawButtonAsHighlighted,
                shouldDrawButtonAsDown);

    g.setColour(button.findColour(juce::ToggleButton::textColourId).withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));
    g.setFont(fontSize);
    g.drawFittedText(button.getButtonText(),
                     button.getLocalBounds().withTrimmedLeft(juce::roundToInt(tickWidth) + 8)
                                            .withTrimmedRight(2),
                     juce::Justification::centredLeft,
                     1);
}

void StyleSheet::drawTickBox(juce::Graphics& g, juce::Component& component,
                             const float x, const float y, const float w, const float h,
                             const bool ticked, const bool isEnabled,
                             const bool shouldDrawButtonAsHighlighted, const bool shouldDrawButtonAsDown)
{
    auto bounds = juce::Rectangle<float>(x, y, w, h).reduced(0.5f, 0.5f);
    auto outline = component.findColour(juce::ToggleButton::tickDisabledColourId);
    auto fill = juce::Colours::transparentBlack;

    if (shouldDrawButtonAsDown)
        fill = outline.withAlpha(0.12f);
    else if (shouldDrawButtonAsHighlighted)
        fill = outline.withAlpha(0.06f);

    g.setColour(fill);
    g.fillEllipse(bounds);

    g.setColour(outline.withMultipliedAlpha(isEnabled ? 1.0f : 0.45f));
    g.drawEllipse(bounds, 1.0f);

    if (ticked)
    {
        auto inner = bounds.reduced(bounds.getWidth() * 0.28f);
        g.setColour(component.findColour(juce::ToggleButton::tickColourId).withMultipliedAlpha(isEnabled ? 1.0f : 0.45f));
        g.fillEllipse(inner);
    }
}

void StyleSheet::changeToggleButtonWidthToFitText(juce::ToggleButton& button)
{
    const auto fontSize = juce::jmin(15.0f, static_cast<float>(button.getHeight()) * 0.72f);
    const auto tickWidth = juce::jmax(14.0f, fontSize * 1.05f);
    const juce::Font font { juce::FontOptions { fontSize } };

    button.setSize(juce::GlyphArrangement::getStringWidthInt(font, button.getButtonText())
                   + juce::roundToInt(tickWidth) + 14,
                   button.getHeight());
}

juce::Font StyleSheet::getComboBoxFont(juce::ComboBox&)
{
    return juce::FontOptions(13.0f);
}

void StyleSheet::drawComboBox(juce::Graphics& g, const int width, const int height, const bool isButtonDown,
                              const int buttonX, const int buttonY, const int buttonW, const int buttonH,
                              juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(0.5f, 0.5f);
    auto fill = box.findColour(juce::ComboBox::backgroundColourId);

    if (! box.isEnabled())
        fill = fill.withMultipliedAlpha(0.55f);
    else if (isButtonDown)
        fill = fill.brighter(0.03f);

    g.setColour(fill);
    g.fillRoundedRectangle(bounds, 5.0f);

    auto border = box.hasKeyboardFocus(true) ? box.findColour(juce::ComboBox::focusedOutlineColourId)
                                             : box.findColour(juce::ComboBox::outlineColourId);

    if (isButtonDown)
        border = border.brighter(0.08f);

    g.setColour(border.withMultipliedAlpha(box.isEnabled() ? 1.0f : 0.55f));
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);

    auto arrowColour = box.findColour(juce::ComboBox::arrowColourId).withMultipliedAlpha(box.isEnabled() ? 1.0f : 0.45f);
    auto arrowBounds = juce::Rectangle<float>(static_cast<float>(buttonX), static_cast<float>(buttonY),
                                              static_cast<float>(buttonW), static_cast<float>(buttonH));
    juce::Path arrow;
    arrow.addTriangle(arrowBounds.getCentreX() - 4.0f, arrowBounds.getCentreY() - 2.0f,
                      arrowBounds.getCentreX() + 4.0f, arrowBounds.getCentreY() - 2.0f,
                      arrowBounds.getCentreX(), arrowBounds.getCentreY() + 4.0f);
    g.setColour(arrowColour);
    g.fillPath(arrow);
}

void StyleSheet::positionComboBoxText(juce::ComboBox& box, juce::Label& labelToPosition)
{
    labelToPosition.setBounds(juce::Rectangle<int>(4, 0, juce::jmax(0, box.getWidth() - 22), box.getHeight()));
    labelToPosition.setFont(getComboBoxFont(box));
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
