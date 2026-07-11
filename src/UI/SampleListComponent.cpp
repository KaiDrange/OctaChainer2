#include "SampleListComponent.h"

SampleListComponent::SampleListComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                                         const juce::String& title)
    : PanelComponent(height, width, title)
{
    addAndMakeVisible(table);
    addAndMakeVisible(btnAdd);
    addAndMakeVisible(btnAddSilence);
    addAndMakeVisible(btnRemove);
    addAndMakeVisible(btnRemoveAll);
    addAndMakeVisible(chainMaxLength);
}

void SampleListComponent::resized()
{
    PanelComponent::resized();

    auto contentArea = innerBounds;
    auto buttonArea = contentArea.removeFromBottom(StyleSheet::defaultButtonHeight);
    table.setBounds(contentArea);

    btnAdd.setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 5).reduced(StyleSheet::buttonMargins, StyleSheet::buttonMargins));
    btnAddSilence.setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 4).reduced(StyleSheet::buttonMargins, StyleSheet::buttonMargins));
    btnRemove.setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 3).reduced(StyleSheet::buttonMargins, StyleSheet::buttonMargins));
    btnRemoveAll.setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 2).reduced(StyleSheet::buttonMargins, StyleSheet::buttonMargins));
    chainMaxLength.setBounds(buttonArea.reduced(StyleSheet::buttonMargins, StyleSheet::buttonMargins));
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
