#pragma once

#include <JuceHeader.h>
#include "../Core/StateHandler.h"
#include "NumberInputComponent.h"
#include "PanelComponent.h"
#include "SectionComponent.h"

class SettingsPanelComponent : public PanelComponent, StateHandler::Listener, NumberInputComponent::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void chainExportRequested() = 0;
        virtual void megabreakExportRequested() = 0;
    };

    SettingsPanelComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                           StateHandler& stateHandlerToUse,
                           const juce::String& panelTitle = "");
    ~SettingsPanelComponent() override;
    void resized() override;

    void addListener(Listener* listener);
    void removeListener(Listener* listenerToRemove);

    static constexpr int topSectionHeight = 67;
    static constexpr int otSectionHeight = 129;
    static constexpr int chainExportSectionHeight = 129;
    static constexpr int megabreakExportSectionHeight = 80;

    void stateChanged(const StateHandler::StateChange& change) override;
    void numberInputChanged(NumberInputComponent* numberInput) override;

private:
    static bool isOtCompatibleExport(const StateHandler& stateHandler, bool hasSlices);
    static bool isMegabreakCompatibleExport(const StateHandler& stateHandler, bool hasSlices);
    void layoutTopSections();
    void layoutOtAttributesSection();
    void layoutChainExportSection();
    void layoutMegabreakExportSection();
    void updateExportButtonState();

    enum RadioGroupId
    {
        bitDepthGroupId = 1,
        channelGroupId = 2,
        sampleRateGroupId = 3
    };

    juce::ToggleButton bitDepth16Bit;
    juce::ToggleButton bitDepth24Bit;

    juce::ToggleButton channelMono;
    juce::ToggleButton channelStereo;

    juce::ToggleButton sampleRate48k;
    juce::ToggleButton sampleRate44k;

    SectionComponent bitDepthSection{"Bit depth"};
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
    juce::ComboBox megabreakFadeinBox;
    juce::ComboBox megabreakFadeoutBox;

    NumberInputComponent gainInput{"Gain:", StateHandler::gainValue.min, StateHandler::gainValue.max, StateHandler::gainValue.defaultValue, StateHandler::gainValue.stepSize, false};
    NumberInputComponent bpmInput{"BPM:", StateHandler::bpmValue.min, StateHandler::bpmValue.max, StateHandler::bpmValue.defaultValue, StateHandler::bpmValue.stepSize, false};

    juce::ToggleButton exportOtFile{"OT file"};
    juce::ToggleButton exportEvenGrid{"Even grid"};
    juce::ToggleButton exportEmbedMarkers{"Embed markers"};
    juce::TextButton createButton{"Save chain(s)"};
    juce::TextButton createMegabreakButton{"Save megabreak"};
    bool hasSlices = false;

    static void configureRadioButtons(StateHandler& stateHandler, const juce::Identifier& identifier,
                                      int groupId, std::initializer_list<juce::ToggleButton*> buttons);
    static void configureRadioButton(juce::ToggleButton& button, int groupId);
    void sendChainExportRequested();
    void sendMegabreakExportRequested();

    StateHandler& stateHandler;
    juce::ListenerList<Listener> listeners;
};
