#include "PluginEditor.h"

#include "Core/AudioUtil.h"

OctaChainer2AudioProcessorEditor::OctaChainer2AudioProcessorEditor(OctaChainer2AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    mainComponent = std::make_unique<MainComponent>(audioProcessor.getStateHandler(), audioProcessor.getAudioPlaybackEngine());
    addAndMakeVisible(*mainComponent);
    mainComponent->addListener(this);
    setResizable(true, true);
    setResizeLimits(MainComponent::minWidth, MainComponent::minHeight, MainComponent::maxWidth,
                    MainComponent::maxHeight);
    setSize(MainComponent::defaultWidth, MainComponent::defaultHeight);
    mainComponent->setBounds(getLocalBounds());
}

OctaChainer2AudioProcessorEditor::~OctaChainer2AudioProcessorEditor()
{
    if (mainComponent != nullptr)
    {
        mainComponent->detachPlaybackListener();
        mainComponent->removeListener(this);
    }
    audioProcessor.editorDestroyed();
}

void OctaChainer2AudioProcessorEditor::resized()
{
    mainComponent->setBounds(getLocalBounds());
}

void OctaChainer2AudioProcessorEditor::transportButtonPressed(const TransportButtonComponent::TransportEvent event)
{
    if (event == TransportButtonComponent::TransportEvent::PlayChain)
    {
        DBG("PlayChain transport is not implemented yet");
    }
    else if (event == TransportButtonComponent::TransportEvent::PlaySlice)
    {
        juce::AudioBuffer<float> audioData;
        double sampleRate = 0.0;

        if (audioProcessor.getStateHandler().loadSelectedSliceAudio(audioData, sampleRate))
        {
            const auto renderedClip = AudioUtil::renderPlaybackClip(
                AudioClip(std::move(audioData), sampleRate),
                audioProcessor.getAudioPlaybackEngine().deviceSampleRate,
                audioProcessor.getAudioPlaybackEngine().deviceChannelCount);
            audioProcessor.getAudioPlaybackEngine().play(renderedClip);
        }
        else
        {
            DBG("No selected slice audio is available for playback");
            audioProcessor.getAudioPlaybackEngine().stop();
        }
    }
    else if (event == TransportButtonComponent::TransportEvent::Stop)
    {
        audioProcessor.getAudioPlaybackEngine().stop();
    }
}

