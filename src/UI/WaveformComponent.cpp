#include "WaveformComponent.h"

WaveformComponent::WaveformComponent(const Dimension& height, const Dimension& width,
                                     const juce::String& title)
    : PanelComponent(height, width, title)
{
    formatManager.registerBasicFormats();
    thumbnail.addChangeListener(this);
    addAndMakeVisible(playHeadOverlay);
    playHeadOverlay.setVisible(false);
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
    playHeadOverlay.setVisible(true);
    repaint();
}

void WaveformComponent::clearAudioData()
{
    thumbnail.clear();
    thumbnailSourceBuffer.setSize(0, 0);
    playHeadOverlay.setVisible(false);
    repaint();
}

void WaveformComponent::setPlayHeadPositionFactor(const double newPlayHeadPositionFactor)
{
    playHeadOverlay.setPlayHeadPositionFactor(newPlayHeadPositionFactor);
}

void WaveformComponent::paint(juce::Graphics& g)
{
    PanelComponent::paint(g);
    drawWaveform(g);
}

void WaveformComponent::resized()
{
    PanelComponent::resized();
    playHeadOverlay.setBounds(innerBounds.reduced(4));
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

WaveformComponent::PlayHeadOverlayComponent::PlayHeadOverlayComponent()
{
    setInterceptsMouseClicks(false, false);
    setOpaque(false);
}

void WaveformComponent::PlayHeadOverlayComponent::setPlayHeadPositionFactor(double newPlayHeadPositionFactor)
{
    const auto clampedFactor = juce::jlimit(0.0, 1.0, newPlayHeadPositionFactor);
    if (clampedFactor == playHeadPositionFactor)
        return;

    playHeadPositionFactor = clampedFactor;
    repaint();
}

void WaveformComponent::PlayHeadOverlayComponent::paint(juce::Graphics& g)
{
    const auto overlayArea = getLocalBounds();
    if (overlayArea.isEmpty())
        return;

    const auto playHeadX = static_cast<int>(overlayArea.getX() + playHeadPositionFactor * overlayArea.getWidth());
    g.setColour(juce::Colour(StyleSheet::playHeadColour));
    g.drawVerticalLine(playHeadX, static_cast<float>(overlayArea.getY()), static_cast<float>(overlayArea.getBottom()));
}
