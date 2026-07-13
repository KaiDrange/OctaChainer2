#include <array>
#include <algorithm>

#include "StateHandler.h"

StateHandler::StateHandler()
    : valueTree(stateTypeId)
{
    initialiseDefaultState();
    addTreeListeners();
}

StateHandler::~StateHandler()
{
    removeTreeListeners();
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

    removeTreeListeners();
    valueTree = newState;
    ensureSettingsTree();
    addTreeListeners();
    notifyListeners();
}

juce::XmlElement* StateHandler::createXml() const
{
    return valueTree.createXml().release();
}

bool StateHandler::restoreFromXml(const juce::XmlElement& xml)
{
    const auto tree = juce::ValueTree::fromXml(xml);

    if (! tree.isValid())
        return false;

    setState(tree);
    return true;
}

StateHandler::ComboBoxOption StateHandler::getCurrentComboBoxOption(const juce::Identifier& comboBox) const
{
    const auto options = getComboBoxOptions(comboBox);
    if (options.empty())
        return { 0, typeid(int), 0, {} };

    const auto selectedOption = settingsTree.getProperty(comboBox);
    const auto it = std::find_if(options.begin(), options.end(),
                                 [selectedOption](const ComboBoxOption& option)
                                 {
                                     return option.value == selectedOption;
                                 });

    if (it != options.end())
        return *it;

    return options.front();
}

var StateHandler::getComboBoxValue(const ComboBoxOption& option)
{
    if (option.valueType == typeid(int))
        return juce::var(static_cast<int>(option.value));

    if (option.valueType == typeid(bool))
        return juce::var(static_cast<bool>(option.value));

    if (option.valueType == typeid(double))
        return juce::var(static_cast<double>(option.value));

    if (option.valueType == typeid(float))
        return juce::var(static_cast<double>(static_cast<float>(option.value)));

    if (option.valueType == typeid(juce::String))
        return juce::var(option.value.toString());

    jassertfalse;
    return option.value;
}

bool StateHandler::setComboBoxValue(const juce::Identifier& comboBox, const var& value, juce::UndoManager* undoManager)
{
    if (getCurrentComboBoxOption(comboBox).value == value)
        return false;

    settingsTree.setProperty(comboBox, value, undoManager);
    return true;
}

bool StateHandler::setComboBoxValueFromItemId(const juce::Identifier& comboBox, int itemId)
{
    const auto options = getComboBoxOptions(comboBox);
    const auto it = std::find_if(options.begin(), options.end(),
                                 [itemId](const ComboBoxOption& option)
                                 {
                                     return option.itemId == itemId;
                                 });

    if (it != options.end())
        return setComboBoxValue(comboBox, it->value);

    jassertfalse;
    return false;
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
    const auto selectedOption = getCurrentComboBoxOption(comboBox);

    comboBoxRef.clear(juce::dontSendNotification);

    bool selectedOptionStillValid = false;

    for (const auto& option : options)
    {
        comboBoxRef.addItem(option.name, option.itemId);

        if (option.value == selectedOption.value)
            selectedOptionStillValid = true;
    }

    if (! options.empty())
    {
        const auto optionToSelect = selectedOptionStillValid ? selectedOption : options.front();
        comboBoxRef.setSelectedId(optionToSelect.itemId, juce::dontSendNotification);

        if (! selectedOptionStillValid)
            setComboBoxValueFromItemId(comboBox, optionToSelect.itemId);
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

void StateHandler::addTreeListeners()
{
    valueTree.addListener(this);
}

void StateHandler::removeTreeListeners()
{
    valueTree.removeListener(this);
}

void StateHandler::initialiseDefaultState()
{
    valueTree.setProperty(versionId, ProjectInfo::versionString, nullptr);

    settingsTree = juce::ValueTree(settingsId);
    settingsTree.setProperty(timestretchId, getComboBoxOptions(timestretchId).front().value, nullptr);
    settingsTree.setProperty(loopModeId, getComboBoxOptions(loopModeId).front().value, nullptr);
    settingsTree.setProperty(triqQuantId, getComboBoxOptions(triqQuantId).front().value, nullptr);
    settingsTree.setProperty(normalizationId, getComboBoxOptions(normalizationId).front().value, nullptr);
    settingsTree.setProperty(fadeinId, getComboBoxOptions(fadeinId).front().value, nullptr);
    settingsTree.setProperty(fadeoutId, getComboBoxOptions(fadeoutId).front().value, nullptr);
    settingsTree.setProperty(megabreakFileCountId, getComboBoxOptions(megabreakFileCountId).front().value, nullptr);

    valueTree.addChild(settingsTree, -1, nullptr);
}

void StateHandler::ensureSettingsTree()
{
    settingsTree = valueTree.getChildWithName(settingsId);

    if (settingsTree.isValid())
        return;

    settingsTree = juce::ValueTree(settingsId);
    valueTree.addChild(settingsTree, -1, nullptr);
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
