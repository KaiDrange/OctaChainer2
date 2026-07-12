#pragma once

#include <JuceHeader.h>
#include <vector>

class StateHandler : private juce::ValueTree::Listener
{
public:
    const juce::Identifier stateTypeId { "octaChainer2State" };
    const juce::Identifier versionId { "version" };
    const juce::Identifier timestretchId = "timeStretch";
    const juce::Identifier loopModeId = "loopMode";
    const juce::Identifier triqQuantId = "trigQuant";
    const juce::Identifier normalizationId = "normalizationMode";
    const juce::Identifier fadeinId = "fade-in";
    const juce::Identifier fadeoutId = "fade-out";
    const juce::Identifier megabreakFileCountId = "megabreakFileCount";

    struct ComboBoxOption
    {
        int value;
        juce::String name;
    };

    const std::vector<ComboBoxOption> trigQuantOptions = {
        { 1000, "Trig quant direct" },
        { 2000, "Trig quant pattern" },
        { 1, "Trig quant 1" },
        { 2, "Trig quant 2" },
        { 3, "Trig quant 3" },
        { 4, "Trig quant 4" },
        { 6, "Trig quant 6" },
        { 8, "Trig quant 8" },
        { 12, "Trig quant 12" },
        { 16, "Trig quant 16" },
        { 24, "Trig quant 24" },
        { 32, "Trig quant 32" },
        { 48, "Trig quant 48" },
        { 64, "Trig quant 64" },
        { 96, "Trig quant 96" },
        { 128, "Trig quant 128" },
        { 192, "Trig quant 192" },
        { 256, "Trig quant 256" }
    };

    const std::vector<ComboBoxOption> timestretchOptions = {
        { 1, "Timestretch off" },
        { 2, "Timestretch normal" },
        { 3, "Timestretch beat" }
    };

    const std::vector<ComboBoxOption> loopOptions = {
        { 1, "Loop off" },
        { 2, "Loop on" },
        { 3, "Loop pingpong" }
    };

    const std::vector<ComboBoxOption> normalizationOptions = {
        { 1, "No normalization" },
        { 2, "Normalize slices" },
        { 3, "Normalize chain" }
    };

    const std::vector<ComboBoxOption> fadeinOptions = {
        { -1, "No fade-in" },
        { 1, "1ms fade-in" },
        { 2, "2ms fade-in" },
        { 3, "3ms fade-in" },
        { 5, "5ms fade-in" },
        { 10, "10ms fade-in" }
    };

    const std::vector<ComboBoxOption> fadeoutOptions = {
        { -1, "No fade-out" },
        { 1, "1ms fade-out" },
        { 2, "2ms fade-out" },
        { 3, "3ms fade-out" },
        { 5, "5ms fade-out" },
        { 10, "10ms fade-out" }
    };

    const std::vector<ComboBoxOption> megabreakFileCountOptions = {
        { 1, "File count: 1" },
        { 2, "File count: 2" },
        { 3, "File count: 3" },
        { 4, "File count: 4" },
        { 6, "File count: 6" },
        { 8, "File count: 8" },
        { 12, "File count: 12" },
        { 16, "File count: 16" },
        { 24, "File count: 24" },
        { 32, "File count: 32" },
        { 48, "File count: 48" },
        { 64, "File count: 64" }
    };

    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void stateChanged() = 0;
    };

    StateHandler();
    ~StateHandler() override;

    void addListener(Listener* listenerToAdd);
    void removeListener(Listener* listenerToRemove);

    const juce::ValueTree& getState() const noexcept;
    void setState(const juce::ValueTree& newState);
    juce::XmlElement* createXml() const;
    bool restoreFromXml(const juce::XmlElement& xml);

    ComboBoxOption getComboBoxOption(const juce::Identifier& comboBox) const;
    bool setComboBoxValue(const juce::Identifier& comboBox, int value, juce::UndoManager* undoManager = nullptr);
    std::vector<ComboBoxOption> getComboBoxOptions(const juce::Identifier& comboBox) const;
    void refreshComboBox(const juce::Identifier& comboBox, juce::ComboBox& comboBoxRef);

    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) override;
    void valueTreeChildOrderChanged(juce::ValueTree&, int, int) override;
    void valueTreeParentChanged(juce::ValueTree&) override;
    void valueTreeRedirected(juce::ValueTree&) override;

private:
    void notifyListeners();

    juce::ValueTree valueTree;
    juce::ListenerList<Listener> listeners;
};
