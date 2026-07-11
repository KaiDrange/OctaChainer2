#pragma once

#include <JuceHeader.h>

#include "StyleSheet.h"

class SectionComponent : public juce::Component
{
public:
    explicit SectionComponent(juce::String title = "");

    void setTitle(juce::String newTitle);
    const juce::String& getTitle();
    juce::Rectangle<int> getContentBounds() const noexcept;

    void paint(juce::Graphics& g) override;
    void resized() override;

    juce::Colour backgroundColour{StyleSheet::getDefaultSectionBackgroundColour()};

private:
    juce::String title;
    juce::Rectangle<int> contentBounds;
    juce::Font titleFont{juce::FontOptions(StyleSheet::fontDefaultSize, juce::Font::bold)};
};
