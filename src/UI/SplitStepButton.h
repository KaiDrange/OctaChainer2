#pragma once

#include <JuceHeader.h>

class SplitStepButton final : public juce::Button
{
public:
    enum class Direction
    {
        none,
        increment,
        decrement
    };

    SplitStepButton();

    std::function<void(Direction)> onStep;

    void setIncrementEnabled(bool enabled);
    void setDecrementEnabled(bool enabled);
    void mouseMove(const juce::MouseEvent& event) override;

protected:
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void clicked(const juce::ModifierKeys&) override;

private:
    Direction hoverDirection = Direction::none;
    Direction pressedDirection = Direction::none;
    bool incrementEnabled = true;
    bool decrementEnabled = true;

    Direction getDirectionForPoint(const juce::Point<float>& point) const;
    bool isDirectionEnabled(Direction direction) const;
    void triggerDirection(Direction direction) const;
    void updateHoverDirection(const juce::Point<float>& point);
    void paintHalf(juce::Graphics& g, const juce::Rectangle<float>& bounds, Direction direction,
                   bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) const;

};
