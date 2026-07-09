#pragma once

#include <JuceHeader.h>

#include "PanelComponent.h"

class SettingsPanelComponent : public PanelComponent
{
public:
    SettingsPanelComponent(int heightPercentage, int widthPercentage, const juce::String& title = "");
};
