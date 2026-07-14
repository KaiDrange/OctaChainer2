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

void StateHandler::initialiseDefaultState()
{
    valueTree.setProperty(versionId, ProjectInfo::versionString, nullptr);

    settingsTree = juce::ValueTree(settingsId);
    const auto setDefaultOption = [this](const juce::Identifier& identifier)
    {
        const auto options = getOptions(identifier);
        if (! options.empty())
            settingsTree.setProperty(identifier, options.front().value, nullptr);
    };

    setDefaultOption(bitrateId);
    setDefaultOption(timestretchId);
    setDefaultOption(loopModeId);
    setDefaultOption(triqQuantId);
    setDefaultStateValue(gainId, gainValue.defaultValue);
    setDefaultStateValue(bpmId, bpmValue.defaultValue);
    setDefaultOption(normalizationId);
    setDefaultOption(fadeinId);
    setDefaultOption(fadeoutId);
    setDefaultStateValue(otFileId, otFileDefault);
    setDefaultStateValue(evenGridId, evenGridDefault);
    setDefaultStateValue(embedMarkersId, embedMarkersDefault);
    setDefaultOption(megabreakFileCountId);

    valueTree.addChild(settingsTree, -1, nullptr);
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

StateHandler::Option StateHandler::getCurrentOption(const juce::Identifier& identifier) const
{
    const auto options = getOptions(identifier);
    if (options.empty())
        return { 0, typeid(int), 0, {} };

    const auto selectedOption = settingsTree.getProperty(identifier);
    const auto it = std::find_if(options.begin(), options.end(),
                                 [selectedOption](const Option& option)
                                 {
                                     return option.value == selectedOption;
                                 });

    if (it != options.end())
        return *it;

    return options.front();
}

var StateHandler::getOptionValue(const Option& option)
{
    if (option.valueType == typeid(int))
        return {static_cast<int>(option.value)};

    if (option.valueType == typeid(bool))
        return {static_cast<bool>(option.value)};

    if (option.valueType == typeid(double))
        return {static_cast<double>(option.value)};

    if (option.valueType == typeid(float))
        return {static_cast<double>(static_cast<float>(option.value))};

    if (option.valueType == typeid(juce::String))
        return {option.value.toString()};

    jassertfalse;
    return option.value;
}

bool StateHandler::setStateValue(const juce::Identifier& identifier, const var& value, juce::UndoManager* undoManager)
{
    const auto options = getOptions(identifier);

    if (! options.empty())
    {
        if (getCurrentOption(identifier).value == value)
            return false;
    }
    else if (settingsTree.hasProperty(identifier) && settingsTree.getProperty(identifier) == value)
    {
        return false;
    }

    settingsTree.setProperty(identifier, value, undoManager);
    return true;
}

bool StateHandler::setStateValueFromItemId(const juce::Identifier& identifier, int itemId)
{
    const auto options = getOptions(identifier);
    const auto it = std::find_if(options.begin(), options.end(),
                                 [itemId](const Option& option)
                                 {
                                     return option.itemId == itemId;
                                 });

    if (it != options.end())
        return setStateValue(identifier, it->value);

    jassertfalse;
    return false;
}

std::vector<StateHandler::Option> StateHandler::getOptions(const juce::Identifier& identifier) const
{
    if (identifier == this->bitrateId)
        return bitrateOptions;
    if (identifier == this->channelsId)
        return channelOptions;
    if (identifier == this->samplerateId)
        return samplerateOptions;
    if (identifier == this->timestretchId)
        return timestretchOptions;
    if (identifier == this->loopModeId)
        return loopOptions;
    if (identifier == this->triqQuantId)
        return trigQuantOptions;
    if (identifier == this->normalizationId)
        return normalizationOptions;
    if (identifier == this->fadeinId)
        return fadeinOptions;
    if (identifier == this->fadeoutId)
        return fadeoutOptions;
    if (identifier == this->megabreakFileCountId)
        return megabreakFileCountOptions;

    return {};
}

void StateHandler::refreshComboBox(const juce::Identifier& identifier, juce::ComboBox& comboBoxRef)
{
    const auto options = getOptions(identifier);
    const auto selectedOption = getCurrentOption(identifier);

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
            setStateValueFromItemId(identifier, optionToSelect.itemId);
    }
    else
    {
        comboBoxRef.setSelectedId(0, juce::dontSendNotification);
    }
}

void StateHandler::refreshRadioButtons(const juce::Identifier& identifier, const std::initializer_list<juce::ToggleButton*> buttons)
{
    auto options = getOptions(identifier);
    std::sort(options.begin(), options.end(),
              [](const Option& lhs, const Option& rhs)
              {
                  return lhs.itemId < rhs.itemId;
              });
    const auto selectedOption = getCurrentOption(identifier);

    auto optionIt = options.begin();
    bool selectedOptionStillValid = false;

    for (auto* button : buttons)
    {
        if (button == nullptr)
            continue;

        if (optionIt != options.end())
        {
            const auto& option = *optionIt++;
            button->setButtonText(option.name);
            const auto isSelected = option.value == selectedOption.value;
            button->setToggleState(isSelected, juce::dontSendNotification);
            selectedOptionStillValid |= isSelected;
        }
        else
        {
            button->setToggleState(false, juce::dontSendNotification);
        }
    }

    if (! options.empty() && ! selectedOptionStillValid)
        setStateValueFromItemId(identifier, options.front().itemId);
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

void StateHandler::ensureSettingsTree()
{
    settingsTree = valueTree.getChildWithName(settingsId);

    if (settingsTree.isValid())
    {
        setDefaultStateValue(gainId, 0);
        setDefaultStateValue(bpmId, 120);
        return;
    }

    settingsTree = juce::ValueTree(settingsId);
    setDefaultStateValue(gainId, 0);
    setDefaultStateValue(bpmId, 120);
    valueTree.addChild(settingsTree, -1, nullptr);
}

void StateHandler::setDefaultStateValue(const juce::Identifier& identifier, const juce::var& value)
{
    if (! settingsTree.hasProperty(identifier))
        settingsTree.setProperty(identifier, value, nullptr);
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
