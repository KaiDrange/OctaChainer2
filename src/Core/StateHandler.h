#pragma once

#include <JuceHeader.h>
#include <initializer_list>
#include <vector>

class Slice;

class StateHandler : juce::ValueTree::Listener
{
public:
#include "StateIdentifiers.h"
#include "StateValueDefinitions.h"

    struct StateChange
    {
        enum Flag : uint32_t
        {
            none = 0,
            settings = 1 << 0,
            sliceList = 1 << 1,
            selectedSlice = 1 << 2,
            fullReload = 1 << 3
        };

        uint32_t flags = none;
        juce::Identifier property;

        bool has(const Flag flag) const { return (flags & flag) != 0; }
        bool isSetting(const juce::Identifier& id) const
        {
            return has(settings) && property == id;
        }
    };

    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void stateChanged(const StateChange& change) = 0;
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
    template <typename T>
    T getStateValue(const juce::Identifier& identifier, T defaultValue)
    {
        if (! settingsTree.isValid() || ! settingsTree.hasProperty(identifier))
            return defaultValue;

        return static_cast<T>(settingsTree.getProperty(identifier, defaultValue));
    }

    bool setStateValue(const juce::Identifier& identifier, const var& value, juce::UndoManager* undoManager = nullptr);
    bool setStateValueFromItemId(const juce::Identifier& identifier, int itemId);
    std::vector<Option> getOptions(const juce::Identifier& identifier) const;
    void refreshComboBox(const juce::Identifier& identifier, juce::ComboBox& comboBoxRef);
    void refreshRadioButtons(const juce::Identifier& identifier, std::initializer_list<juce::ToggleButton*> buttons);

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) override;
    void valueTreeChildOrderChanged(juce::ValueTree&, int, int) override;
    void valueTreeParentChanged(juce::ValueTree&) override;
    void valueTreeRedirected(juce::ValueTree&) override;

    int getNumSlices() const;
    juce::ValueTree getSliceTree(int index) const;
    juce::ValueTree getSelectedSliceTree() const;
    bool loadSelectedSliceAudio(juce::AudioBuffer<float>& destination, double& sampleRate) const;

    int addSlice(const Slice& slice, juce::UndoManager* undoManager = nullptr);
    int addBlankSlice(int64 lengthInSamples, juce::UndoManager* undoManager = nullptr);
    void removeSelectedSlice();
    void removeAllSlices();

    bool selectSlice(int index, juce::UndoManager* undoManager = nullptr);
    int getSelectedSliceIndex() const;
    bool moveSlice(int fromIndex, int toIndex);

private:
    void addTreeListeners();
    void removeTreeListeners();
    void initialiseDefaultState();
    void ensureSettingsTree();
    void ensureDataTree();
    void setDefaultStateValue(const juce::Identifier& identifier, const juce::var& value);
    void notifyListeners(const StateChange& change);
    static juce::MemoryBlock createAudioDataBlock(const Slice& slice);

    juce::ValueTree valueTree;
    juce::ValueTree settingsTree;
    juce::ValueTree dataTree;
    juce::ListenerList<Listener> listeners;
};
