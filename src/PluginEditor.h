#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/MainComponent.h"

class OctaChainer2AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit OctaChainer2AudioProcessorEditor(OctaChainer2AudioProcessor&);

    void resized() override;
    std::unique_ptr<MainComponent> mainComponent;

private:
    OctaChainer2AudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OctaChainer2AudioProcessorEditor)
};

