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

    waveformAudioClip.reset();
    waveformSourceBuffer.makeCopyOf(audioData);
    waveformSegments = segments;
    waveformSampleRate = sampleRate;
    selectedSegmentIndex = selectedSegmentIndexToUse;
    sliceStartSample = 0;
    sliceEndSample = audioData.getNumSamples();
    sliceRangeEnabled = false;
    activeSliceRangeHandle = SliceRangeHandle::none;
    playHeadOverlay.setSliceRange(0.0, 1.0);
    processing = false;
    processingMessage.clear();
    playHeadOverlay.setVisible(true);
    repaint();
}

void WaveformComponent::setAudioClip(std::shared_ptr<const AudioClip> audioClip,
                                     const std::vector<Chain::Segment>& segments,
                                     const int selectedSegmentIndexToUse)
{
    if (audioClip == nullptr || ! audioClip->isValid())
    {
        clearAudioData();
        return;
    }

    waveformAudioClip = std::move(audioClip);
    waveformSourceBuffer.setSize(0, 0);
    waveformSegments = segments;
    waveformSampleRate = waveformAudioClip->getSampleRate();
    selectedSegmentIndex = selectedSegmentIndexToUse;
    sliceStartSample = 0;
    sliceEndSample = waveformAudioClip->getAudioData().getNumSamples();
    sliceRangeEnabled = false;
    activeSliceRangeHandle = SliceRangeHandle::none;
    playHeadOverlay.setSliceRange(0.0, 1.0);
    processing = false;
    processingMessage.clear();
    playHeadOverlay.setVisible(true);
    repaint();
}

void WaveformComponent::setSliceRange(const int startSample, const int endSample)
{
    const auto* sourceBuffer = getSourceBuffer();
    const auto totalSamples = sourceBuffer != nullptr ? sourceBuffer->getNumSamples() : 0;
    if (totalSamples <= 0)
    {
        sliceRangeEnabled = false;
        sliceStartSample = 0;
        sliceEndSample = 0;
        activeSliceRangeHandle = SliceRangeHandle::none;
        repaint();
        return;
    }

    auto newStart = juce::jlimit(0, totalSamples, startSample);
    auto newEnd = juce::jlimit(0, totalSamples, endSample);
    const auto minimumRangeSamples = juce::jmin(getMinimumSliceRangeSamples(waveformSampleRate), totalSamples);
    const auto maxStart = juce::jmax(0, totalSamples - minimumRangeSamples);

    newStart = juce::jlimit(0, maxStart, newStart);
    newEnd = juce::jlimit(newStart + minimumRangeSamples, totalSamples, newEnd);

    const bool changed = ! sliceRangeEnabled
                         || sliceStartSample != newStart
                         || sliceEndSample != newEnd;

    sliceRangeEnabled = true;
    sliceStartSample = newStart;
    sliceEndSample = newEnd;
    playHeadOverlay.setSliceRange(static_cast<double>(sliceStartSample) / static_cast<double>(totalSamples),
                                  static_cast<double>(sliceEndSample) / static_cast<double>(totalSamples));

    if (changed)
        repaint();
}

void WaveformComponent::clearAudioData()
{
    waveformAudioClip.reset();
    waveformSourceBuffer.setSize(0, 0);
    waveformSegments.clear();
    waveformSampleRate = 0.0;
    selectedSegmentIndex = -1;
    sliceStartSample = 0;
    sliceEndSample = 0;
    sliceRangeEnabled = false;
    activeSliceRangeHandle = SliceRangeHandle::none;
    playHeadOverlay.setSliceRange(0.0, 1.0);
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
        waveformAudioClip.reset();
        waveformSourceBuffer.setSize(0, 0);
        waveformSegments.clear();
        waveformSampleRate = 0.0;
        selectedSegmentIndex = -1;
        sliceStartSample = 0;
        sliceEndSample = 0;
        sliceRangeEnabled = false;
        activeSliceRangeHandle = SliceRangeHandle::none;
        playHeadOverlay.setSliceRange(0.0, 1.0);
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

    if (sliceRangeEnabled)
    {
        const auto handle = getSliceRangeHandleAtPoint(event.getPosition());
        if (handle != static_cast<int>(SliceRangeHandle::none))
        {
            activeSliceRangeHandle = static_cast<SliceRangeHandle>(handle);
            updateSliceRangeFromMouse(event.getPosition());
            return;
        }
    }

    const auto segmentIndex = getSegmentIndexAtPoint(event.getPosition());
    if (segmentIndex < 0 || ! juce::isPositiveAndBelow(segmentIndex, static_cast<int>(waveformSegments.size())))
        return;

    const auto sliceIndex = waveformSegments[static_cast<size_t>(segmentIndex)].sliceIndex;
    if (sliceIndex < 0)
        return;

    sendWaveformSegmentClicked(segmentIndex, sliceIndex);
}

void WaveformComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (processing || activeSliceRangeHandle == SliceRangeHandle::none)
        return;

    updateSliceRangeFromMouse(event.getPosition());
}

void WaveformComponent::mouseUp(const juce::MouseEvent&)
{
    if (processing)
    {
        activeSliceRangeHandle = SliceRangeHandle::none;
        return;
    }

    if (activeSliceRangeHandle != SliceRangeHandle::none && sliceRangeEnabled)
        sendWaveformSliceRangeChanged(sliceStartSample, sliceEndSample);

    activeSliceRangeHandle = SliceRangeHandle::none;
}

void WaveformComponent::drawWaveform(juce::Graphics& g) const
{
    const auto* sourceBuffer = getSourceBuffer();
    if (sourceBuffer == nullptr || sourceBuffer->getNumChannels() <= 0 || sourceBuffer->getNumSamples() <= 0)
        return;

    const auto waveformArea = innerBounds.reduced(4);
    if (waveformArea.isEmpty())
        return;

    if (sliceRangeEnabled)
    {
        drawSliceRangeWaveform(g, waveformArea);
        return;
    }

    if (waveformSegments.size() <= 1 || waveformSampleRate <= 0.0)
    {
        drawChannelCenterlines(g, waveformArea);
        drawBufferWaveform(g, waveformArea, 0, sourceBuffer->getNumSamples(), StyleSheet::getWaveformColour());
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
    const auto* sourceBuffer = getSourceBuffer();
    const auto totalSamples = juce::jmax(1, sourceBuffer != nullptr ? sourceBuffer->getNumSamples() : 0);
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

void WaveformComponent::drawSliceRangeWaveform(juce::Graphics& g, const juce::Rectangle<int>& waveformArea) const
{
    const auto* sourceBuffer = getSourceBuffer();
    const auto totalSamples = sourceBuffer != nullptr ? sourceBuffer->getNumSamples() : 0;
    const auto numChannels = sourceBuffer != nullptr ? sourceBuffer->getNumChannels() : 0;
    if (totalSamples <= 0 || numChannels <= 0)
        return;

    const auto clampedStart = juce::jlimit(0, totalSamples, sliceStartSample);
    const auto clampedEnd = juce::jlimit(clampedStart, totalSamples, sliceEndSample);
    const auto selectionLength = clampedEnd - clampedStart;
    if (selectionLength <= 0)
        return;

    drawChannelCenterlines(g, waveformArea);

    const auto selectionStartX = sampleToX(clampedStart, waveformArea);
    const auto selectionEndX = sampleToX(clampedEnd, waveformArea);
    const auto selectionArea = waveformArea.withX(selectionStartX)
                                            .withWidth(juce::jmax(1, selectionEndX - selectionStartX));

    drawBufferWaveform(g, waveformArea, 0, totalSamples, StyleSheet::getSliceWaveformMutedColour());
    g.setColour(StyleSheet::getSliceWaveformSelectionColour());
    g.fillRect(selectionArea);
    drawBufferWaveform(g, selectionArea, clampedStart, selectionLength, StyleSheet::getSelectedWaveformColour());

    drawSliceRangeMarkers(g, waveformArea);
}

void WaveformComponent::drawSliceRangeMarkers(juce::Graphics& g, const juce::Rectangle<int>& waveformArea) const
{
    const auto* sourceBuffer = getSourceBuffer();
    const auto totalSamples = sourceBuffer != nullptr ? sourceBuffer->getNumSamples() : 0;
    if (totalSamples <= 0 || waveformArea.isEmpty())
        return;

    const auto startX = sampleToX(sliceStartSample, waveformArea);
    const auto endX = sampleToX(sliceEndSample, waveformArea);
    const auto markerColour = StyleSheet::getSliceWaveformMarkerColour();
    const auto handleColour = StyleSheet::getSliceWaveformMarkerHandleColour();
    constexpr auto handleWidth = 8;
    const auto handleHeight = juce::jmin(18, juce::jmax(10, waveformArea.getHeight() / 2));
    const auto topY = waveformArea.getY();
    const auto bottomY = waveformArea.getBottom();

    auto drawMarker = [&](const int x, const bool isStartMarker)
    {
        g.setColour(markerColour);
        g.drawVerticalLine(x, static_cast<float>(topY), static_cast<float>(bottomY));

        const auto handleRect = juce::Rectangle<int>(isStartMarker ? x : x - handleWidth + 1,
                                                     isStartMarker ? topY : bottomY - handleHeight,
                                                     handleWidth,
                                                     handleHeight);
        g.setColour(handleColour);
        g.fillRoundedRectangle(handleRect.toFloat(), 2.0f);
        g.setColour(markerColour);
        g.drawRoundedRectangle(handleRect.toFloat(), 2.0f, 1.0f);
    };

    drawMarker(startX, true);
    drawMarker(endX, false);
}

void WaveformComponent::drawBufferWaveform(juce::Graphics& g, const juce::Rectangle<int>& waveformArea,
                                           const int startSample, const int sampleCount,
                                           const juce::Colour waveformColour) const
{
    const auto* sourceBuffer = getSourceBuffer();
    const auto totalSamples = sourceBuffer != nullptr ? sourceBuffer->getNumSamples() : 0;
    const auto numChannels = sourceBuffer != nullptr ? sourceBuffer->getNumChannels() : 0;
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

        const auto* samples = sourceBuffer->getReadPointer(channel);
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

void WaveformComponent::updateSliceRangeFromMouse(const juce::Point<int>& position)
{
    if (activeSliceRangeHandle == SliceRangeHandle::none)
        return;

    const auto waveformArea = innerBounds.reduced(4);
    if (waveformArea.isEmpty())
        return;

    const auto* sourceBuffer = getSourceBuffer();
    const auto totalSamples = sourceBuffer != nullptr ? sourceBuffer->getNumSamples() : 0;
    if (totalSamples <= 0)
        return;

    const auto newSample = xToSample(position.x, waveformArea);
    const auto minimumRangeSamples = juce::jmin(getMinimumSliceRangeSamples(waveformSampleRate), totalSamples);
    if (activeSliceRangeHandle == SliceRangeHandle::start)
    {
        const auto maxStart = juce::jmax(0, sliceEndSample - minimumRangeSamples);
        const auto clampedStart = juce::jlimit(0, maxStart, newSample);
        if (clampedStart == sliceStartSample)
            return;

        sliceStartSample = clampedStart;
        repaint();
    }
    else if (activeSliceRangeHandle == SliceRangeHandle::end)
    {
        const auto minEnd = juce::jmin(totalSamples, sliceStartSample + minimumRangeSamples);
        const auto clampedEnd = juce::jlimit(minEnd, totalSamples, newSample);
        if (clampedEnd == sliceEndSample)
            return;

        sliceEndSample = clampedEnd;
        repaint();
    }
}

void WaveformComponent::drawChannelCenterlines(juce::Graphics& g, const juce::Rectangle<int>& waveformArea) const
{
    const auto* sourceBuffer = getSourceBuffer();
    const auto numChannels = sourceBuffer != nullptr ? sourceBuffer->getNumChannels() : 0;
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

int WaveformComponent::sampleToX(const int sample, const juce::Rectangle<int>& waveformArea) const
{
    const auto* sourceBuffer = getSourceBuffer();
    const auto totalSamples = juce::jmax(1, sourceBuffer != nullptr ? sourceBuffer->getNumSamples() : 0);
    return waveformArea.getX() + juce::roundToInt((static_cast<double>(juce::jlimit(0, totalSamples, sample))
                                                   / static_cast<double>(totalSamples))
                                                  * static_cast<double>(waveformArea.getWidth()));
}

int WaveformComponent::xToSample(const int x, const juce::Rectangle<int>& waveformArea) const
{
    const auto* sourceBuffer = getSourceBuffer();
    const auto totalSamples = juce::jmax(1, sourceBuffer != nullptr ? sourceBuffer->getNumSamples() : 0);
    const auto clampedX = juce::jlimit(waveformArea.getX(), waveformArea.getRight(), x);
    const auto relativeX = clampedX - waveformArea.getX();
    return juce::jlimit(0, totalSamples,
                        juce::roundToInt((static_cast<double>(relativeX)
                                          / static_cast<double>(juce::jmax(1, waveformArea.getWidth())))
                                         * static_cast<double>(totalSamples)));
}

const juce::AudioBuffer<float>* WaveformComponent::getSourceBuffer() const
{
    if (waveformAudioClip != nullptr)
        return &waveformAudioClip->getAudioData();

    if (waveformSourceBuffer.getNumChannels() > 0 && waveformSourceBuffer.getNumSamples() > 0)
        return &waveformSourceBuffer;

    return nullptr;
}

int WaveformComponent::getSegmentIndexAtPoint(const juce::Point<int> position) const
{
    const auto* sourceBuffer = getSourceBuffer();
    if (waveformSegments.empty() || sourceBuffer == nullptr || sourceBuffer->getNumSamples() <= 0)
        return -1;

    const auto waveformArea = innerBounds.reduced(4);
    if (! waveformArea.contains(position))
        return -1;

    for (size_t index = 0; index < waveformSegments.size(); ++index)
    {
        const auto& segment = waveformSegments[index];
        if (segment.sampleCount <= 0)
            continue;

        const auto totalSamples = juce::jmax(1, sourceBuffer->getNumSamples());
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

int WaveformComponent::getSliceRangeHandleAtPoint(const juce::Point<int> position) const
{
    const auto* sourceBuffer = getSourceBuffer();
    if (! sliceRangeEnabled || sourceBuffer == nullptr || sourceBuffer->getNumSamples() <= 0)
        return static_cast<int>(SliceRangeHandle::none);

    const auto waveformArea = innerBounds.reduced(4);
    if (! waveformArea.contains(position))
        return static_cast<int>(SliceRangeHandle::none);

    const auto startX = sampleToX(sliceStartSample, waveformArea);
    const auto endX = sampleToX(sliceEndSample, waveformArea);
    const auto hitDistance = 6;

    if (std::abs(position.x - startX) <= hitDistance)
        return static_cast<int>(SliceRangeHandle::start);

    if (std::abs(position.x - endX) <= hitDistance)
        return static_cast<int>(SliceRangeHandle::end);

    return static_cast<int>(SliceRangeHandle::none);
}

void WaveformComponent::sendWaveformSliceRangeChanged(const int startSample, const int endSample)
{
    listeners.call([startSample, endSample](Listener& listener)
    {
        listener.waveformSliceRangeChanged(startSample, endSample);
    });
}

void WaveformComponent::sendWaveformSegmentClicked(const int segmentIndex, const int sliceIndex)
{
    listeners.call([segmentIndex, sliceIndex](Listener& listener)
    {
        listener.waveformSegmentClicked(segmentIndex, sliceIndex);
    });
}

int WaveformComponent::getMinimumSliceRangeSamples(const double sampleRate)
{
    return juce::jmax(1, juce::roundToInt(sampleRate * minimumSliceRangeSeconds));
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

void WaveformComponent::PlayHeadOverlayComponent::setSliceRange(const double newSliceStartFactor,
                                                                const double newSliceEndFactor)
{
    const auto clampedStart = juce::jlimit(0.0, 1.0, newSliceStartFactor);
    const auto clampedEnd = juce::jlimit(clampedStart, 1.0, newSliceEndFactor);
    if (clampedStart == sliceStartFactor && clampedEnd == sliceEndFactor)
        return;

    sliceStartFactor = clampedStart;
    sliceEndFactor = clampedEnd;
    repaint();
}

void WaveformComponent::PlayHeadOverlayComponent::paint(juce::Graphics& g)
{
    const auto overlayArea = getLocalBounds();
    if (overlayArea.isEmpty())
        return;

    const auto rangeWidth = juce::jmax(0.0, sliceEndFactor - sliceStartFactor);
    const auto mappedFactor = rangeWidth > 0.0
        ? sliceStartFactor + playHeadPositionFactor * rangeWidth
        : playHeadPositionFactor;
    const auto playHeadX = static_cast<int>(overlayArea.getX() + mappedFactor * overlayArea.getWidth());
    g.setColour(juce::Colour(StyleSheet::playHeadColour));
    g.drawVerticalLine(playHeadX, static_cast<float>(overlayArea.getY()), static_cast<float>(overlayArea.getBottom()));
}
