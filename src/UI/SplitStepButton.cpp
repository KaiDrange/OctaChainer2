#include "SplitStepButton.h"
#include "StyleSheet.h"

SplitStepButton::SplitStepButton()
    : juce::Button("Number stepper")
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void SplitStepButton::setIncrementEnabled(const bool enabled)
{
    if (incrementEnabled != enabled)
    {
        incrementEnabled = enabled;
        repaint();
    }
}

void SplitStepButton::setDecrementEnabled(const bool enabled)
{
    if (decrementEnabled != enabled)
    {
        decrementEnabled = enabled;
        repaint();
    }
}

void SplitStepButton::mouseMove(const juce::MouseEvent& event)
{
    updateHoverDirection(event.position);
    juce::Button::mouseMove(event);
}

void SplitStepButton::mouseDrag(const juce::MouseEvent& event)
{
    updateHoverDirection(event.position);
    juce::Button::mouseDrag(event);
}

void SplitStepButton::mouseDown(const juce::MouseEvent& event)
{
    pressedDirection = getDirectionForPoint(event.position);
    updateHoverDirection(event.position);
    juce::Button::mouseDown(event);
}

void SplitStepButton::mouseUp(const juce::MouseEvent& event)
{
    juce::Button::mouseUp(event);
    pressedDirection = Direction::none;
    repaint();
}

void SplitStepButton::mouseExit(const juce::MouseEvent& event)
{
    hoverDirection = Direction::none;
    juce::Button::mouseExit(event);
    repaint();
}

void SplitStepButton::paintButton(juce::Graphics& g, const bool shouldDrawButtonAsHighlighted, const bool shouldDrawButtonAsDown)
{
    const auto bounds = getLocalBounds().toFloat().reduced(0.5f, 0.5f);
    constexpr auto cornerSize = 3.0f;
    constexpr auto dividerThickness = 1.0f;
    const auto halfHeight = bounds.getHeight() * 0.5f;

    g.setColour(juce::Colour(StyleSheet::buttonBackgroundColour).withAlpha(isEnabled() ? 1.0f : 0.45f));
    g.fillRoundedRectangle(bounds, cornerSize);

    auto topHalf = bounds.withHeight(halfHeight);
    auto bottomHalf = bounds.withY(bounds.getY() + halfHeight).withHeight(bounds.getBottom() - (bounds.getY() + halfHeight));

    paintHalf(g, topHalf, Direction::increment, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    paintHalf(g, bottomHalf, Direction::decrement, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    g.setColour(juce::Colour(StyleSheet::buttonBorderColour).withAlpha(isEnabled() ? 1.0f : 0.55f));
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);

    g.setColour(juce::Colour(StyleSheet::buttonBorderColour).withAlpha(isEnabled() ? 0.7f : 0.35f));
    g.fillRect(bounds.getX() + 1.0f, bounds.getCentreY() - dividerThickness * 0.5f, bounds.getWidth() - 2.0f, dividerThickness);
}

SplitStepButton::Direction SplitStepButton::getDirectionForPoint(const juce::Point<float>& point) const
{
    return point.y < static_cast<float>(getHeight()) * 0.5f ? Direction::increment : Direction::decrement;
}

bool SplitStepButton::isDirectionEnabled(const Direction direction) const
{
    if (direction == Direction::increment)
        return incrementEnabled;

    if (direction == Direction::decrement)
        return decrementEnabled;

    return false;
}

void SplitStepButton::triggerDirection(const Direction direction) const
{
    if (! isDirectionEnabled(direction) || onStep == nullptr)
        return;

    onStep(direction);
}

void SplitStepButton::updateHoverDirection(const juce::Point<float>& point)
{
    const auto newDirection = getDirectionForPoint(point);
    if (hoverDirection != newDirection)
    {
        hoverDirection = newDirection;
        repaint();
    }
}

void SplitStepButton::paintHalf(juce::Graphics& g, const juce::Rectangle<float>& bounds, const Direction direction,
                                const bool shouldDrawButtonAsHighlighted, const bool shouldDrawButtonAsDown) const
{
    const auto active = isDirectionEnabled(direction);
    auto fill = juce::Colour(StyleSheet::buttonBackgroundColour);

    if (!isEnabled() || !active)
        fill = fill.withMultipliedAlpha(0.45f);
    else if (shouldDrawButtonAsDown && pressedDirection == direction)
        fill = juce::Colour(StyleSheet::buttonBackgroundDownColour);
    else if (hoverDirection == direction || shouldDrawButtonAsHighlighted)
        fill = juce::Colour(StyleSheet::buttonBackgroundHoveredColour);

    if (!active && isEnabled())
        fill = fill.withMultipliedAlpha(0.8f);

    g.setColour(fill);
    auto fillBounds = bounds.reduced(1.0f, 0.0f);
    if (direction == Direction::increment)
        fillBounds = fillBounds.withBottom(bounds.getBottom() + 0.5f);
    else
        fillBounds = fillBounds.withTop(bounds.getY() - 0.5f);

    g.fillRect(fillBounds);

    g.setColour(juce::Colour(StyleSheet::buttonTextColour).withAlpha(active && isEnabled() ? 1.0f : 0.5f));
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawFittedText(direction == Direction::increment ? "+" : "-",
                     bounds.toNearestInt(),
                     juce::Justification::centred,
                     1);
}

void SplitStepButton::clicked(const juce::ModifierKeys&)
{
    triggerDirection(pressedDirection);
}
