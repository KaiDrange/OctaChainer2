#pragma once

#include <JuceHeader.h>

class TextInputComponent : public juce::Component {
public:
    TextInputComponent(const juce::String& labelText, int minimumLength, int maximumLength, const juce::String& allowedChars, bool labelOnTop);
    ~TextInputComponent() override;

    juce::String getValue() const;
    void setValue(const juce::String& text);
    void setLabelText(const juce::String& text);
    void resized() override;
    
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void textInputChanged(TextInputComponent*) = 0;
    };
    void addListener(Listener *listenerToAdd);
    void removeListener(Listener *listenerToRemove);
    juce::TextEditor input;

private:
    void sendChangeMessage();
    juce::Label label;
    int minLength;
    int maxLength;
    juce::String legalChars;
    juce::ListenerList<Listener> listeners;
    bool labelAboveInput;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextInputComponent)
};
