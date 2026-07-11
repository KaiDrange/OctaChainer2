#include "PanelComponent.h"


PanelComponent::PanelComponent(Dimension heightDimension, Dimension widthDimension, juce::String title)
    : panelWidth(widthDimension),
      panelHeight(heightDimension),
      title(std::move(title))
{
}

int PanelComponent::resolveDimension(const Dimension dimension, const int availableSize)
{
    const auto minimum = juce::jmax(0, dimension.minimum);
    const auto resolved = dimension.mode == Dimension::Mode::Fixed
        ? juce::jmin(availableSize, dimension.value)
        : availableSize * dimension.value / 100;

    return juce::jmax(minimum, resolved);
}

int PanelComponent::getResolvedWidth(const int availableSize) const
{
    return resolveDimension(panelWidth, availableSize);
}

int PanelComponent::getResolvedHeight(const int availableSize) const
{
    return resolveDimension(panelHeight, availableSize);
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

    if (title.isEmpty())
        return;

    graphics.setFont(StyleSheet::getTitleFont());
    graphics.setColour(juce::Colour(StyleSheet::textDefaultColour));
    const auto titleArea = calculateTitleBounds();
    graphics.drawText(title, titleArea, juce::Justification::centredTop);
}

void PanelComponent::resized()
{
    innerBounds = calculateInnerBounds();
}

Rectangle<int> PanelComponent::calculateInnerBounds() const
{
    auto reducedBounds = getLocalBounds()
            .reduced(StyleSheet::panelMargins + StyleSheet::panelPadding + StyleSheet::panelBorderThickness);
    if (!title.isEmpty())
        reducedBounds.removeFromTop(StyleSheet::panelTitleHeight);

    return reducedBounds.toNearestInt();
}

Rectangle<int> PanelComponent::calculateTitleBounds() const
{
    auto reducedBounds = getLocalBounds()
            .reduced(StyleSheet::panelMargins + StyleSheet::panelPadding + StyleSheet::panelBorderThickness);
    return  reducedBounds.removeFromTop(StyleSheet::panelTitleHeight);
}
