#pragma once
#include "PanelComponent.h"


class WaveformComponent : public PanelComponent
{
public:
    WaveformComponent(PanelComponent::Dimension height, PanelComponent::Dimension width,
                      const juce::String& title);
};

