#pragma once

#include <JuceHeader.h>

#include "NumberInputComponent.h"
#include "PanelComponent.h"
#include "SectionComponent.h"

class SettingsPanelComponent : public PanelComponent
{
public:
    SettingsPanelComponent(PanelComponent::Dimension height, PanelComponent::Dimension width,
                           const juce::String& title = "");
    void resized() override;

    static constexpr int topSectionHeight = 80;
    static constexpr int otSectionHeight = 240;
    static constexpr int chainExportSectionHeight = 96;
    static constexpr int megabreakExportSectionHeight = 63;

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
    SectionComponent chainExportSection{"Chain export"};
    SectionComponent megabreakExportSection{"Megabreak export"};

    juce::ComboBox timestretchBox;
    juce::ComboBox loopBox;
    juce::ComboBox trigQuantBox;
    juce::ComboBox normalizationBox;
    juce::ComboBox fadeinBox;
    juce::ComboBox fadeoutBox;
    juce::ComboBox megabreakFileCountBox;

    NumberInputComponent gainInput{"Gain:", 3, -24, 24, false};
    NumberInputComponent bpmInput{"BPM:", 3, 30, 300, false};

    juce::ToggleButton exportOtFile{"OT file"};
    juce::ToggleButton exportEvenGrid{"Even grid"};
    juce::ToggleButton exportEmbedMarkers{"Embed markers"};
    juce::TextButton createButton{"Save chain"};
    juce::TextButton createMegabreakButton{"Save megabreak"};

    static void configureRadioButton(juce::ToggleButton& button, int groupId, bool selected);
};
