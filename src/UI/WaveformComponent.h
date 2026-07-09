#pragma once
#include "PanelComponent.h"


class WaveformComponent : public PanelComponent
{
public:
    WaveformComponent(int heightPercentage, int widthPercentage, const juce::String& title);
};

