#pragma once

#include <JuceHeader.h>

#include "StyleSheet.h"
#include "PanelComponent.h"


class AudioPanelComponent : public PanelComponent
{
public:
    AudioPanelComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                           const juce::String& title = "");
    void resized() override;

private:
    juce::DrawableButton btnPlaySlice{"Play Slice", juce::DrawableButton::ImageFitted};
    juce::DrawableButton btnPlayChain{"Play Chain", juce::DrawableButton::ImageFitted};
    juce::Label sliceLabel;
    juce::Label chainLabel;
    juce::Slider masterVolumeSlider;
    juce::Label masterVolumeLabel;
};
