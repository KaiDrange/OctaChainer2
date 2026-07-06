#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class OctaChainer2AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit OctaChainer2AudioProcessorEditor(OctaChainer2AudioProcessor&);
    ~OctaChainer2AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    OctaChainer2AudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OctaChainer2AudioProcessorEditor)
};

