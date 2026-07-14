#include "WaveformComponent.h"

WaveformComponent::WaveformComponent(const Dimension& height, const Dimension& width,
                                     const juce::String& title)
    : PanelComponent(height, width, title)
{
    formatManager.registerBasicFormats();
    thumbnail.addChangeListener(this);
}

WaveformComponent::~WaveformComponent()
{
    thumbnail.removeChangeListener(this);
}

void WaveformComponent::setAudioData(const juce::AudioBuffer<float>& audioData, const double sampleRate)
{
    if (audioData.getNumChannels() <= 0 || audioData.getNumSamples() <= 0 || sampleRate <= 0.0)
    {
        clearAudioData();
        return;
    }

    thumbnailSourceBuffer.makeCopyOf(audioData);
    thumbnail.reset(thumbnailSourceBuffer.getNumChannels(), sampleRate, thumbnailSourceBuffer.getNumSamples());
    thumbnail.addBlock(0, thumbnailSourceBuffer, 0, thumbnailSourceBuffer.getNumSamples());
    repaint();
}

void WaveformComponent::clearAudioData()
{
    thumbnail.clear();
    thumbnailSourceBuffer.setSize(0, 0);
    repaint();
}

void WaveformComponent::paint(juce::Graphics& g)
{
    PanelComponent::paint(g);
    drawWaveform(g);
}

void WaveformComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &thumbnail)
        repaint();
}

void WaveformComponent::drawWaveform(juce::Graphics& g)
{
    if (thumbnail.getNumChannels() <= 0 || thumbnail.getTotalLength() <= 0.0)
        return;

    const auto waveformArea = innerBounds.reduced(4);
    if (waveformArea.isEmpty())
        return;

    g.setColour(juce::Colour(StyleSheet::controlBorderColour).withAlpha(0.55f));
    g.drawHorizontalLine(waveformArea.getCentreY(), static_cast<float>(waveformArea.getX()), static_cast<float>(waveformArea.getRight()));

    g.setColour(juce::Colour(StyleSheet::buttonBackgroundColour));
    thumbnail.drawChannels(g, waveformArea, 0.0, thumbnail.getTotalLength(), 1.0f);
}
