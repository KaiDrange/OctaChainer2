#include <algorithm>

#include "StateHandler.h"

StateHandler::StateHandler()
    : valueTree(stateTypeId)
{
    valueTree.setProperty(versionId, ProjectInfo::versionString, nullptr);
    valueTree.setProperty(timestretchId, getComboBoxOptions(timestretchId).front().value, nullptr);
    valueTree.setProperty(loopModeId, getComboBoxOptions(loopModeId).front().value, nullptr);
    valueTree.setProperty(triqQuantId, getComboBoxOptions(triqQuantId).front().value, nullptr);
    valueTree.setProperty(normalizationId, getComboBoxOptions(normalizationId).front().value, nullptr);
    valueTree.setProperty(fadeinId, getComboBoxOptions(fadeinId).front().value, nullptr);
    valueTree.setProperty(fadeoutId, getComboBoxOptions(fadeoutId).front().value, nullptr);
    valueTree.addListener(this);
}

StateHandler::~StateHandler()
{
    valueTree.removeListener(this);
}

void StateHandler::addListener(Listener* listenerToAdd)
{
    listeners.add(listenerToAdd);
}

void StateHandler::removeListener(Listener* listenerToRemove)
{
    listeners.remove(listenerToRemove);
}

const juce::ValueTree& StateHandler::getState() const noexcept
{
    return valueTree;
}

void StateHandler::setState(const juce::ValueTree& newState)
{
    if (! newState.isValid())
        return;

    if (newState.getType() != stateTypeId)
        return;

    valueTree = newState;
}

juce::XmlElement* StateHandler::createXml() const
{
    return valueTree.createXml().release();
}

bool StateHandler::restoreFromXml(const juce::XmlElement& xml)
{
    auto tree = juce::ValueTree::fromXml(xml);

    if (! tree.isValid())
        return false;

    setState(tree);
    return true;
}

StateHandler::ComboBoxOption StateHandler::getComboBoxOption(const juce::Identifier& comboBox) const
{
    const auto options = getComboBoxOptions(comboBox);
    if (options.empty())
        return { 0, {} };

    const auto value = static_cast<int>(valueTree.getProperty(comboBox, options.front().value));

    const auto it = std::find_if(options.begin(), options.end(),
                                 [value](const ComboBoxOption& option)
                                 {
                                     return option.value == value;
                                 });

    if (it != options.end())
        return *it;

    return options.front();
}

bool StateHandler::setComboBoxValue(const juce::Identifier& comboBox, const int value, juce::UndoManager* undoManager)
{
    if (getComboBoxOption(comboBox).value == value)
        return false;

    valueTree.setProperty(comboBox, value, undoManager);
    return true;
}

std::vector<StateHandler::ComboBoxOption> StateHandler::getComboBoxOptions(const juce::Identifier& comboBox) const
{
    if (comboBox == this->timestretchId)
        return timestretchOptions;
    else if (comboBox == this->loopModeId)
        return loopOptions;
    else if (comboBox == this->triqQuantId)
        return trigQuantOptions;
    else if (comboBox == this->normalizationId)
        return normalizationOptions;
    else if (comboBox == this->fadeinId)
        return fadeinOptions;
    else if (comboBox == this->fadeoutId)
        return fadeoutOptions;
    else if (comboBox == this->megabreakFileCountId)
        return megabreakFileCountOptions;

    return {};
}

void StateHandler::refreshComboBox(const juce::Identifier& comboBox, juce::ComboBox& comboBoxRef)
{
    const auto options = getComboBoxOptions(comboBox);
    const auto selectedOption = getComboBoxOption(comboBox).value;

    comboBoxRef.clear(juce::dontSendNotification);

    bool selectedOptionStillValid = false;

    for (const auto& option : options)
    {
        comboBoxRef.addItem(option.name, option.value);

        if (option.value == selectedOption)
            selectedOptionStillValid = true;
    }

    if (! options.empty())
    {
        const auto optionToSelect = selectedOptionStillValid ? selectedOption : options.front().value;
        comboBoxRef.setSelectedId(optionToSelect, juce::dontSendNotification);

        if (! selectedOptionStillValid)
            setComboBoxValue(comboBox, optionToSelect);
    }
    else
    {
        comboBoxRef.setSelectedId(0, juce::dontSendNotification);
    }
}

void StateHandler::notifyListeners()
{
    listeners.call([](Listener& listener) { listener.stateChanged(); });
}

void StateHandler::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&)
{
    notifyListeners();
}

void StateHandler::valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&)
{
    notifyListeners();
}

void StateHandler::valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int)
{
    notifyListeners();
}

void StateHandler::valueTreeChildOrderChanged(juce::ValueTree&, int, int)
{
    notifyListeners();
}

void StateHandler::valueTreeParentChanged(juce::ValueTree&)
{
    notifyListeners();
}

void StateHandler::valueTreeRedirected(juce::ValueTree&)
{
    notifyListeners();
}
