#include "WaveformComponent.h"

WaveformComponent::WaveformComponent(const PanelComponent::Dimension& height, const PanelComponent::Dimension& width,
                                     const juce::String& title, StateHandler& stateHandlerToUse)
    : PanelComponent(height, width, title),
      stateHandler(&stateHandlerToUse)
{
    stateHandler->addListener(this);
}

WaveformComponent::~WaveformComponent()
{
    if (stateHandler != nullptr)
        stateHandler->removeListener(this);
}

void WaveformComponent::paint(juce::Graphics& g)
{
    PanelComponent::paint(g);
    drawSelectedSliceWaveform(g);
}

void WaveformComponent::stateChanged()
{
    repaint();
}

void WaveformComponent::drawSelectedSliceWaveform(juce::Graphics& g) const
{
    if (stateHandler == nullptr)
        return;

    const auto sliceTree = stateHandler->getSelectedSliceTree();
    if (! sliceTree.isValid())
        return;

    const auto numChannels = static_cast<int>(sliceTree.getProperty(stateHandler->sliceChannelsId, 0));
    const auto numSamples = static_cast<juce::int64>(sliceTree.getProperty(stateHandler->sliceNumSamplesId, static_cast<juce::int64>(0)));
    const auto* audioDataValue = sliceTree.getPropertyPointer(stateHandler->sliceAudioDataId);

    if (numChannels <= 0 || numSamples <= 0 || audioDataValue == nullptr)
        return;

    const auto* audioDataBlock = audioDataValue->getBinaryData();
    if (audioDataBlock == nullptr)
        return;

    const auto samplesPerChannel = static_cast<size_t>(numSamples);
    const auto expectedBytes = static_cast<size_t>(numChannels) * samplesPerChannel * sizeof(float);
    if (audioDataBlock->getSize() < expectedBytes)
        return;

    auto waveformArea = innerBounds.reduced(4);
    if (waveformArea.isEmpty())
        return;

    const auto* samples = static_cast<const float*>(audioDataBlock->getData());
    const auto width = waveformArea.getWidth();
    const auto centreY = static_cast<float>(waveformArea.getCentreY());
    const auto halfHeight = static_cast<float>(waveformArea.getHeight()) * 0.48f;

    g.setColour(juce::Colour(StyleSheet::controlBorderColour).withAlpha(0.55f));
    g.drawHorizontalLine(waveformArea.getCentreY(), static_cast<float>(waveformArea.getX()), static_cast<float>(waveformArea.getRight()));

    g.setColour(juce::Colour(StyleSheet::buttonBackgroundColour));

    for (int x = 0; x < width; ++x)
    {
        const auto sampleStart = static_cast<juce::int64>(x) * numSamples / width;
        auto sampleEnd = static_cast<juce::int64>(x + 1) * numSamples / width;

        if (sampleEnd <= sampleStart)
            sampleEnd = sampleStart + 1;

        float minValue = 1.0f;
        float maxValue = -1.0f;

        for (auto sample = sampleStart; sample < sampleEnd && sample < numSamples; ++sample)
        {
            const auto sampleIndex = static_cast<size_t>(sample);

            for (int channel = 0; channel < numChannels; ++channel)
            {
                const auto value = samples[static_cast<size_t>(channel) * samplesPerChannel + sampleIndex];
                minValue = juce::jmin(minValue, value);
                maxValue = juce::jmax(maxValue, value);
            }
        }

        const auto y1 = centreY - juce::jlimit(-1.0f, 1.0f, maxValue) * halfHeight;
        const auto y2 = centreY - juce::jlimit(-1.0f, 1.0f, minValue) * halfHeight;
        g.drawVerticalLine(waveformArea.getX() + x, y1, y2);
    }
}
