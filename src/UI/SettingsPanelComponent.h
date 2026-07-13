#pragma once

#include <JuceHeader.h>

#include "../Core/StateHandler.h"
#include "NumberInputComponent.h"
#include "PanelComponent.h"
#include "SectionComponent.h"

class SettingsPanelComponent : public PanelComponent,
                              private StateHandler::Listener,
                              private NumberInputComponent::Listener
{
public:
    SettingsPanelComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                           StateHandler& stateHandlerToUse,
                           const juce::String& title = "");
    ~SettingsPanelComponent() override;
    void resized() override;

    static constexpr int topSectionHeight = 80;
    static constexpr int otSectionHeight = 160;
    static constexpr int chainExportSectionHeight = 180;
    static constexpr int megabreakExportSectionHeight = 63;

    void stateChanged() override;
    void numberInputChanged(NumberInputComponent* numberInput) override;

private:
    void layoutTopSections();
    void layoutOtAttributesSection();
    void layoutChainExportSection();
    void layoutMegabreakExportSection();

    enum RadioGroupId
    {
        bitrateGroupId = 1,
        channelGroupId = 2,
        sampleRateGroupId = 3
    };

    juce::ToggleButton bitrate16Bit;
    juce::ToggleButton bitrate24Bit;

    juce::ToggleButton channelMono;
    juce::ToggleButton channelStereo;

    juce::ToggleButton sampleRate48k;
    juce::ToggleButton sampleRate44k;

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

    NumberInputComponent gainInput{"Gain:", -24, 24, 0.0f, 0.5f, false};
    NumberInputComponent bpmInput{"BPM:", 30, 300, 120, 0.25f, false};

    juce::ToggleButton exportOtFile{"OT file"};
    juce::ToggleButton exportEvenGrid{"Even grid"};
    juce::ToggleButton exportEmbedMarkers{"Embed markers"};
    juce::TextButton createButton{"Save chain"};
    juce::TextButton createMegabreakButton{"Save megabreak"};

    static void configureRadioButtons(StateHandler& stateHandler, const juce::Identifier& identifier,
                                      int groupId, std::initializer_list<juce::ToggleButton*> buttons);
    static void configureRadioButton(juce::ToggleButton& button, int groupId);

    StateHandler& stateHandler;
};
