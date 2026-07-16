#pragma once
#include <JuceHeader.h>

class NumberInputComponent  : public juce::Component {
public:
    NumberInputComponent(const juce::String& labelText, double minValue, double maxValue,
                         double defaultValue, double stepSize, bool labelAboveInput);
    ~NumberInputComponent() override;

    juce::var getValue() const;
    void setValue(int number);
    void setValue(double number);
    void setValue(const juce::var& value);
    void setLabelText(const juce::String& text);
    void setLabelColour(const juce::Colour& colour);
    void resized() override;
    
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void numberInputChanged(NumberInputComponent*) = 0;
    };
    void addListener(Listener *listenerToAdd);
    void removeListener(Listener *listenerToRemove);
    juce::TextEditor input;

private:
    static bool isValidNumberText(const juce::String& text);
    void setValueInternal(double number, bool notifyListeners);
    double snapToStep(double number) const;
    juce::var getValidatedValue() const;
    void adjustValueByStep(int direction);
    void updateStepButtonStates();
    void sendChangeMessage();
    juce::Label label;
    juce::TextButton decrementButton;
    juce::TextButton incrementButton;
    double maxValue;
    double minValue;
    double defaultValue;
    double stepSize;
    juce::ListenerList<Listener> listeners;
    bool labelAboveInput;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NumberInputComponent)
};
