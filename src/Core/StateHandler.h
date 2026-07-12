#pragma once

#include <JuceHeader.h>

#include <vector>

class StateHandler : private juce::ValueTree::Listener
{
public:
    enum class NormalizationMode : int
    {
        None = 0,
        Slice = 1,
        Chain = 2
    };

    struct NormalizationOption
    {
        NormalizationMode mode;
        juce::String label;
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

    NormalizationMode getNormalizationMode() const;
    bool setNormalizationMode(NormalizationMode mode, juce::UndoManager* undoManager = nullptr);

    static std::vector<NormalizationOption> getNormalizationOptions();
    static juce::String getNormalizationLabel(NormalizationMode mode);
    static int getNormalizationItemId(NormalizationMode mode);
    static NormalizationMode normalizationModeFromItemId(int itemId);

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
