#pragma once

#include <JuceHeader.h>

class TransportButtonComponent : public juce::Component
{
public:
    explicit TransportButtonComponent(const juce::String& labelText);

    juce::DrawableButton& getButton() noexcept;
    const juce::DrawableButton& getButton() const noexcept;
    bool showLabel = true;

    void resized() override;

private:
    static std::unique_ptr<juce::DrawablePath> createPlayIcon(juce::Colour colour);
    static std::unique_ptr<juce::DrawablePath> createStopIcon(juce::Colour colour);

    juce::DrawableButton button;
    juce::Label label;
};
