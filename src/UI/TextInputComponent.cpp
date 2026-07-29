#include "TextInputComponent.h"
#include "StyleSheet.h"

TextInputComponent::TextInputComponent(const juce::String& labelText,
                                           const int minimumLength,
                                           const int maximumLength,
                                           const juce::String& allowedChars,
                                           const bool labelOnTop) {
    this->labelAboveInput = labelOnTop;
    label.setText(labelText, juce::dontSendNotification);
    label.setFont(StyleSheet::getControlFont());

    if (allowedChars.isEmpty())
        this->legalChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-#";
    else
        this->legalChars = allowedChars;
    
    input.setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(maximumLength, this->legalChars), true);
    input.setFont(StyleSheet::getControlFont());
    input.setIndents(StyleSheet::controlTextInsetX, StyleSheet::controlTextInsetY);
    addAndMakeVisible(label);
    addAndMakeVisible(input);

    this->maxLength = maximumLength;
    this->minLength = minimumLength;
    
    input.onTextChange = [=] {
        if (const auto newText = getValue(); newText.length() >= minimumLength && newText.length() <= maximumLength) {
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
