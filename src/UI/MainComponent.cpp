#include "MainComponent.h"

MainComponent::MainComponent()
{
    setLookAndFeel(&style);
    testLabel.setText("Hello World", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(testLabel);
}

MainComponent::~MainComponent()
{
    setLookAndFeel(nullptr);
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll();
    g.setFont(juce::FontOptions(24.0f));
    g.drawFittedText("OctaChainer 2", getLocalBounds(), juce::Justification::centred, 1);
}

void MainComponent::resized()
{
    testLabel.setBounds(getLocalBounds());
}
