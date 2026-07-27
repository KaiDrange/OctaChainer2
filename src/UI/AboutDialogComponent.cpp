#include "AboutDialogComponent.h"
#include "StyleSheet.h"

AboutDialogComponent::AboutDialogComponent(const juce::String& appName,
                                           const juce::String& versionString)
    : sourceButton("Source code", juce::URL(sourceCodeUrl)),
      releasesButton("Releases", juce::URL(releasesUrl)),
      juceButton("JUCE", juce::URL(juceUrl))
{
    setSize(dialogWidth, dialogHeight);

    titleLabel.setText(appName, juce::dontSendNotification);
    titleLabel.setFont(StyleSheet::getTitleFont());
    titleLabel.setJustificationType(juce::Justification::centredLeft);

    versionLabel.setText("Version " + versionString, juce::dontSendNotification);
    versionLabel.setFont(StyleSheet::getControlFont());
    versionLabel.setJustificationType(juce::Justification::centredLeft);

    descriptionBox.setMultiLine(true);
    descriptionBox.setReturnKeyStartsNewLine(true);
    descriptionBox.setReadOnly(true);
    descriptionBox.setPopupMenuEnabled(false);
    descriptionBox.setCaretVisible(false);
    descriptionBox.setText("OctaChainer 2 is a tool for merging short audio clips into long chains. "
                           "It was written primarily for the Elektron Octatrack, but is designed to be useful for "
                           "other samplers as well. It is released both as a standalone application and as a VST3. "
                           "OctaChainer 2 is written by Kai Drange and is an open source project released under the "
                           "MIT license. The author go by the user name 'Abhoth' at Elektronauts. Feel free to reach "
                           "out to him there if you have any questions or suggestions.");
    descriptionBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    descriptionBox.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    descriptionBox.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    descriptionBox.setColour(juce::TextEditor::shadowColourId, juce::Colours::transparentBlack);
    descriptionBox.setColour(juce::TextEditor::textColourId, juce::Colour(StyleSheet::textDefaultColour));
    descriptionBox.setFont(StyleSheet::getControlFont());

    linksLabel.setText("Links", juce::dontSendNotification);
    linksLabel.setFont(StyleSheet::getControlFont().boldened());
    linksLabel.setJustificationType(juce::Justification::centredLeft);

    closeButton.onClick = [this]
    {
        closeDialog();
    };

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(versionLabel);
    addAndMakeVisible(descriptionBox);
    addAndMakeVisible(linksLabel);
    addAndMakeVisible(sourceButton);
    addAndMakeVisible(releasesButton);
    addAndMakeVisible(juceButton);
    addAndMakeVisible(closeButton);
}

void AboutDialogComponent::resized()
{
    auto area = getLocalBounds().reduced(outerMargin);

    titleLabel.setBounds(area.removeFromTop(24));
    versionLabel.setBounds(area.removeFromTop(20));
    area.removeFromTop(rowGap);

    descriptionBox.setBounds(area.removeFromTop(96));
    area.removeFromTop(rowGap);

    linksLabel.setBounds(area.removeFromTop(20));
    area.removeFromTop(4);

    sourceButton.setBounds(area.removeFromTop(buttonHeight));
    area.removeFromTop(4);
    releasesButton.setBounds(area.removeFromTop(buttonHeight));
    area.removeFromTop(4);
    juceButton.setBounds(area.removeFromTop(buttonHeight));
    area.removeFromTop(rowGap);

    auto footerArea = area.removeFromBottom(buttonHeight);
    closeButton.setBounds(footerArea.removeFromRight(100));
}

void AboutDialogComponent::closeDialog() const
{
    if (auto* dialog = findParentComponentOfClass<juce::DialogWindow>())
        dialog->exitModalState(0);
}
