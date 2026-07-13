#pragma once

#include <JuceHeader.h>
#include <vector>

class StateHandler : private juce::ValueTree::Listener
{
public:
    const juce::Identifier stateTypeId { "octaChainer2State" };
    const juce::Identifier versionId { "version" };
    const juce::Identifier settingsId { "settings" };
    const juce::Identifier timestretchId = "timeStretch";
    const juce::Identifier loopModeId = "loopMode";
    const juce::Identifier triqQuantId = "trigQuant";
    const juce::Identifier normalizationId = "normalizationMode";
    const juce::Identifier fadeinId = "fade-in";
    const juce::Identifier fadeoutId = "fade-out";
    const juce::Identifier megabreakFileCountId = "megabreakFileCount";

#include "SettingsOptions.h"

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

    ComboBoxOption getCurrentComboBoxOption(const juce::Identifier& comboBox) const;
    static var getComboBoxValue(const ComboBoxOption& option);
    bool setComboBoxValue(const juce::Identifier& comboBox, const var& value, juce::UndoManager* undoManager = nullptr);
    bool setComboBoxValueFromItemId(const juce::Identifier& comboBox, int itemId);
    std::vector<ComboBoxOption> getComboBoxOptions(const juce::Identifier& comboBox) const;
    void refreshComboBox(const juce::Identifier& comboBox, juce::ComboBox& comboBoxRef);

    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) override;
    void valueTreeChildOrderChanged(juce::ValueTree&, int, int) override;
    void valueTreeParentChanged(juce::ValueTree&) override;
    void valueTreeRedirected(juce::ValueTree&) override;

private:
    void addTreeListeners();
    void removeTreeListeners();
    void initialiseDefaultState();
    void ensureSettingsTree();
    void notifyListeners();

    juce::ValueTree valueTree;
    juce::ValueTree settingsTree;
    juce::ListenerList<Listener> listeners;
};
