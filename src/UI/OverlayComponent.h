#pragma once

#include <JuceHeader.h>

class OverlayComponent : public juce::Component
{
public:
    OverlayComponent();

    void beginOperation(juce::String operationVerb, int totalSteps, juce::String singularLabel, juce::String pluralLabel);
    void setProgress(int completedSteps, int totalSteps);
    void setActive(bool shouldBeActive);

    void paint(juce::Graphics& g) override;
    bool keyPressed(const juce::KeyPress& key) override;

private:
    juce::String operationVerb;
    std::atomic<int> currentStep{0};
    std::atomic<int> totalSteps{1};
    juce::String singularLabel;
    juce::String pluralLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OverlayComponent)
};
