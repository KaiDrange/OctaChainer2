#pragma once

#include <JuceHeader.h>
#include <initializer_list>
#include <vector>

class StateHandler : private juce::ValueTree::Listener
{
public:
    // Root identifiers
    const juce::Identifier stateTypeId { "octaChainer2" };
    const juce::Identifier versionId { "version" };
    const juce::Identifier settingsId { "settings" };

    // Settings identifiers
    const juce::Identifier bitrateId = "bitrate";
    const juce::Identifier channelsId = "channels";
    const juce::Identifier samplerateId = "samplerate";
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

    Option getCurrentOption(const juce::Identifier& identifier) const;
    static var getOptionValue(const Option& option);
    bool setStateValue(const juce::Identifier& identifier, const var& value, juce::UndoManager* undoManager = nullptr);
    bool setStateValueFromItemId(const juce::Identifier& identifier, int itemId);
    std::vector<Option> getOptions(const juce::Identifier& identifier) const;
    void refreshComboBox(const juce::Identifier& identifier, juce::ComboBox& comboBoxRef);
    void refreshRadioButtons(const juce::Identifier& identifier, std::initializer_list<juce::ToggleButton*> buttons);

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
