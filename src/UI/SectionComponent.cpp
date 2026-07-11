#include "SectionComponent.h"

#include "StyleSheet.h"

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
    constexpr auto cornerSize = static_cast<float>(StyleSheet::sectionCornerSize);

    g.setColour(backgroundColour);
    g.fillRoundedRectangle(area, cornerSize);

    g.setColour(juce::Colour(StyleSheet::panelBorderColour));
    g.drawRoundedRectangle(area.reduced(StyleSheet::sectionBorderThickness * 0.5f), cornerSize, StyleSheet::sectionBorderThickness);

    g.setFont(titleFont);
    g.setColour(juce::Colour(StyleSheet::textDefaultColour));
    const auto titleBounds = juce::Rectangle(0, 0, getWidth(), StyleSheet::sectionHeaderHeight);
    g.drawText(title, titleBounds.reduced(6, 0), juce::Justification::centredLeft);
}

void SectionComponent::resized()
{
    contentBounds = getLocalBounds();
    contentBounds.removeFromTop(StyleSheet::sectionHeaderHeight);
    contentBounds.reduce(StyleSheet::sectionContentPadding, StyleSheet::sectionContentPadding);
    contentBounds.removeFromTop(4);
}
