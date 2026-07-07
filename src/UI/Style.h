#pragma once
#include <JuceHeader.h>

class Style : public juce::LookAndFeel_V4
{
public:
    const juce::Colour textDefault = juce::Colour(0xFFFFFFFF);

    Style()
    {
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0x112233FF));

        setColour(juce::Label::textColourId, textDefault);
    }
};
