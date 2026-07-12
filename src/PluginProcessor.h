#pragma once

#include <JuceHeader.h>

#include "Core/StateHandler.h"

class OctaChainer2AudioProcessor : public juce::AudioProcessor
{
public:
    OctaChainer2AudioProcessor();
    ~OctaChainer2AudioProcessor() override;

    StateHandler& getStateHandler() noexcept;
    const StateHandler& getStateHandler() const noexcept;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

private:
    StateHandler stateHandler;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OctaChainer2AudioProcessor)
};

