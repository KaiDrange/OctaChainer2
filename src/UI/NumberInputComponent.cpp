#include <JuceHeader.h>
#include "NumberInputComponent.h"
#include "StyleSheet.h"

NumberInputComponent::NumberInputComponent(const juce::String& labelText,
                                           const double minValue,
                                           const double maxValue,
                                           const double defaultValue,
                                           const double stepSize,
                                           const bool labelAboveInput) {
    this->labelAboveInput = labelAboveInput;
    label.setText(labelText, juce::dontSendNotification);
    label.setFont(StyleSheet::getControlFont());
    input.setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(16, "-0123456789."), true);
    input.setJustification(juce::Justification::right);
    input.setFont(StyleSheet::getControlFont());
    input.setColour(juce::TextEditor::textColourId, juce::Colour(StyleSheet::controlTextColour));
    input.setIndents(StyleSheet::controlTextInsetX, StyleSheet::controlTextInsetY);
    addAndMakeVisible(label);
    addAndMakeVisible(input);

    this->minValue = minValue;
    this->maxValue = maxValue;
    this->defaultValue = defaultValue;
    this->stepSize = stepSize;
    setValue(defaultValue);

    decrementButton.setButtonText("-");
    incrementButton.setButtonText("+");
    addAndMakeVisible(decrementButton);
    addAndMakeVisible(incrementButton);

    decrementButton.onClick = [this] { adjustValueByStep(-1); };
    incrementButton.onClick = [this] { adjustValueByStep(1); };

    input.onFocusLost = [this] {
        const auto value = getValidatedValue();
        if (value.isVoid())
        {
            sendChangeMessage();
            return;
        }

        setValueInternal(value, false);
        sendChangeMessage();
    };

    updateStepButtonStates();
}

NumberInputComponent::~NumberInputComponent() = default;

void NumberInputComponent::resized() {
    auto area = getLocalBounds();
    if (labelAboveInput)
        label.setBounds(area.removeFromTop(static_cast<int>(area.getHeight()*0.5)));
    else
        label.setBounds(area.removeFromLeft(static_cast<int>(area.getWidth()*0.5)));

    const auto buttonWidth = juce::jlimit(14, 18, juce::jmax(0, area.getHeight() - 2));
    const auto buttonAreaWidth = juce::jmin(area.getWidth(), buttonWidth);
    auto buttonArea = area.removeFromRight(buttonAreaWidth);
    input.setBounds(area);

    const auto buttonGap = juce::jmin(StyleSheet::controlGap, juce::jmax(1, buttonArea.getHeight() / 10));
    const auto buttonHeight = juce::jmax(1, (buttonArea.getHeight() - buttonGap) / 2);
    const auto topButtonHeight = buttonHeight;
    const auto bottomButtonHeight = juce::jmax(1, buttonArea.getHeight() - topButtonHeight - buttonGap);

    incrementButton.setBounds(buttonArea.removeFromTop(topButtonHeight));
    buttonArea.removeFromTop(buttonGap);
    decrementButton.setBounds(buttonArea.removeFromTop(bottomButtonHeight));
}

juce::var NumberInputComponent::getValue() const
{
    return getValidatedValue();
}

juce::var NumberInputComponent::getValidatedValue() const
{
    const auto text = input.getText().trim();
    if (! isValidNumberText(text))
        return defaultValue;

    if (text.containsChar('.'))
        return {text.getDoubleValue()};

    return {text.getIntValue()};
}

void NumberInputComponent::setValue(const int number) {
    setValue(juce::var(number));
}

void NumberInputComponent::setValue(const double number) {
    setValue(juce::var(number));
}

void NumberInputComponent::setValue(const juce::var& value) {
    if (value.isVoid()) {
        input.setText(juce::String(defaultValue), juce::dontSendNotification);
        updateStepButtonStates();
        return;
    }

    setValueInternal(value, false);
}

void NumberInputComponent::setValueInternal(const double number, const bool notifyListeners)
{
    const auto normalised = juce::jlimit(minValue, maxValue, snapToStep(number));
    const auto clampedValue = normalised == static_cast<double>(juce::roundToInt(normalised))
        ? juce::var(juce::roundToInt(normalised))
        : juce::var(normalised);

    input.setText(clampedValue.toString(), juce::dontSendNotification);
    updateStepButtonStates();

    if (notifyListeners)
        sendChangeMessage();
}

double NumberInputComponent::snapToStep(const double number) const
{
    if (stepSize <= 0.0)
        return number;

    const auto snappedSteps = juce::roundToInt((number - minValue) / stepSize);
    return minValue + static_cast<double>(snappedSteps) * stepSize;
}

void NumberInputComponent::adjustValueByStep(const int direction)
{
    const auto currentValue = getValidatedValue();
    const auto startValue = currentValue.isVoid() ? defaultValue : static_cast<double>(currentValue);
    setValueInternal(startValue + stepSize * static_cast<double>(direction), true);
}

void NumberInputComponent::updateStepButtonStates()
{
    if (stepSize <= 0.0)
    {
        decrementButton.setEnabled(false);
        incrementButton.setEnabled(false);
        return;
    }

    const auto currentValue = getValidatedValue();
    const auto value = currentValue.isVoid() ? defaultValue : static_cast<double>(currentValue);

    decrementButton.setEnabled(value > minValue);
    incrementButton.setEnabled(value < maxValue);
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

bool NumberInputComponent::isValidNumberText(const juce::String& text)
{
    if (text.isEmpty())
        return false;

    auto startIndex = 0;
    auto digitCount = 0;
    auto dotCount = 0;

    if (text.startsWithChar('-'))
        startIndex = 1;

    for (auto i = startIndex; i < text.length(); ++i)
    {
        const auto c = text[i];

        if (juce::CharacterFunctions::isDigit(c))
        {
            ++digitCount;
            continue;
        }

        if (c == '.')
        {
            ++dotCount;
            if (dotCount > 1)
                return false;
            continue;
        }

        return false;
    }

    return digitCount > 0;
}
