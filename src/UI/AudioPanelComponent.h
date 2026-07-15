#pragma once

#include <JuceHeader.h>

#include "PanelComponent.h"
#include "TransportButtonComponent.h"

class AudioPanelComponent : public PanelComponent, public juce::ActionListener
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

    void actionListenerCallback (const juce::String& message) override;

    AudioPanelComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                        const juce::String& title = "");
    void resized() override;

    TransportButtonComponent btnPlaySlice{"Slice"};
    TransportButtonComponent btnPlayChain{"Chain"};

private:
    juce::Slider masterVolumeSlider;
    juce::Label masterVolumeLabel;

    void sendTransportEvent(TransportButtonComponent::TransportEvent event);
    juce::ListenerList<Listener> listeners;
};
