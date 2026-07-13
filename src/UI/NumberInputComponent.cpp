#include <JuceHeader.h>
#include "NumberInputComponent.h"
#include "StyleSheet.h"

NumberInputComponent::NumberInputComponent(const juce::String& labelText,
                                           const int maxDigits,
                                           const int minValue,
                                           const int maxValue,
                                           const bool labelAboveInput) {
    this->labelAboveInput = labelAboveInput;
    label.setText(labelText, juce::dontSendNotification);
    label.setFont(StyleSheet::getControlFont());
    input.setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(maxDigits, "-0123456789."), true);
    input.setJustification(juce::Justification::right);
    input.setFont(StyleSheet::getControlFont());
    input.setIndents(StyleSheet::controlTextInsetX, StyleSheet::controlTextInsetY);
    addAndMakeVisible(label);
    addAndMakeVisible(input);

    this->minValue = minValue;
    this->maxValue = maxValue;

    input.onFocusLost = [this, maxValue, minValue] {
        const auto value = getValue();
        if (value.isVoid())
        {
            sendChangeMessage();
            return;
        }

        auto number = static_cast<double>(value);
        number = juce::jlimit(static_cast<double>(minValue), static_cast<double>(maxValue), number);

        const auto clampedValue = number == static_cast<double>(juce::roundToInt(number))
            ? juce::var(juce::roundToInt(number))
            : juce::var(number);

        input.setText(clampedValue.toString(), juce::dontSendNotification);
        sendChangeMessage();
    };
}

NumberInputComponent::~NumberInputComponent() = default;

void NumberInputComponent::resized() {
    auto area = getLocalBounds();
    if (labelAboveInput)
        label.setBounds(area.removeFromTop(static_cast<int>(area.getHeight()*0.5)));
    else
        label.setBounds(area.removeFromLeft(static_cast<int>(area.getWidth()*0.5)));
    input.setBounds(area);
}

juce::var NumberInputComponent::getValue() const
{
    const auto text = input.getText().trim();
    if (text.isEmpty())
        return {};

    if (text.containsOnly("0123456789"))
        return {text.getIntValue()};

    if (text.containsOnly("0123456789."))
        return {text.getDoubleValue()};

    return {};
}

void NumberInputComponent::setValue(const int number) {
    setValue(juce::var(number));
}

void NumberInputComponent::setValue(const double number) {
    setValue(juce::var(number));
}

void NumberInputComponent::setValue(const juce::var& value) {
    if (value.isVoid()) {
        input.setText({}, juce::dontSendNotification);
        return;
    }

    auto number = static_cast<double>(value);
    number = juce::jlimit(static_cast<double>(minValue), static_cast<double>(maxValue), number);

    const auto clampedValue = number == static_cast<double>(juce::roundToInt(number))
        ? juce::var(juce::roundToInt(number))
        : juce::var(number);

    input.setText(clampedValue.toString(), juce::dontSendNotification);
}
void NumberInputComponent::setLabelText(const juce::String& text) {
    label.setText(text, juce::dontSendNotification);
}

void NumberInputComponent::addListener(Listener* listenerToAdd) {
    listeners.add(listenerToAdd);
}

void NumberInputComponent::removeListener(Listener* listenerToRemove) {
    jassert(listeners.contains(listenerToRemove));
    listeners.remove(listenerToRemove);
}

void NumberInputComponent::sendChangeMessage() {
    listeners.call([this](Listener& l) { l.numberInputChanged(this); });
}
