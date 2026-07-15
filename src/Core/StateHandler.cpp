#include <limits>
#include <algorithm>
#include "StateHandler.h"
#include "Slice.h"

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

    setDefaultOption(bitDepthId);
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

    dataTree = juce::ValueTree(dataId);
    dataTree.setProperty(selectedSliceId, -1, nullptr);
    valueTree.addChild(dataTree, -1, nullptr);
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
    ensureDataTree();
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
    if (identifier == this->bitDepthId)
        return bitDepthOptions;
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

int StateHandler::getNumSlices() const
{
    return dataTree.isValid() ? dataTree.getNumChildren() : 0;
}

juce::ValueTree StateHandler::getSliceTree(const int index) const
{
    if (! dataTree.isValid() || ! juce::isPositiveAndBelow(index, dataTree.getNumChildren()))
        return {};

    return dataTree.getChild(index);
}

juce::ValueTree StateHandler::getSelectedSliceTree() const
{
    return getSliceTree(getSelectedSliceIndex());
}

bool StateHandler::loadSelectedSliceAudio(juce::AudioBuffer<float>& destination, double& sampleRate) const
{
    const auto sliceTree = getSelectedSliceTree();
    if (! sliceTree.isValid())
        return false;

    const auto numChannels = static_cast<int>(sliceTree.getProperty(sliceChannelsId, 0));
    sampleRate = static_cast<double>(sliceTree.getProperty(sliceSamplerateId, 0.0));
    const auto numSamples = static_cast<juce::int64>(sliceTree.getProperty(sliceNumSamplesId, static_cast<juce::int64>(0)));
    const auto* audioDataValue = sliceTree.getPropertyPointer(sliceAudioDataId);

    if (numChannels <= 0
        || sampleRate <= 0.0
        || numSamples <= 0
        || numSamples > static_cast<juce::int64>(std::numeric_limits<int>::max())
        || audioDataValue == nullptr)
    {
        return false;
    }

    const auto* audioDataBlock = audioDataValue->getBinaryData();
    if (audioDataBlock == nullptr)
        return false;

    const auto samplesPerChannel = static_cast<size_t>(numSamples);
    const auto expectedBytes = static_cast<size_t>(numChannels) * samplesPerChannel * sizeof(float);
    if (audioDataBlock->getSize() < expectedBytes)
        return false;

    const auto numSamplesAsInt = static_cast<int>(numSamples);
    const auto* samples = static_cast<const float*>(audioDataBlock->getData());

    destination.setSize(numChannels, numSamplesAsInt, false, false, false);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        destination.copyFrom(channel,
                             0,
                             samples + static_cast<size_t>(channel) * samplesPerChannel,
                             numSamplesAsInt);
    }

    return true;
}

int StateHandler::addSlice(const Slice& slice, juce::UndoManager* undoManager)
{
    ensureDataTree();

    juce::ValueTree sliceTree(sliceId);
    sliceTree.setProperty(sliceNameId, slice.name, nullptr);
    sliceTree.setProperty(sliceSourcePathId, slice.sourcePath, nullptr);
    sliceTree.setProperty(sliceChannelsId, slice.channels, nullptr);
    sliceTree.setProperty(sliceSamplerateId, slice.samplerate, nullptr);
    sliceTree.setProperty(sliceBitrateId, static_cast<int>(slice.bitDepth), nullptr);
    sliceTree.setProperty(sliceNumSamplesId, slice.lengthInSamples, nullptr);
    sliceTree.setProperty(sliceStartSampleId, slice.start, nullptr);
    sliceTree.setProperty(sliceEndSampleId, slice.end, nullptr);
    sliceTree.setProperty(sliceLoopStartSampleId, slice.loopStart, nullptr);
    sliceTree.setProperty(sliceLoopEndSampleId, slice.loopEnd, nullptr);
    sliceTree.setProperty(sliceAudioDataId, juce::var(createAudioDataBlock(slice)), nullptr);

    const auto newIndex = dataTree.getNumChildren();
    dataTree.addChild(sliceTree, -1, undoManager);
    dataTree.setProperty(selectedSliceId, newIndex, undoManager);

    return newIndex;
}

bool StateHandler::selectSlice(const int index, juce::UndoManager* undoManager)
{
    ensureDataTree();

    const auto selectedIndex = juce::isPositiveAndBelow(index, dataTree.getNumChildren()) ? index : -1;
    if (getSelectedSliceIndex() == selectedIndex)
        return false;

    dataTree.setProperty(selectedSliceId, selectedIndex, undoManager);
    return true;
}

int StateHandler::getSelectedSliceIndex() const
{
    if (! dataTree.isValid())
        return -1;

    const auto selectedIndex = static_cast<int>(dataTree.getProperty(selectedSliceId, -1));
    return juce::isPositiveAndBelow(selectedIndex, dataTree.getNumChildren()) ? selectedIndex : -1;
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
        return;

    settingsTree = juce::ValueTree(settingsId);
    valueTree.addChild(settingsTree, -1, nullptr);
}

void StateHandler::ensureDataTree()
{
    dataTree = valueTree.getChildWithName(dataId);

    if (dataTree.isValid())
    {
        if (! dataTree.hasProperty(selectedSliceId))
            dataTree.setProperty(selectedSliceId, -1, nullptr);

        return;
    }

    dataTree = juce::ValueTree(dataId);
    dataTree.setProperty(selectedSliceId, -1, nullptr);
    valueTree.addChild(dataTree, -1, nullptr);
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

juce::MemoryBlock StateHandler::createAudioDataBlock(const Slice& slice)
{
    const auto* audioData = slice.getAudioData();
    juce::MemoryOutputStream stream(static_cast<size_t>(audioData->getNumChannels())
                                    * static_cast<size_t>(audioData->getNumSamples())
                                    * sizeof(float));

    for (int channel = 0; channel < audioData->getNumChannels(); ++channel)
        stream.write(audioData->getReadPointer(channel), static_cast<size_t>(audioData->getNumSamples()) * sizeof(float));

    return stream.getMemoryBlock();
}
