#pragma once
#include <JuceHeader.h>
#include "Style.h"

class MainComponent : public juce::Component
{
    public:
        static constexpr int defaultWidth = 1536;
        static constexpr int defaultHeight = 864;
        static constexpr int maxWidth = 2560;
        static constexpr int maxHeight = 1440;
        static constexpr int minWidth = 800;
        static constexpr int minHeight = 600;

        MainComponent();
        ~MainComponent() override;

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)

        juce::Label testLabel;
        Style style;
};
