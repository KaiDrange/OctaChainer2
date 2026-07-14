#pragma once

#include "../Core/StateHandler.h"
#include "PanelComponent.h"


class WaveformComponent : public PanelComponent,
                          private StateHandler::Listener
{
public:
    WaveformComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                      const juce::String& title, StateHandler& stateHandlerToUse);
    ~WaveformComponent() override;
    void stateChanged() override;
    void paint(juce::Graphics& g) override;

private:
    void drawSelectedSliceWaveform(juce::Graphics& g) const;

    StateHandler* stateHandler = nullptr;
};
