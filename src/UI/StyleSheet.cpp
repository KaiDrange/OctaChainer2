#include "StyleSheet.h"

namespace
{
    bool isTransportDrawableButton(const juce::Button& button)
    {
        const auto* drawable = dynamic_cast<const juce::DrawableButton*>(&button);
        return drawable != nullptr && drawable->getButtonText().startsWithIgnoreCase("Play");
    }

    void drawTransportButtonBackground(juce::Graphics& g, juce::Button& button,
                                       const bool shouldDrawButtonAsHighlighted, const bool shouldDrawButtonAsDown)
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);
        const auto accent = button.getToggleState() ? juce::Colour(0xFFD94B4B)
                                                    : juce::Colour(0xFF2EAF58);
        auto base = juce::Colour(StyleSheet::controlBackgroundColour);

        if (! button.isEnabled())
        {
            base = base.withMultipliedAlpha(0.45f);
        }

        const auto shadowOffset = shouldDrawButtonAsDown ? 1.0f : 2.0f;
        g.setColour(juce::Colours::black.withAlpha(button.isEnabled() ? (shouldDrawButtonAsDown ? 0.12f : 0.16f) : 0.0f));
        g.fillRoundedRectangle(bounds.translated(0.0f, shadowOffset), 10.0f);

        auto top = base.brighter(0.16f).interpolatedWith(accent.withAlpha(0.14f), 0.16f);
        auto bottom = base.darker(0.10f).interpolatedWith(accent.withAlpha(0.24f), 0.24f);

        if (shouldDrawButtonAsDown)
        {
            top = top.darker(0.03f);
            bottom = bottom.darker(0.08f);
        }
        else if (shouldDrawButtonAsHighlighted)
        {
            top = top.brighter(0.12f).interpolatedWith(juce::Colours::white.withAlpha(0.10f), 0.35f);
            bottom = bottom.brighter(0.08f).interpolatedWith(accent.withAlpha(0.34f), 0.22f);
        }

        juce::ColourGradient gradient(top, bounds.getCentreX(), bounds.getY(),
                                      bottom, bounds.getCentreX(), bounds.getBottom(), false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds, 10.0f);

        auto inner = bounds.reduced(1.0f);
        const auto sheenAlpha = shouldDrawButtonAsDown ? 0.08f : (shouldDrawButtonAsHighlighted ? 0.26f : 0.18f);
        juce::ColourGradient sheen(juce::Colours::white.withAlpha(sheenAlpha),
                                   inner.getCentreX(), inner.getY(),
                                   juce::Colours::transparentWhite,
                                   inner.getCentreX(), inner.getCentreY(), false);
        g.setGradientFill(sheen);
        g.fillRoundedRectangle(inner, 9.0f);

        auto border = accent.withAlpha(0.38f);
        if (shouldDrawButtonAsDown)
            border = border.brighter(0.18f);
        else if (shouldDrawButtonAsHighlighted)
            border = border.brighter(0.18f);

        g.setColour(border.withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.55f));
        g.drawRoundedRectangle(bounds, 10.0f, 1.0f);

        if (shouldDrawButtonAsHighlighted && ! shouldDrawButtonAsDown)
        {
            g.setColour(accent.withAlpha(button.isEnabled() ? 0.16f : 0.0f));
            g.drawRoundedRectangle(bounds.reduced(2.0f), 8.0f, 1.0f);
        }

        g.setColour(juce::Colours::white.withAlpha(button.isEnabled() ? (shouldDrawButtonAsDown ? 0.08f : 0.18f) : 0.0f));
        g.drawRoundedRectangle(bounds.reduced(1.5f), 8.0f, 1.0f);
    }
}

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
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(panelBackgroundColour));
    setColour(juce::PopupMenu::textColourId, juce::Colour(textDefaultColour));
    setColour(juce::PopupMenu::headerTextColourId, juce::Colour(textDefaultColour));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(buttonBackgroundHoveredColour));
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(buttonTextColour));
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(comboBackgroundColour));
    setColour(juce::ComboBox::textColourId, juce::Colour(controlTextColour));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(controlBorderColour));
    setColour(juce::ComboBox::buttonColourId, juce::Colour(comboButtonColour));
    setColour(juce::ComboBox::arrowColourId, juce::Colour(comboArrowColour));
    setColour(juce::ComboBox::focusedOutlineColourId, juce::Colour(controlBorderFocusedColour));
    setColour(juce::ToggleButton::textColourId, juce::Colour(controlTextColour));
    setColour(juce::ToggleButton::tickColourId, juce::Colour(toggleTickColour));
    setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(controlBorderColour));
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(controlBorderFocusedColour));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(controlBorderColour));
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
    if (isTransportDrawableButton(button))
    {
        drawTransportButtonBackground(g, button, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
        return;
    }

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

void StyleSheet::drawRotarySlider(juce::Graphics& g, const int x, const int y, const int width, const int height,
                                  const float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle,
                                  juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                               static_cast<float>(width), static_cast<float>(height)).reduced(2.0f);
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centreX = bounds.getCentreX();
    const auto centreY = bounds.getCentreY();
    const auto pointerThickness = juce::jmax(1.5f, radius * 0.14f);
    const auto trackThickness = juce::jmax(2.0f, radius * 0.12f);
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto arcRadius = radius - pointerThickness;

    auto outline = slider.findColour(juce::Slider::rotarySliderOutlineColourId);
    auto fill = slider.findColour(juce::Slider::rotarySliderFillColourId);

    if (! slider.isEnabled())
    {
        outline = outline.withMultipliedAlpha(0.45f);
        fill = fill.withMultipliedAlpha(0.45f);
    }

    g.setColour(juce::Colour(controlBackgroundColour));
    g.fillEllipse(bounds);

    g.setColour(outline);
    g.drawEllipse(bounds, trackThickness);

    juce::Path arc;
    arc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                      rotaryStartAngle, angle, true);
    g.setColour(fill);
    g.strokePath(arc, juce::PathStrokeType(trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path pointer;
    const auto pointerLength = radius * 0.62f;
    pointer.addRoundedRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength, pointerThickness * 0.5f);
    g.setColour(juce::Colour(buttonBackgroundColour));
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
}

void StyleSheet::drawTickBox(juce::Graphics& g, juce::Component& component,
                             const float x, const float y, const float w, const float h,
                             const bool ticked, const bool isEnabled,
                             const bool shouldDrawButtonAsHighlighted, const bool shouldDrawButtonAsDown)
{
    auto bounds = juce::Rectangle<float>(x, y, w, h).reduced(0.5f, 0.5f);
    const auto* toggle = dynamic_cast<juce::ToggleButton*> (&component);
    const auto isRadio = toggle != nullptr && toggle->getRadioGroupId() != 0;
    auto outline = component.findColour(juce::ToggleButton::tickDisabledColourId);
    auto fill = juce::Colours::transparentBlack;

    if (shouldDrawButtonAsDown)
        fill = outline.withAlpha(0.12f);
    else if (shouldDrawButtonAsHighlighted)
        fill = outline.withAlpha(0.06f);

    if (isRadio)
    {
        g.setColour(fill);
        g.fillEllipse(bounds);

        g.setColour(outline.withMultipliedAlpha(isEnabled ? 1.0f : 0.45f));
        g.drawEllipse(bounds, 1.0f);
    }
    else
    {
        g.setColour(fill);
        g.fillRoundedRectangle(bounds, 2.0f);

        g.setColour(outline.withMultipliedAlpha(isEnabled ? 1.0f : 0.45f));
        g.drawRoundedRectangle(bounds, 2.0f, 1.0f);
    }

    if (ticked)
    {
        g.setColour(component.findColour(juce::ToggleButton::tickColourId).withMultipliedAlpha(isEnabled ? 1.0f : 0.45f));
        if (isRadio)
        {
            auto inner = bounds.reduced(bounds.getWidth() * 0.28f);
            g.fillEllipse(inner);
        }
        else
        {
            auto inner = bounds.reduced(bounds.getWidth() * 0.25f, bounds.getHeight() * 0.20f);
            juce::Path tick;
            tick.startNewSubPath(inner.getX(), inner.getCentreY());
            tick.lineTo(inner.getCentreX() - inner.getWidth() * 0.15f, inner.getBottom());
            tick.lineTo(inner.getRight(), inner.getY());
            g.strokePath(tick, juce::PathStrokeType(2.0f));
        }
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
    return juce::FontOptions(StyleSheet::fontDefaultSize);
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

void StyleSheet::drawPopupMenuBackground(juce::Graphics& g, const int width, const int height)
{
    const auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

    g.setColour(juce::Colour(panelBackgroundColour));
    g.fillRect(bounds);

    g.setColour(juce::Colour(panelBorderColour).withAlpha(0.78f));
    g.drawRect(bounds.toNearestInt());
}

void StyleSheet::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                   const bool isSeparator, const bool isActive, const bool isHighlighted,
                                   const bool isTicked, const bool hasSubMenu,
                                   const juce::String& text, const juce::String& shortcutKeyText,
                                   const juce::Drawable* icon, const juce::Colour* textColour)
{
    if (isSeparator)
    {
        auto r = area.reduced(6, 0);
        r.removeFromTop(juce::roundToInt(static_cast<float>(r.getHeight()) * 0.5f));

        g.setColour(juce::Colour(panelBorderColour).withAlpha(0.45f));
        g.fillRect(r.removeFromTop(1));
        return;
    }

    auto r = area.reduced(2, 1);
    auto fill = juce::Colours::transparentBlack;
    auto fg = textColour != nullptr ? *textColour : juce::Colour(textDefaultColour);

    if (isHighlighted && isActive)
    {
        fill = juce::Colour(buttonBackgroundHoveredColour);
        fg = juce::Colour(buttonTextColour);
    }
    else if (! isActive)
    {
        fg = fg.withMultipliedAlpha(0.45f);
    }

    if (fill.getAlpha() > 0.0f)
    {
        g.setColour(fill);
        g.fillRoundedRectangle(r.toFloat(), 4.0f);
    }

    g.setColour(fg);
    auto font = getPopupMenuFont();
    const auto maxFontHeight = static_cast<float>(r.getHeight()) / 1.3f;
    if (font.getHeight() > maxFontHeight)
        font.setHeight(maxFontHeight);
    g.setFont(font);

    auto iconArea = r.removeFromLeft(juce::roundToInt(maxFontHeight));
    if (icon != nullptr)
    {
        icon->drawWithin(g, iconArea.toFloat(), juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
        r.removeFromLeft(juce::roundToInt(maxFontHeight * 0.5f));
    }
    else if (isTicked)
    {
        auto tick = getTickShape(1.0f);
        g.fillPath(tick, tick.getTransformToScaleToFit(iconArea.reduced(iconArea.getWidth() / 5, 0).toFloat(), true));
        r.removeFromLeft(juce::roundToInt(maxFontHeight * 0.5f));
    }

    if (hasSubMenu)
    {
        auto arrowH = 0.6f * getPopupMenuFont().getAscent();
        auto x = static_cast<float>(r.removeFromRight(static_cast<int>(arrowH)).getX());
        auto halfH = static_cast<float>(r.getCentreY());

        juce::Path path;
        path.startNewSubPath(x, halfH - arrowH * 0.5f);
        path.lineTo(x + arrowH * 0.6f, halfH);
        path.lineTo(x, halfH + arrowH * 0.5f);
        g.strokePath(path, juce::PathStrokeType(2.0f));
    }

    r.removeFromRight(4);
    g.drawFittedText(text, r, juce::Justification::centredLeft, 1);

    if (shortcutKeyText.isNotEmpty())
    {
        auto shortcutFont = font;
        shortcutFont.setHeight(shortcutFont.getHeight() * 0.75f);
        shortcutFont.setHorizontalScale(0.95f);
        g.setFont(shortcutFont);
        g.drawText(shortcutKeyText, r, juce::Justification::centredRight, true);
    }
}

void StyleSheet::drawMenuBarBackground(juce::Graphics& g, const int width, const int height,
                                       const bool isMouseOverBar, juce::MenuBarComponent& menuBar)
{
    juce::ignoreUnused(menuBar);

    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    auto fill = juce::Colour(panelBackgroundColour);

    if (isMouseOverBar)
        fill = fill.brighter(0.02f);

    g.setColour(fill);
    g.fillRect(bounds);

    g.setColour(juce::Colour(panelBorderColour).withAlpha(0.75f));
    g.fillRect(bounds.removeFromBottom(1.0f));
}

void StyleSheet::drawMenuBarItem(juce::Graphics& g, const int width, const int height,
                                 const int itemIndex, const juce::String& itemText,
                                 const bool isMouseOverItem, const bool isMenuOpen, const bool,
                                 juce::MenuBarComponent& menuBar)
{
    juce::ignoreUnused(itemIndex);

    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(1.0f, 1.0f);
    auto textColour = juce::Colour(textDefaultColour);
    auto fill = juce::Colours::transparentBlack;

    if (! menuBar.isEnabled())
    {
        textColour = textColour.withMultipliedAlpha(0.45f);
    }
    else if (isMenuOpen || isMouseOverItem)
    {
        fill = juce::Colour(buttonBackgroundHoveredColour);
        textColour = juce::Colour(buttonTextColour);
    }
    else if (menuBar.isMouseOver())
    {
        fill = juce::Colour(buttonBackgroundHoveredColour).withAlpha(0.35f);
    }

    if (fill.isOpaque() || fill.getAlpha() > 0.0f)
    {
        g.setColour(fill);
        g.fillRoundedRectangle(bounds, 4.0f);
    }

    g.setColour(textColour);
    g.setFont(getMenuBarFont(menuBar, itemIndex, itemText));
    g.drawFittedText(itemText, bounds.getSmallestIntegerContainer(), juce::Justification::centred, 1);
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
