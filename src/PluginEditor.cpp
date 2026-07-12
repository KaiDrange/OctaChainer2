#include "PluginEditor.h"

OctaChainer2AudioProcessorEditor::OctaChainer2AudioProcessorEditor(OctaChainer2AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    mainComponent = std::make_unique<MainComponent>(audioProcessor.getStateHandler());
    addAndMakeVisible(*mainComponent);
    setResizable(true, true);
    setResizeLimits(MainComponent::minWidth, MainComponent::minHeight, MainComponent::maxWidth,
                    MainComponent::maxHeight);
    setSize(MainComponent::defaultWidth, MainComponent::defaultHeight);
    mainComponent->setBounds(getLocalBounds());
}

void OctaChainer2AudioProcessorEditor::resized()
{
    mainComponent->setBounds(getLocalBounds());
}

