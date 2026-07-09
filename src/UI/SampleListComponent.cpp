#include "SampleListComponent.h"

SampleListComponent::SampleListComponent(const int heightPercentage, const int widthPercentage, const juce::String& title) : PanelComponent(heightPercentage, widthPercentage, title)
{
    addAndMakeVisible(table);
}

void SampleListComponent::resized()
{
    PanelComponent::resized();
    table.setOpaque(false);
    table.getViewport()->setOpaque(false);
    table.setBounds(innerBounds);
}

int SampleListComponent::getNumRows()
{
    return 0;
}

void SampleListComponent::paintRowBackground(juce::Graphics& g, int rowNumber, int, int, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour (juce::ListBox::backgroundColourId).interpolatedWith (getLookAndFeel().findColour (juce::ListBox::textColourId), 0.03f);
    if (rowIsSelected)
        g.fillAll (juce::Colours::lightblue);
    else if (rowNumber % 2)
        g.fillAll (alternateColour);
}

void SampleListComponent::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height,
    bool rowIsSelected)
{
}
