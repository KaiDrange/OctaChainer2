#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/MainComponent.h"
#include "UI/TransportButtonComponent.h"

class OctaChainer2AudioProcessorEditor : public juce::AudioProcessorEditor,
                                         public MainComponent::Listener
{
public:
    explicit OctaChainer2AudioProcessorEditor(OctaChainer2AudioProcessor&);
    ~OctaChainer2AudioProcessorEditor() override;

    void resized() override;
    void transportButtonPressed(TransportButtonComponent::TransportEvent event) override;
    std::unique_ptr<MainComponent> mainComponent;

private:
    OctaChainer2AudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OctaChainer2AudioProcessorEditor)
};

