#include "SectionComponent.h"

#include "StyleSheet.h"

namespace
{
    constexpr int headerHeight = 16;
    constexpr int contentPadding = 8;
    constexpr float cornerSize = 4.0f;
}

SectionComponent::SectionComponent(juce::String title)
    : title(std::move(title))
{
}

void SectionComponent::setTitle(juce::String newTitle)
{
    title = std::move(newTitle);
    repaint();
}

const juce::String& SectionComponent::getTitle()
{
    return title;
}

juce::Rectangle<int> SectionComponent::getContentBounds() const noexcept
{
    return contentBounds;
}

void SectionComponent::paint(juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();

    g.setColour(juce::Colour(StyleSheet::panelBackgroundColour).darker(0.03f));
    g.fillRoundedRectangle(area, cornerSize);

    g.setColour(juce::Colour(StyleSheet::panelBorderColour));
    g.drawRoundedRectangle(area, cornerSize, 1.0f);

    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.setColour(juce::Colour(StyleSheet::textDefaultColour));
    const auto titleBounds = juce::Rectangle<int>(0, 0, getWidth(), headerHeight);
    g.drawText(title, titleBounds.reduced(6, 0), juce::Justification::centredLeft);
}

void SectionComponent::resized()
{
    contentBounds = getLocalBounds();
    contentBounds.removeFromTop(headerHeight);
    contentBounds.reduce(contentPadding, contentPadding);
    contentBounds.removeFromTop(4);
}
