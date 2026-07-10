#include "PanelComponent.h"


PanelComponent::PanelComponent(Dimension heightDimension, Dimension widthDimension, juce::String title)
    : width(widthDimension),
      height(heightDimension),
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
    return resolveDimension(width, availableSize);
}

int PanelComponent::getResolvedHeight(const int availableSize) const
{
    return resolveDimension(height, availableSize);
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

    titleArea = titleArea.removeFromTop(juce::roundToInt(StyleSheet::panelBorderMargin * 2.0f));
    titleArea.reduce(0, 2);
    graphics.setFont(StyleSheet::getTitleFont());
    graphics.setColour(juce::Colour(StyleSheet::textDefaultColour));
    graphics.drawText(title, titleArea, juce::Justification::centred);
}

void PanelComponent::resized()
{
    const auto reducedBounds = getLocalBounds().toFloat()
        .reduced(StyleSheet::panelMargins + StyleSheet::panelPadding + StyleSheet::panelBorderThickness);
    innerBounds = reducedBounds.toNearestInt();
    if (!title.isEmpty())
        innerBounds.reduce(StyleSheet::panelBorderMargin * 2.0f, StyleSheet::panelBorderMargin * 2.0f);
}
