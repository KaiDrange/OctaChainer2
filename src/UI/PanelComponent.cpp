#include "PanelComponent.h"

#include <utility>

PanelComponent::PanelComponent(const int heightPercentage, const int widthPercentage, const juce::String& title)
    : widthPercentage(widthPercentage), heightPercentage(heightPercentage)
{
    this->title = title;
}

void PanelComponent::paint(juce::Graphics& graphics)
{
    auto area = getLocalBounds().toFloat();
    area.reduce(StyleSheet::panelMargins, StyleSheet::panelMargins);
    area.reduce(StyleSheet::panelBorderThickness * 0.5f, StyleSheet::panelBorderThickness * 0.5f);
    graphics.setColour(backgroundColour);
    graphics.fillRoundedRectangle(area, StyleSheet::panelBorderCornerSize);
    graphics.setColour(borderColour);
    graphics.drawRoundedRectangle(area, StyleSheet::panelBorderCornerSize, StyleSheet::panelBorderThickness);
    auto titleArea = area.toNearestInt();
    area.reduce(StyleSheet::panelBorderMargin, StyleSheet::panelBorderMargin);
    graphics.drawRoundedRectangle(area, StyleSheet::panelBorderCornerSize, StyleSheet::panelBorderThickness);

    if (title.isEmpty())
        return;

    titleArea = titleArea.removeFromTop(StyleSheet::panelBorderMargin * 2);
    titleArea.reduce(0, 2);
    graphics.setFont(StyleSheet::titleFont);
    graphics.setColour(juce::Colour(StyleSheet::textDefaultColour));
    graphics.drawText(title, titleArea, juce::Justification::centred);
}

void PanelComponent::resized()
{
    innerBounds = getLocalBounds().toFloat()
        .reduced(StyleSheet::panelMargins + StyleSheet::panelPadding + StyleSheet::panelBorderThickness).toNearestInt();
}
