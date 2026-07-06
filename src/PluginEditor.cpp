#include "PluginEditor.h"

OctaChainer2AudioProcessorEditor::OctaChainer2AudioProcessorEditor(OctaChainer2AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    juce::ignoreUnused(audioProcessor);
    setSize(480, 320);
}

OctaChainer2AudioProcessorEditor::~OctaChainer2AudioProcessorEditor() = default;

void OctaChainer2AudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(24.0f));
    g.drawFittedText("OctaChainer2", getLocalBounds(), juce::Justification::centred, 1);
}

void OctaChainer2AudioProcessorEditor::resized()
{
}

