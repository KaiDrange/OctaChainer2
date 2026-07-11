#pragma once

#include <JuceHeader.h>

#include "PanelComponent.h"
#include "TransportButtonComponent.h"


class AudioPanelComponent : public PanelComponent
{
public:
    AudioPanelComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                           const juce::String& title = "");
    void resized() override;

private:
    TransportButtonComponent btnPlaySlice{"Slice"};
    TransportButtonComponent btnPlayChain{"Chain"};
    juce::Slider masterVolumeSlider;
    juce::Label masterVolumeLabel;
};
