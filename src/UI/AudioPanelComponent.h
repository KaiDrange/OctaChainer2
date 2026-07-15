#pragma once

#include <JuceHeader.h>

#include "PanelComponent.h"
#include "TransportButtonComponent.h"


class AudioPanelComponent : public PanelComponent
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

    AudioPanelComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                           const juce::String& title = "");
    void resized() override;

private:
    TransportButtonComponent btnPlaySlice{"Slice"};
    TransportButtonComponent btnPlayChain{"Chain"};
    juce::Slider masterVolumeSlider;
    juce::Label masterVolumeLabel;

    void sendTransportEvent(TransportButtonComponent::TransportEvent event);
    juce::ListenerList<Listener> listeners;
};
