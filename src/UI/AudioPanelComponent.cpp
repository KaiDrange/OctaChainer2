#include "AudioPanelComponent.h"
#include "../Core/AudioPlaybackEngine.h"

AudioPanelComponent::AudioPanelComponent(const Dimension& height, const Dimension& width,
    StateHandler& stateHandlerToUse, const juce::String& title)
    : PanelComponent(height, width, title), stateHandler(stateHandlerToUse)
{
    addAndMakeVisible(btnPlaySlice);
    addAndMakeVisible(btnPlayChain);
    addAndMakeVisible(masterVolumeLabel);
    addAndMakeVisible(masterVolumeSlider);

    masterVolumeLabel.setText("Master Volume", juce::dontSendNotification);
    masterVolumeLabel.setJustificationType(juce::Justification::centred);
    masterVolumeLabel.setInterceptsMouseClicks(false, false);

    masterVolumeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    masterVolumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    masterVolumeSlider.setRange(-24.0f, 0.0f, 0.1f);

    masterVolumeSlider.onValueChange = [this]
    {
        stateHandler.setStateValue(stateHandler.masterVolumeId, juce::Decibels::decibelsToGain(masterVolumeSlider.getValue()));
    };

    btnPlaySlice.getButton().onClick = [this]
    {
        btnPlayChain.getButton().setToggleState(false, juce::dontSendNotification);
        const bool isPlaying = !btnPlaySlice.getButton().getToggleState();
        sendTransportEvent(isPlaying ? TransportButtonComponent::TransportEvent::Stop : TransportButtonComponent::TransportEvent::PlaySlice);
    };
    btnPlayChain.getButton().onClick = [this]
    {
        btnPlaySlice.getButton().setToggleState(false, juce::dontSendNotification);
        const bool isPlaying = !btnPlayChain.getButton().getToggleState();
        sendTransportEvent(isPlaying ? TransportButtonComponent::TransportEvent::Stop : TransportButtonComponent::TransportEvent::PlayChain);
    };

    stateHandler.addListener(this);
    stateChanged({ StateHandler::StateChange::fullReload });
}

AudioPanelComponent::~AudioPanelComponent()
{
    stateHandler.removeListener(this);
}

void AudioPanelComponent::resized()
{
    PanelComponent::resized();

    const auto height = innerBounds.getHeight();
    setVisible(height > 40);

    const auto hideLabels = height < 80;
    btnPlaySlice.showLabel = !hideLabels;
    btnPlayChain.showLabel = !hideLabels;
    masterVolumeLabel.setVisible(!hideLabels);

    const auto area = innerBounds.reduced(StyleSheet::controlGap);
    constexpr int labelHeight = static_cast<int>(StyleSheet::fontDefaultSize);

    auto columns = area;
    const auto columnWidth = columns.getWidth() / 3;
    const auto sliceColumn = columns.removeFromLeft(columnWidth);
    const auto chainColumn = columns.removeFromLeft(columnWidth);
    auto volumeColumn = columns;
    btnPlaySlice.setBounds(sliceColumn);
    btnPlayChain.setBounds(chainColumn);

    if (!hideLabels)
    {
        const auto volumeLabelArea = volumeColumn.removeFromTop(labelHeight);
        masterVolumeLabel.setBounds(volumeLabelArea);
        volumeColumn.removeFromTop(StyleSheet::controlGap);
    }
    const auto sliderSize = juce::jmax(0, juce::jmin(volumeColumn.getWidth(), volumeColumn.getHeight()));
    masterVolumeSlider.setBounds(volumeColumn.withSizeKeepingCentre(sliderSize, sliderSize));
}

void AudioPanelComponent::sendTransportEvent(TransportButtonComponent::TransportEvent event)
{
    listeners.call([event](Listener& l)
    {
        l.transportButtonPressed(event);
    });
}

void AudioPanelComponent::addListener(Listener* listener) {
    listeners.add(listener);
}

void AudioPanelComponent::removeListener(Listener* listenerToRemove) {
    jassert(listeners.contains(listenerToRemove));
    listeners.remove(listenerToRemove);
}

void AudioPanelComponent::stateChanged(const StateHandler::StateChange& change)
{
    if (change.has(StateHandler::StateChange::sliceList)
        || change.has(StateHandler::StateChange::selectedSlice)
        || change.has(StateHandler::StateChange::fullReload))
    {
        btnPlaySlice.setEnabled(stateHandler.getSelectedSliceIndex() >= 0);
        btnPlayChain.setEnabled(stateHandler.getNumSlices() > 0);
    }

    if ((change.has(StateHandler::StateChange::settings) && change.isSetting(stateHandler.masterVolumeId))
        || change.has(StateHandler::StateChange::fullReload))
    {
        const auto volume = juce::Decibels::gainToDecibels(stateHandler.getStateValue<float>(stateHandler.masterVolumeId, 0.5f));
        if (volume != masterVolumeSlider.getValue())
            masterVolumeSlider.setValue(volume, juce::dontSendNotification);
    }
}

void AudioPanelComponent::actionListenerCallback(const juce::String& message)
{
    if (message == AudioPlaybackEngine::playbackStoppedMessage)
    {
        btnPlaySlice.getButton().setToggleState(false, juce::dontSendNotification);
        btnPlayChain.getButton().setToggleState(false, juce::dontSendNotification);
    }
}
