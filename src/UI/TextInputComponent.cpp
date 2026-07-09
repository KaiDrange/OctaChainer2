#include <JuceHeader.h>
#include "TextInputComponent.h"

TextInputComponent::TextInputComponent(const juce::String& labelText,
                                           const int minLength,
                                           const int maxLength,
                                           const juce::String& legalChars,
                                           const bool labelAboveInput) {
    this->labelAboveInput = labelAboveInput;
    label.setText(labelText, juce::dontSendNotification);

    if (legalChars.isEmpty())
        this->legalChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-#";
    
    input.setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(maxLength, this->legalChars), true);
    addAndMakeVisible(label);
    addAndMakeVisible(input);

    this->maxLength = maxLength;
    this->minLength = minLength;
    
    input.onTextChange = [=] {
        if (const auto newText = getValue(); newText.length() >= minLength && newText.length() <= maxLength) {
            input.setText(newText);
        }
    };
    
    input.onFocusLost = [&] {
        sendChangeMessage();
    };
}

TextInputComponent::~TextInputComponent() = default;

void TextInputComponent::resized() {
    auto area = getLocalBounds();
    if (labelAboveInput)
        label.setBounds(area.removeFromTop(static_cast<int>(area.getHeight()*0.5)));
    else
        label.setBounds(area.removeFromLeft(static_cast<int>(area.getWidth()*0.5)));
    input.setBounds(area);
}

juce::String TextInputComponent::getValue() const
{
    return input.getText();
}

void TextInputComponent::setValue(const juce::String& text) {
    input.setText(text);
}

void TextInputComponent::setLabelText(const juce::String& text) {
    label.setText(text, juce::dontSendNotification);
}

void TextInputComponent::addListener(Listener* listenerToAdd) {
    listeners.add(listenerToAdd);
}

void TextInputComponent::removeListener(Listener* listenerToRemove) {
    jassert(listeners.contains(listenerToRemove));
    listeners.remove(listenerToRemove);
}

void TextInputComponent::sendChangeMessage() {
    listeners.call([this](Listener& l) { l.textInputChanged(this); });
}
