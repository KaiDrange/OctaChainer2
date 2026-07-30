#include "NumberInputComponent.h"
#include "StyleSheet.h"

NumberInputComponent::NumberInputComponent(const juce::String& labelText,
                                           const double minimumValue,
                                           const double maximumValue,
                                           const double defaultNumber,
                                           const double stepAmount,
                                           const bool labelOnTop) {
    this->labelAboveInput = labelOnTop;
    label.setText(labelText, juce::dontSendNotification);
    label.setFont(StyleSheet::getControlFont());
    input.setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(16, "-0123456789."), true);
    input.setJustification(juce::Justification::right);
    input.setFont(StyleSheet::getControlFont());
    setValueColour(juce::Colour(StyleSheet::controlTextColour));
    input.setIndents(StyleSheet::controlTextInsetX, StyleSheet::controlTextInsetY);
    addAndMakeVisible(label);
    addAndMakeVisible(input);

    this->minValue = minimumValue;
    this->maxValue = maximumValue;
    this->defaultValue = defaultNumber;
    this->stepSize = stepAmount;
    stepButton = std::make_unique<SplitStepButton>();
    stepButton->onStep = [this](const SplitStepButton::Direction direction)
    {
        adjustValueByStep(direction == SplitStepButton::Direction::increment ? 1 : -1);
    };
    addAndMakeVisible(*stepButton);
    setValue(defaultNumber);

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

    const auto buttonWidth = juce::jlimit(16, 20, juce::jmax(0, area.getHeight()));
    const auto buttonAreaWidth = juce::jmin(area.getWidth(), buttonWidth);
    auto buttonArea = area.removeFromRight(buttonAreaWidth);
    input.setBounds(area);
    stepButton->setBounds(buttonArea);
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
    const auto normalised = roundToStepPrecision(juce::jlimit(minValue, maxValue, snapToStep(number)));
    const auto clampedValue = juce::approximatelyEqual(normalised, static_cast<double>(juce::roundToInt(normalised)))
        ? juce::var(juce::roundToInt(normalised))
        : juce::var(normalised);

    input.setText(formatValue(static_cast<double>(clampedValue)), juce::dontSendNotification);
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

int NumberInputComponent::getDecimalPlaces() const
{
    if (stepSize <= 0.0)
        return 0;

    auto scaledStep = std::abs(stepSize);
    int decimalPlaces = 0;

    while (decimalPlaces < 6 && std::abs(scaledStep - std::round(scaledStep)) > 1.0e-9)
    {
        scaledStep *= 10.0;
        ++decimalPlaces;
    }

    return decimalPlaces;
}

double NumberInputComponent::roundToStepPrecision(const double number) const
{
    const auto decimalPlaces = getDecimalPlaces();
    if (decimalPlaces <= 0)
        return std::round(number);

    const auto scale = std::pow(10.0, static_cast<double>(decimalPlaces));
    return std::round(number * scale) / scale;
}

juce::String NumberInputComponent::formatValue(const double number) const
{
    const auto decimalPlaces = getDecimalPlaces();
    if (decimalPlaces <= 0)
        return juce::String(juce::roundToInt(number));

    return juce::String(number, decimalPlaces);
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
        stepButton->setEnabled(false);
        stepButton->setIncrementEnabled(false);
        stepButton->setDecrementEnabled(false);
        return;
    }

    stepButton->setEnabled(true);
    const auto currentValue = getValidatedValue();
    const auto value = currentValue.isVoid() ? defaultValue : static_cast<double>(currentValue);

    stepButton->setDecrementEnabled(value > minValue);
    stepButton->setIncrementEnabled(value < maxValue);
}

void NumberInputComponent::setLabelText(const juce::String& text) {
    label.setText(text, juce::dontSendNotification);
}

void NumberInputComponent::setLabelColour(const juce::Colour& colour)
{
    label.setColour(juce::Label::textColourId, colour);
}

void NumberInputComponent::setValueColour(const juce::Colour& colour)
{
    input.setColour(juce::TextEditor::textColourId, colour);
    input.applyColourToAllText(colour);
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
