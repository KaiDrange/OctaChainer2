#pragma once

#include <JuceHeader.h>
#include "PanelComponent.h"
#include "TransportButtonComponent.h"
#include "../Core/StateHandler.h"

class StateHandler;

class AudioPanelComponent : public PanelComponent, public juce::ActionListener, StateHandler::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void transportButtonPressed(TransportButtonComponent::TransportEvent event) = 0;
    };

    void addListener(Listener* listener);
    void removeListener(Listener* listenerToRemove);
    void setChainReady(bool isReady);
    ~AudioPanelComponent() override;
    void stateChanged(const StateHandler::StateChange& change) override;

    void actionListenerCallback(const juce::String& message) override;

    AudioPanelComponent(const Dimension& height, const Dimension& width,
        StateHandler& stateHandlerToUse, const juce::String& panelTitle = "");
    void resized() override;

    TransportButtonComponent btnPlaySlice{"Slice"};
    TransportButtonComponent btnPlayChain{"Chain"};

private:
    void updateChainTransportEnabled();

    StateHandler& stateHandler;
    juce::Slider masterVolumeSlider;
    juce::Label masterVolumeLabel;
    bool hasSlices = false;
    bool chainReady = false;

    void sendTransportEvent(TransportButtonComponent::TransportEvent event);
    juce::ListenerList<Listener> listeners;
    juce::Font labelFont{juce::FontOptions(StyleSheet::audioPanelFontSize)};

};
