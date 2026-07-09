#pragma once

#include <JuceHeader.h>

#include "NumberInputComponent.h"
#include "PanelComponent.h"
#include "SectionComponent.h"

class SettingsPanelComponent : public PanelComponent
{
public:
    SettingsPanelComponent(int heightPercentage, int widthPercentage, const juce::String& title = "");
    void resized() override;

private:
    enum RadioGroupId
    {
        bitrateGroupId = 1,
        channelGroupId = 2,
        sampleRateGroupId = 3
    };

    juce::ToggleButton bitrate16Bit{"16bit"};
    juce::ToggleButton bitrate24Bit{"24bit"};

    juce::ToggleButton channelMono{"Mono"};
    juce::ToggleButton channelStereo{"Stereo"};

    juce::ToggleButton sampleRate48k{"48k"};
    juce::ToggleButton sampleRate44k1{"44.1k"};

    SectionComponent bitrateSection{"Bitrate"};
    SectionComponent channelSection{"Channels"};
    SectionComponent sampleRateSection{"Sample rate"};
    SectionComponent otAttributesSection{"OT attributes"};

    juce::ComboBox timestretchBox;
    juce::ComboBox loopBox;
    juce::ComboBox trigQuantBox;
    juce::ComboBox normalizationBox;
    juce::ComboBox fadeinBox;
    juce::ComboBox fadeoutBox;

    NumberInputComponent gainInput{"Gain:", 2, -24, 24, false};
    NumberInputComponent bpmInput{"BPM:", 3, 30, 300, false};

    static void configureRadioButton(juce::ToggleButton& button, int groupId, bool selected);
};
