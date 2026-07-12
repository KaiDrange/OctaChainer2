#pragma once

#include <JuceHeader.h>
#include <vector>

class StateHandler : private juce::ValueTree::Listener
{
public:
    const juce::Identifier cbNormalization = "normalizationMode";

    struct ComboBoxOption
    {
        int value;
        juce::String name;
    };

    const std::vector<ComboBoxOption> normalizationOptions = {
        { 1, "No normalization" },
        { 2, "Normalize slices" },
        { 3, "Normalize chain" }
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
