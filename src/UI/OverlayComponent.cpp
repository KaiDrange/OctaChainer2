#include "OverlayComponent.h"
#include "StyleSheet.h"

OverlayComponent::OverlayComponent()
{
    setInterceptsMouseClicks(true, true);
    setWantsKeyboardFocus(true);
    Component::setVisible(false);
}

void OverlayComponent::beginOperation(juce::String verb, const int total, juce::String singular, juce::String plural)
{
    operationVerb = std::move(verb);
    singularLabel = std::move(singular);
    pluralLabel = std::move(plural);
    totalSteps.store(juce::jmax(1, total), std::memory_order_release);
    currentStep.store(0, std::memory_order_release);
    repaint();
}

void OverlayComponent::setProgress(const int completedSteps, const int total)
{
    totalSteps.store(juce::jmax(1, total), std::memory_order_release);
    currentStep.store(juce::jlimit(0, juce::jmax(1, total), completedSteps), std::memory_order_release);
    repaint();
}

void OverlayComponent::setActive(const bool shouldBeActive)
{
    setVisible(shouldBeActive);

    if (shouldBeActive)
    {
        toFront(false);
        grabKeyboardFocus();
        repaint();
    }
}

void OverlayComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xAA111111));

    const auto total = juce::jmax(1, totalSteps.load(std::memory_order_acquire));
    const auto current = juce::jlimit(0, total, currentStep.load(std::memory_order_acquire));
    const auto& noun = total == 1 ? singularLabel : pluralLabel;

    juce::String message = operationVerb.isEmpty() ? "Working" : operationVerb;
    message += " ";
    if (total == 1)
        message += noun;
    else
        message += juce::String(current) + "/" + juce::String(total) + " " + noun;
    message += "...";

    g.setColour(juce::Colours::white.withAlpha(0.96f));
    g.setFont(StyleSheet::getControlFont().withHeight(17.0f).boldened());
    g.drawFittedText(message, getLocalBounds().reduced(24), juce::Justification::centred, 1);
}

bool OverlayComponent::keyPressed(const juce::KeyPress&)
{
    return true;
}
