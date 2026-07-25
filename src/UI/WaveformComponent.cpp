#include "WaveformComponent.h"

WaveformComponent::WaveformComponent(const Dimension& height, const Dimension& width,
                                     const juce::String& title)
    : PanelComponent(height, width, title)
{
    addAndMakeVisible(playHeadOverlay);
    playHeadOverlay.setVisible(false);
}

WaveformComponent::~WaveformComponent() = default;

void WaveformComponent::addListener(Listener* listener)
{
    listeners.add(listener);
}

void WaveformComponent::removeListener(Listener* listenerToRemove)
{
    jassert(listeners.contains(listenerToRemove));
    listeners.remove(listenerToRemove);
}

void WaveformComponent::setAudioData(const juce::AudioBuffer<float>& audioData, const double sampleRate)
{
    setAudioData(audioData, sampleRate, {}, -1);
}

void WaveformComponent::setAudioData(const juce::AudioBuffer<float>& audioData, const double sampleRate,
                                     const std::vector<Chain::Segment>& segments,
                                     const int selectedSegmentIndexToUse)
{
    if (audioData.getNumChannels() <= 0 || audioData.getNumSamples() <= 0 || sampleRate <= 0.0)
    {
        clearAudioData();
        return;
    }

    waveformSourceBuffer.makeCopyOf(audioData);
    waveformSegments = segments;
    waveformSampleRate = sampleRate;
    selectedSegmentIndex = selectedSegmentIndexToUse;
    processing = false;
    processingMessage.clear();
    playHeadOverlay.setVisible(true);
    repaint();
}

void WaveformComponent::clearAudioData()
{
    waveformSourceBuffer.setSize(0, 0);
    waveformSegments.clear();
    waveformSampleRate = 0.0;
    selectedSegmentIndex = -1;
    processing = false;
    processingMessage.clear();
    playHeadOverlay.setVisible(false);
    repaint();
}

void WaveformComponent::setProcessingState(const bool isProcessing, juce::String message)
{
    processing = isProcessing;
    processingMessage = std::move(message);

    if (processing)
    {
        waveformSourceBuffer.setSize(0, 0);
        waveformSegments.clear();
        waveformSampleRate = 0.0;
        selectedSegmentIndex = -1;
        playHeadOverlay.setVisible(false);
    }

    repaint();
}

void WaveformComponent::setSelectedSegmentIndex(const int newSelectedSegmentIndex)
{
    if (selectedSegmentIndex == newSelectedSegmentIndex)
        return;

    selectedSegmentIndex = newSelectedSegmentIndex;
    repaint();
}

void WaveformComponent::setPlayHeadPositionFactor(const double newPlayHeadPositionFactor)
{
    playHeadOverlay.setPlayHeadPositionFactor(newPlayHeadPositionFactor);
}

void WaveformComponent::paint(juce::Graphics& g)
{
    PanelComponent::paint(g);

    if (processing)
    {
        const auto area = innerBounds.reduced(4);
        if (!area.isEmpty())
        {
            g.setColour(juce::Colour(StyleSheet::textDefaultColour).withAlpha(0.72f));
            g.setFont(StyleSheet::getTitleFont());
            g.drawFittedText(processingMessage.isEmpty() ? "Processing..." : processingMessage,
                             area,
                             juce::Justification::centred,
                             1);
        }
        return;
    }

    drawWaveform(g);
}

void WaveformComponent::resized()
{
    PanelComponent::resized();
    playHeadOverlay.setBounds(innerBounds.reduced(4));
}

void WaveformComponent::mouseDown(const juce::MouseEvent& event)
{
    if (processing)
        return;

    const auto segmentIndex = getSegmentIndexAtPoint(event.getPosition());
    if (segmentIndex < 0 || ! juce::isPositiveAndBelow(segmentIndex, static_cast<int>(waveformSegments.size())))
        return;

    const auto sliceIndex = waveformSegments[static_cast<size_t>(segmentIndex)].sliceIndex;
    if (sliceIndex < 0)
        return;

    sendWaveformSegmentClicked(segmentIndex, sliceIndex);
}

void WaveformComponent::drawWaveform(juce::Graphics& g) const
{
    if (waveformSourceBuffer.getNumChannels() <= 0 || waveformSourceBuffer.getNumSamples() <= 0)
        return;

    const auto waveformArea = innerBounds.reduced(4);
    if (waveformArea.isEmpty())
        return;

    if (waveformSegments.size() <= 1 || waveformSampleRate <= 0.0)
    {
        drawChannelCenterlines(g, waveformArea);
        drawBufferWaveform(g, waveformArea, 0, waveformSourceBuffer.getNumSamples(), StyleSheet::getWaveformColour());
        return;
    }

    for (size_t index = 0; index < waveformSegments.size(); ++index)
    {
        const auto& segment = waveformSegments[index];
        if (segment.sampleCount <= 0)
            continue;

        const auto isSelectedSegment = segment.sliceIndex == selectedSegmentIndex;
        drawSegmentWaveform(g, waveformArea, static_cast<int>(index), segment.startSample, segment.sampleCount,
                            isSelectedSegment);
    }
}

void WaveformComponent::drawSegmentWaveform(juce::Graphics& g, const juce::Rectangle<int>& waveformArea,
                                            const int segmentIndex, const int segmentStartSample,
                                            const int segmentSampleCount, const bool isSelectedSegment) const
{
    const auto totalSamples = juce::jmax(1, waveformSourceBuffer.getNumSamples());
    const auto segmentStart = juce::jlimit(0, totalSamples, segmentStartSample);
    const auto segmentEnd = juce::jlimit(segmentStart, totalSamples, segmentStartSample + segmentSampleCount);
    const auto clampedSampleCount = segmentEnd - segmentStart;
    if (clampedSampleCount <= 0)
        return;

    const auto startX = waveformArea.getX() + juce::roundToInt((static_cast<double>(segmentStart)
                                                                / static_cast<double>(totalSamples))
                                                               * static_cast<double>(waveformArea.getWidth()));
    const auto endX = (segmentIndex == static_cast<int>(waveformSegments.size()) - 1)
        ? waveformArea.getRight()
        : waveformArea.getX() + juce::roundToInt((static_cast<double>(segmentEnd)
                                                  / static_cast<double>(totalSamples))
                                                 * static_cast<double>(waveformArea.getWidth()));
    const auto segmentWidth = juce::jmax(1, endX - startX);
    const auto segmentArea = waveformArea.withX(startX).withWidth(segmentWidth);

    const auto waveformColour = isSelectedSegment
        ? StyleSheet::getSelectedWaveformColour()
        : (segmentIndex % 2 == 0 ? StyleSheet::getWaveformColour() : StyleSheet::getWaveformAltColour());
    const auto segmentBackgroundColour = isSelectedSegment
        ? StyleSheet::getSelectedWaveformBackgroundColour()
        : (segmentIndex % 2 == 0 ? StyleSheet::getWaveformBackgroundColour()
                                  : StyleSheet::getWaveformAltBackgroundColour());

    g.setColour(segmentBackgroundColour);
    g.fillRect(segmentArea);

    drawChannelCenterlines(g, segmentArea);

    g.setColour(waveformColour);
    drawBufferWaveform(g, segmentArea, segmentStart, clampedSampleCount, waveformColour);
}

void WaveformComponent::drawBufferWaveform(juce::Graphics& g, const juce::Rectangle<int>& waveformArea,
                                           const int startSample, const int sampleCount,
                                           const juce::Colour waveformColour) const
{
    const auto totalSamples = waveformSourceBuffer.getNumSamples();
    const auto numChannels = waveformSourceBuffer.getNumChannels();
    if (waveformArea.isEmpty() || totalSamples <= 0 || numChannels <= 0 || sampleCount <= 0)
        return;

    const auto clampedStart = juce::jlimit(0, totalSamples, startSample);
    const auto clampedEnd = juce::jlimit(clampedStart, totalSamples, startSample + sampleCount);
    const auto clampedSampleCount = clampedEnd - clampedStart;
    if (clampedSampleCount <= 0)
        return;

    g.setColour(waveformColour);

    const auto width = waveformArea.getWidth();
    if (width <= 0)
        return;

    for (int channel = 0; channel < numChannels; ++channel)
    {
        const auto channelTop = waveformArea.getY() + juce::roundToInt(static_cast<double>(channel)
                                                                       * waveformArea.getHeight()
                                                                       / static_cast<double>(numChannels));
        const auto channelBottom = waveformArea.getY() + juce::roundToInt(static_cast<double>(channel + 1)
                                                                          * waveformArea.getHeight()
                                                                          / static_cast<double>(numChannels));
        const auto channelArea = waveformArea.withY(channelTop).withBottom(channelBottom);
        if (channelArea.getHeight() <= 0)
            continue;

        const auto* samples = waveformSourceBuffer.getReadPointer(channel);
        const auto centreY = static_cast<float>(channelArea.getCentreY());
        const auto verticalScale = juce::jmax(1.0f, static_cast<float>(channelArea.getHeight()) * 0.5f - 2.0f);

        for (int x = 0; x < width; ++x)
        {
            const auto pixelStart = clampedStart + static_cast<int>((static_cast<int64>(x) * clampedSampleCount) / width);
            auto pixelEnd = clampedStart + static_cast<int>((static_cast<int64>(x + 1) * clampedSampleCount) / width);
            pixelEnd = juce::jlimit(pixelStart + 1, clampedEnd, pixelEnd);

            auto minSample = samples[pixelStart];
            auto maxSample = minSample;

            for (int sample = pixelStart + 1; sample < pixelEnd; ++sample)
            {
                const auto value = samples[sample];
                minSample = juce::jmin(minSample, value);
                maxSample = juce::jmax(maxSample, value);
            }

            const auto yTop = centreY - juce::jlimit(-1.0f, 1.0f, maxSample) * verticalScale;
            const auto yBottom = centreY - juce::jlimit(-1.0f, 1.0f, minSample) * verticalScale;
            const auto drawX = static_cast<float>(waveformArea.getX() + x);
            g.drawVerticalLine(juce::roundToInt(drawX), yTop, yBottom);
        }
    }
}

void WaveformComponent::drawChannelCenterlines(juce::Graphics& g, const juce::Rectangle<int>& waveformArea) const
{
    const auto numChannels = waveformSourceBuffer.getNumChannels();
    if (waveformArea.isEmpty() || numChannels <= 0)
        return;

    g.setColour(juce::Colour(StyleSheet::waveformColour).withAlpha(0.55f));

    for (int channel = 0; channel < numChannels; ++channel)
    {
        const auto channelTop = waveformArea.getY() + juce::roundToInt(static_cast<double>(channel)
                                                                       * waveformArea.getHeight()
                                                                       / static_cast<double>(numChannels));
        const auto channelBottom = waveformArea.getY() + juce::roundToInt(static_cast<double>(channel + 1)
                                                                          * waveformArea.getHeight()
                                                                          / static_cast<double>(numChannels));
        const auto centreY = channelTop + (channelBottom - channelTop) / 2;
        g.drawHorizontalLine(centreY, static_cast<float>(waveformArea.getX()), static_cast<float>(waveformArea.getRight()));
    }
}

int WaveformComponent::getSegmentIndexAtPoint(const juce::Point<int> position) const
{
    if (waveformSegments.empty() || waveformSourceBuffer.getNumSamples() <= 0)
        return -1;

    const auto waveformArea = innerBounds.reduced(4);
    if (! waveformArea.contains(position))
        return -1;

    for (size_t index = 0; index < waveformSegments.size(); ++index)
    {
        const auto& segment = waveformSegments[index];
        if (segment.sampleCount <= 0)
            continue;

        const auto totalSamples = juce::jmax(1, waveformSourceBuffer.getNumSamples());
        const auto segmentStart = juce::jlimit(0, totalSamples, segment.startSample);
        const auto segmentEnd = juce::jlimit(segmentStart, totalSamples, segment.startSample + segment.sampleCount);
        if (segmentEnd <= segmentStart)
            continue;

        const auto startX = waveformArea.getX() + juce::roundToInt((static_cast<double>(segmentStart)
                                                                    / static_cast<double>(totalSamples))
                                                                   * static_cast<double>(waveformArea.getWidth()));
        const auto endX = (index == waveformSegments.size() - 1)
            ? waveformArea.getRight()
            : waveformArea.getX() + juce::roundToInt((static_cast<double>(segmentEnd)
                                                      / static_cast<double>(totalSamples))
                                                     * static_cast<double>(waveformArea.getWidth()));

        if (position.x >= startX && position.x < endX)
            return static_cast<int>(index);
    }

    return -1;
}

void WaveformComponent::sendWaveformSegmentClicked(const int segmentIndex, const int sliceIndex)
{
    listeners.call([segmentIndex, sliceIndex](Listener& listener)
    {
        listener.waveformSegmentClicked(segmentIndex, sliceIndex);
    });
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
