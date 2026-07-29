#include "StateHandler.h"
#include "AudioUtil.h"
#include "AudioFileLoader.h"

static StateHandler::StateChange makeStateChange(const StateHandler::StateChange::Flag flags, const juce::Identifier& property)
{
    return { static_cast<uint32_t>(flags), property };
}

static StateHandler::StateChange makeStateChange(const StateHandler::StateChange::Flag flags)
{
    return makeStateChange(flags, {});
}

static StateHandler::StateChange makeStateChangeFromTreeEvent(const juce::ValueTree& settingsTree,
                                                              const juce::ValueTree& dataTree,
                                                              const juce::Identifier& sliceId,
                                                              const juce::Identifier& selectedSliceId,
                                                              const juce::ValueTree& changedTree,
                                                              const juce::Identifier& property)
{
    if (changedTree == settingsTree)
        return makeStateChange(StateHandler::StateChange::settings, property);

    if (changedTree == dataTree)
    {
        if (property == selectedSliceId)
            return makeStateChange(StateHandler::StateChange::selectedSlice, property);

        return makeStateChange(StateHandler::StateChange::sliceList, property);
    }

    if (changedTree.isValid() && changedTree.getParent() == dataTree)
    {
        if (! changedTree.hasType(sliceId))
            return makeStateChange(StateHandler::StateChange::fullReload, property);

        const auto selectedSliceIndex = static_cast<int>(dataTree.getProperty(selectedSliceId, -1));
        if (juce::isPositiveAndBelow(selectedSliceIndex, dataTree.getNumChildren()))
        {
            const auto selectedSliceTree = dataTree.getChild(selectedSliceIndex);
            if (changedTree == selectedSliceTree)
                return makeStateChange(StateHandler::StateChange::sliceList, property);
        }

        return makeStateChange(StateHandler::StateChange::sliceList, property);
    }

    return makeStateChange(StateHandler::StateChange::fullReload, property);
}

static StateHandler::StateChange makeStateChangeFromChildEvent(const juce::ValueTree& parent,
                                                               const juce::ValueTree& child,
                                                               const juce::Identifier& sliceId,
                                                               const juce::Identifier& settingsId,
                                                               const juce::Identifier& dataId)
{
    if (parent.hasType(dataId) && child.hasType(sliceId))
        return makeStateChange(StateHandler::StateChange::sliceList);

    if (parent.hasType(settingsId) || parent.hasType(dataId))
        return makeStateChange(StateHandler::StateChange::fullReload);

    return makeStateChange(StateHandler::StateChange::fullReload);
}

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
    setDefaultOption(channelsId);
    setDefaultOption(samplerateId);
    setDefaultOption(timestretchId);
    setDefaultOption(loopModeId);
    setDefaultOption(triqQuantId);
    setDefaultStateValue(gainId, gainValue.defaultValue);
    setDefaultStateValue(bpmId, bpmValue.defaultValue);
    setDefaultOption(normalizationId);
    setDefaultOption(fadeinId);
    setDefaultOption(fadeoutId);
    setDefaultOption(megabreakFadeinId);
    setDefaultOption(megabreakFadeoutId);
    setDefaultStateValue(otFileId, otFileDefault);
    setDefaultStateValue(evenGridId, evenGridDefault);
    setDefaultStateValue(embedMarkersId, embedMarkersDefault);
    setDefaultStateValue(chainMaxLengthId, chainMaxLengthValue.defaultValue);
    setDefaultStateValue(defaultAudioFolderId, juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName());
    setDefaultStateValue(defaultExportFolderId, juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName());
    setDefaultStateValue(defaultProjectFolderId, juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName());
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

juce::String StateHandler::getStateValue(const juce::Identifier& identifier, juce::String defaultValue)
{
    if (! settingsTree.isValid() || ! settingsTree.hasProperty(identifier))
        return defaultValue;

    return settingsTree.getProperty(identifier, defaultValue).toString();
}

juce::String StateHandler::getStateValue(const juce::Identifier& identifier, juce::String defaultValue) const
{
    if (! settingsTree.isValid() || ! settingsTree.hasProperty(identifier))
        return defaultValue;

    return settingsTree.getProperty(identifier, defaultValue).toString();
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
    notifyListeners(makeStateChange(StateHandler::StateChange::fullReload));
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
    if (identifier == bitDepthId)
        return bitDepthOptions;
    if (identifier == channelsId)
        return channelOptions;
    if (identifier == samplerateId)
        return samplerateOptions;
    if (identifier == timestretchId)
        return timestretchOptions;
    if (identifier == loopModeId)
        return loopOptions;
    if (identifier == triqQuantId)
        return trigQuantOptions;
    if (identifier == normalizationId)
        return normalizationOptions;
    if (identifier == fadeinId)
        return fadeinOptions;
    if (identifier == fadeoutId)
        return fadeoutOptions;
    if (identifier == megabreakFadeinId)
        return megabreakFadeinOptions;
    if (identifier == megabreakFadeoutId)
        return megabreakFadeoutOptions;
    if (identifier == megabreakFileCountId)
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

bool StateHandler::loadFullSelectedSliceAudio(juce::AudioBuffer<float>& destination, double& sampleRate) const
{
    const auto sliceTree = getSelectedSliceTree();
    if (! sliceTree.isValid())
        return false;

    const auto numChannels = static_cast<int>(sliceTree.getProperty(sliceChannelsId, 0));
    sampleRate = static_cast<double>(sliceTree.getProperty(sliceSamplerateId, 0.0));
    const auto numSamples = static_cast<juce::int64>(sliceTree.getProperty(sliceNumSamplesId, 0));
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

bool StateHandler::loadSelectedSliceRangeAudio(juce::AudioBuffer<float>& destination, double& sampleRate) const
{
    const auto sliceTree = getSelectedSliceTree();
    return loadSliceRangeAudio(sliceTree, destination, sampleRate);
}

bool StateHandler::loadSliceRangeAudio(const juce::ValueTree& sliceTree, juce::AudioBuffer<float>& destination,
                                       double& sampleRate)
{
    if (! sliceTree.isValid())
        return false;

    const auto numChannels = static_cast<int>(sliceTree.getProperty(sliceChannelsId, 0));
    sampleRate = static_cast<double>(sliceTree.getProperty(sliceSamplerateId, 0.0));
    const auto numSamples = static_cast<juce::int64>(sliceTree.getProperty(sliceNumSamplesId, 0));
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

    const auto rangeStart = juce::jlimit<juce::int64>(0, numSamples,
                                                      sliceTree.getProperty(sliceStartSampleId, 0));
    auto rangeEnd = juce::jlimit(rangeStart, numSamples,
                                 static_cast<juce::int64>(sliceTree.getProperty(sliceEndSampleId, numSamples)));
    if (rangeEnd <= rangeStart)
        rangeEnd = juce::jmin<juce::int64>(numSamples, rangeStart + 1);

    const auto rangeLength = rangeEnd - rangeStart;
    if (rangeLength <= 0)
        return false;

    const auto samplesPerChannel = static_cast<size_t>(numSamples);
    const auto expectedBytes = static_cast<size_t>(numChannels) * samplesPerChannel * sizeof(float);
    if (audioDataBlock->getSize() < expectedBytes)
        return false;

    const auto rangeLengthAsInt = static_cast<int>(rangeLength);
    const auto* samples = static_cast<const float*>(audioDataBlock->getData());

    destination.setSize(numChannels, rangeLengthAsInt, false, false, false);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        const auto* source = samples + static_cast<size_t>(channel) * samplesPerChannel + static_cast<size_t>(rangeStart);
        destination.copyFrom(channel, 0, source, rangeLengthAsInt);
    }

    return true;
}

int StateHandler::addSlice(const Slice& slice, juce::UndoManager* undoManager, const bool selectNewSlice)
{
    ensureDataTree();

    juce::ValueTree sliceTree(sliceId);
    sliceTree.setProperty(sliceNameId, slice.getName(), nullptr);
    sliceTree.setProperty(sliceSourcePathId, slice.sourcePath, nullptr);
    sliceTree.setProperty(sliceChannelsId, slice.channels, nullptr);
    sliceTree.setProperty(sliceSamplerateId, slice.samplerate, nullptr);
    sliceTree.setProperty(sliceBitrateId, static_cast<int>(slice.bitDepth), nullptr);
    sliceTree.setProperty(sliceNumSamplesId, slice.lengthInSamples, nullptr);
    sliceTree.setProperty(sliceStartSampleId, slice.start, nullptr);
    sliceTree.setProperty(sliceEndSampleId, slice.end, nullptr);
    sliceTree.setProperty(sliceLoopStartSampleId, slice.loopStart, nullptr);
    sliceTree.setProperty(sliceAudioDataId, juce::var(createAudioDataBlock(slice)), nullptr);

    const auto newIndex = dataTree.getNumChildren();
    dataTree.addChild(sliceTree, -1, undoManager);

    if (selectNewSlice)
        dataTree.setProperty(selectedSliceId, newIndex, undoManager);

    return newIndex;
}

int StateHandler::addBlankSlice(const int64 lengthInSamples, juce::UndoManager* undoManager)
{
    ensureDataTree();
    Slice slice;
    slice.setName("Blank");
    slice.sourcePath = {"N/A"};
    slice.channels = 1;
    slice.samplerate = 44100;
    slice.bitDepth = 16;
    slice.lengthInSamples = lengthInSamples;
    slice.start = 0;
    slice.end = slice.lengthInSamples;
    slice.loopStart = 0;
    slice.createBlankAudioData(slice.channels, lengthInSamples);
    return addSlice(slice, undoManager);
}

bool StateHandler::cloneSelectedSlice(juce::UndoManager* undoManager)
{
    ensureDataTree();

    const auto selectedIndex = getSelectedSliceIndex();
    if (selectedIndex < 0)
        return false;

    const auto selectedSliceTree = getSelectedSliceTree();
    if (! selectedSliceTree.isValid())
        return false;

    const auto currentName = selectedSliceTree.getProperty(sliceNameId).toString().trim();
    auto clonedName = currentName.isEmpty() ? juce::String("Copy") : currentName + " (copy)";
    clonedName = clonedName.substring(0, Slice::maxNameLength);

    const auto insertionIndex = selectedIndex + 1;
    auto clonedSliceTree = selectedSliceTree.createCopy();
    clonedSliceTree.setProperty(sliceNameId, clonedName, nullptr);
    dataTree.addChild(clonedSliceTree, insertionIndex, undoManager);
    dataTree.setProperty(selectedSliceId, insertionIndex, undoManager);

    return true;
}

void StateHandler::removeSelectedSlice()
{
    ensureDataTree();

    const auto selectedIndex = getSelectedSliceIndex();
    if (selectedIndex < 0)
        return;

    dataTree.removeChild(selectedIndex, nullptr);

    const auto newSelectedIndex = juce::jmin(selectedIndex, dataTree.getNumChildren() - 1);
    dataTree.setProperty(selectedSliceId, newSelectedIndex, nullptr);
}

void StateHandler::removeAllSlices()
{
    ensureDataTree();

    dataTree.removeAllChildren(nullptr);
    dataTree.setProperty(selectedSliceId, -1, nullptr);
}

bool StateHandler::cropSelectedSliceToRange(juce::UndoManager* undoManager)
{
    ensureDataTree();

    auto sliceTree = getSelectedSliceTree();
    if (! sliceTree.isValid())
        return false;

    const auto totalSamples = static_cast<juce::int64>(sliceTree.getProperty(sliceNumSamplesId, 0));
    if (totalSamples <= 0)
        return false;

    const auto rangeStart = juce::jlimit<juce::int64>(0, totalSamples,
                                                      sliceTree.getProperty(sliceStartSampleId, 0));
    auto rangeEnd = juce::jlimit(rangeStart, totalSamples,
                                 static_cast<juce::int64>(sliceTree.getProperty(sliceEndSampleId, totalSamples)));
    if (rangeEnd <= rangeStart)
        rangeEnd = juce::jmin<juce::int64>(totalSamples, rangeStart + 1);

    const auto croppedLength = rangeEnd - rangeStart;
    if (croppedLength <= 0 || (rangeStart <= 0 && rangeEnd >= totalSamples))
        return false;

    juce::AudioBuffer<float> croppedAudio;
    double sampleRate = 0.0;
    if (! loadSelectedSliceRangeAudio(croppedAudio, sampleRate))
        return false;

    Slice croppedSlice;
    croppedSlice.setName(sliceTree.getProperty(sliceNameId).toString());
    croppedSlice.sourcePath = sliceTree.getProperty(sliceSourcePathId).toString();
    croppedSlice.channels = croppedAudio.getNumChannels();
    croppedSlice.samplerate = sampleRate;
    croppedSlice.bitDepth = static_cast<unsigned int>(static_cast<int>(sliceTree.getProperty(sliceBitrateId, 0)));
    croppedSlice.lengthInSamples = croppedAudio.getNumSamples();
    croppedSlice.start = 0;
    croppedSlice.end = croppedSlice.lengthInSamples;
    croppedSlice.loopStart = juce::jlimit<juce::int64>(0,
                                                       croppedSlice.lengthInSamples,
                                                       static_cast<juce::int64>(sliceTree.getProperty(sliceLoopStartSampleId, 0)) - rangeStart);

    auto* audioData = croppedSlice.getAudioData();
    audioData->setSize(croppedSlice.channels, static_cast<int>(croppedSlice.lengthInSamples), false, false, false);
    audioData->clear();

    for (int channel = 0; channel < croppedSlice.channels; ++channel)
        audioData->copyFrom(channel, 0, croppedAudio, channel, 0, static_cast<int>(croppedSlice.lengthInSamples));

    sliceTree.setProperty(sliceNameId, croppedSlice.getName(), undoManager);
    sliceTree.setProperty(sliceSourcePathId, croppedSlice.sourcePath, undoManager);
    sliceTree.setProperty(sliceChannelsId, croppedSlice.channels, undoManager);
    sliceTree.setProperty(sliceSamplerateId, croppedSlice.samplerate, undoManager);
    sliceTree.setProperty(sliceBitrateId, static_cast<int>(croppedSlice.bitDepth), undoManager);
    sliceTree.setProperty(sliceNumSamplesId, croppedSlice.lengthInSamples, undoManager);
    sliceTree.setProperty(sliceStartSampleId, croppedSlice.start, undoManager);
    sliceTree.setProperty(sliceEndSampleId, croppedSlice.end, undoManager);
    sliceTree.setProperty(sliceLoopStartSampleId, croppedSlice.loopStart, undoManager);
    sliceTree.setProperty(sliceAudioDataId, juce::var(createAudioDataBlock(croppedSlice)), undoManager);

    return true;
}

bool StateHandler::divideSelectedSliceEvenly(const int sliceCount, juce::UndoManager* undoManager, juce::String* errorMessage)
{
    ensureDataTree();

    const auto fail = [errorMessage](const juce::String& message)
    {
        if (errorMessage != nullptr)
            *errorMessage = message;
        return false;
    };

    if (sliceCount < 2 || sliceCount > 64)
        return fail("The slice count must be between 2 and 64.");

    const auto selectedIndex = getSelectedSliceIndex();
    if (selectedIndex < 0)
        return fail("No slice is selected for dividing.");

    const auto selectedSliceTree = getSelectedSliceTree();
    if (! selectedSliceTree.isValid())
        return fail("The selected slice could not be read.");

    const auto totalSamples = static_cast<juce::int64>(selectedSliceTree.getProperty(sliceNumSamplesId, 0));
    if (totalSamples <= 0)
        return fail("The selected slice does not contain usable audio data.");

    const auto rangeStart = juce::jlimit<juce::int64>(0, totalSamples,
                                                      selectedSliceTree.getProperty(sliceStartSampleId, 0));
    auto rangeEnd = juce::jlimit(rangeStart, totalSamples,
                                 static_cast<juce::int64>(selectedSliceTree.getProperty(sliceEndSampleId, totalSamples)));
    if (rangeEnd <= rangeStart)
        rangeEnd = juce::jmin<juce::int64>(totalSamples, rangeStart + 1);

    const auto rangeLength = rangeEnd - rangeStart;
    if (rangeLength < sliceCount)
        return fail("The selected range is too short to divide into that many slices.");

    juce::AudioBuffer<float> selectedRangeAudio;
    double sampleRate = 0.0;
    if (! loadSelectedSliceRangeAudio(selectedRangeAudio, sampleRate))
        return fail("The selected slice could not be loaded.");

    const auto numChannels = selectedRangeAudio.getNumChannels();
    const auto numSamples = selectedRangeAudio.getNumSamples();
    if (numChannels <= 0 || numSamples <= 0)
        return fail("The selected slice does not contain usable audio data.");

    const auto baseLength = rangeLength / sliceCount;
    const auto remainder = rangeLength % sliceCount;
    const auto sourcePath = selectedSliceTree.getProperty(sliceSourcePathId).toString();
    const auto bitDepth = static_cast<unsigned int>(static_cast<int>(selectedSliceTree.getProperty(sliceBitrateId, 0)));
    const auto loopStartInSelectedRange = juce::jlimit<juce::int64>(0,
                                                                    rangeLength,
                                                                    static_cast<juce::int64>(selectedSliceTree.getProperty(sliceLoopStartSampleId, 0)) - rangeStart);
    auto nameBase = selectedSliceTree.getProperty(sliceNameId).toString().trim();
    if (nameBase.isEmpty())
        nameBase = "Slice";

    std::vector<juce::ValueTree> newSliceTrees;
    newSliceTrees.reserve(static_cast<std::size_t>(sliceCount));

    juce::int64 pieceStart = 0;
    for (int i = 0; i < sliceCount; ++i)
    {
        const auto pieceLength = baseLength + (i < remainder ? 1 : 0);
        if (pieceLength <= 0)
            return fail("The selected range could not be divided evenly.");

        Slice splitSlice;
        splitSlice.setName(nameBase + " " + juce::String(i + 1));
        splitSlice.sourcePath = sourcePath;
        splitSlice.channels = numChannels;
        splitSlice.samplerate = sampleRate;
        splitSlice.bitDepth = bitDepth;
        splitSlice.lengthInSamples = pieceLength;
        splitSlice.start = 0;
        splitSlice.end = pieceLength;
        splitSlice.loopStart = juce::jlimit<juce::int64>(0, pieceLength, loopStartInSelectedRange - pieceStart);

        auto* destinationAudio = splitSlice.getAudioData();
        destinationAudio->setSize(numChannels, static_cast<int>(pieceLength), false, false, false);
        destinationAudio->clear();

        for (int channel = 0; channel < numChannels; ++channel)
            destinationAudio->copyFrom(channel,
                                       0,
                                       selectedRangeAudio,
                                       channel,
                                       static_cast<int>(pieceStart),
                                       static_cast<int>(pieceLength));

        juce::ValueTree splitSliceTree(sliceId);
        splitSliceTree.setProperty(sliceNameId, splitSlice.getName(), nullptr);
        splitSliceTree.setProperty(sliceSourcePathId, splitSlice.sourcePath, nullptr);
        splitSliceTree.setProperty(sliceChannelsId, splitSlice.channels, nullptr);
        splitSliceTree.setProperty(sliceSamplerateId, splitSlice.samplerate, nullptr);
        splitSliceTree.setProperty(sliceBitrateId, static_cast<int>(splitSlice.bitDepth), nullptr);
        splitSliceTree.setProperty(sliceNumSamplesId, splitSlice.lengthInSamples, nullptr);
        splitSliceTree.setProperty(sliceStartSampleId, splitSlice.start, nullptr);
        splitSliceTree.setProperty(sliceEndSampleId, splitSlice.end, nullptr);
        splitSliceTree.setProperty(sliceLoopStartSampleId, splitSlice.loopStart, nullptr);
        splitSliceTree.setProperty(sliceAudioDataId, juce::var(createAudioDataBlock(splitSlice)), nullptr);
        newSliceTrees.push_back(splitSliceTree);

        pieceStart += pieceLength;
    }

    dataTree.removeChild(selectedIndex, undoManager);

    for (int i = 0; i < static_cast<int>(newSliceTrees.size()); ++i)
        dataTree.addChild(newSliceTrees[static_cast<std::size_t>(i)], selectedIndex + i, undoManager);

    dataTree.setProperty(selectedSliceId, selectedIndex, undoManager);

    return true;
}

bool StateHandler::divideSelectedSliceByBpm(const double bpm, const double sixteenthNotesPerSlice,
                                            juce::UndoManager* undoManager, juce::String* errorMessage)
{
    ensureDataTree();

    const auto fail = [errorMessage](const juce::String& message)
    {
        if (errorMessage != nullptr)
            *errorMessage = message;
        return false;
    };

    if (bpm < 30.0 || bpm > 300.0)
        return fail("The BPM must be between 30 and 300.");

    if (sixteenthNotesPerSlice <= 0.0)
        return fail("The sixteenth note count must be greater than 0.");

    const auto selectedIndex = getSelectedSliceIndex();
    if (selectedIndex < 0)
        return fail("No slice is selected for dividing.");

    const auto selectedSliceTree = getSelectedSliceTree();
    if (! selectedSliceTree.isValid())
        return fail("The selected slice could not be read.");

    const auto totalSamples = static_cast<juce::int64>(selectedSliceTree.getProperty(sliceNumSamplesId, 0));
    if (totalSamples <= 0)
        return fail("The selected slice does not contain usable audio data.");

    const auto rangeStart = juce::jlimit<juce::int64>(0, totalSamples,
                                                      selectedSliceTree.getProperty(sliceStartSampleId, 0));
    auto rangeEnd = juce::jlimit(rangeStart, totalSamples,
                                 static_cast<juce::int64>(selectedSliceTree.getProperty(sliceEndSampleId, totalSamples)));
    if (rangeEnd <= rangeStart)
        rangeEnd = juce::jmin<juce::int64>(totalSamples, rangeStart + 1);

    const auto rangeLength = rangeEnd - rangeStart;
    if (rangeLength <= 0)
        return fail("The selected slice does not contain a usable range.");

    juce::AudioBuffer<float> selectedRangeAudio;
    double sampleRate = 0.0;
    if (! loadSelectedSliceRangeAudio(selectedRangeAudio, sampleRate))
        return fail("The selected slice could not be loaded.");

    const auto numChannels = selectedRangeAudio.getNumChannels();
    const auto numSamples = selectedRangeAudio.getNumSamples();
    if (numChannels <= 0 || numSamples <= 0)
        return fail("The selected slice does not contain usable audio data.");

    const auto targetSamplesExact = sampleRate * (60.0 / bpm) * (sixteenthNotesPerSlice / 4.0);
    const auto targetSamples = juce::roundToInt(targetSamplesExact);
    if (targetSamples <= 0)
        return fail("The BPM and slice length would produce an invalid slice size.");

    const auto fullSliceCount = static_cast<int>(rangeLength / targetSamples);
    const auto tailLength = static_cast<int>(rangeLength % targetSamples);
    const auto sliceTotalCount = fullSliceCount + (tailLength > 0 ? 1 : 0);
    if (sliceTotalCount <= 0)
        return fail("The selected range could not be divided.");

    const auto sourcePath = selectedSliceTree.getProperty(sliceSourcePathId).toString();
    const auto bitDepth = static_cast<unsigned int>(static_cast<int>(selectedSliceTree.getProperty(sliceBitrateId, 0)));
    const auto loopStartInSelectedRange = juce::jlimit<juce::int64>(0,
                                                                    rangeLength,
                                                                    static_cast<juce::int64>(selectedSliceTree.getProperty(sliceLoopStartSampleId, 0)) - rangeStart);
    auto nameBase = selectedSliceTree.getProperty(sliceNameId).toString().trim();
    if (nameBase.isEmpty())
        nameBase = "Slice";

    std::vector<juce::ValueTree> newSliceTrees;
    newSliceTrees.reserve(static_cast<std::size_t>(sliceTotalCount));

    juce::int64 pieceStart = 0;
    for (int i = 0; i < fullSliceCount; ++i)
    {
        Slice splitSlice;
        splitSlice.setName(nameBase + " " + juce::String(i + 1));
        splitSlice.sourcePath = sourcePath;
        splitSlice.channels = numChannels;
        splitSlice.samplerate = sampleRate;
        splitSlice.bitDepth = bitDepth;
        splitSlice.lengthInSamples = targetSamples;
        splitSlice.start = 0;
        splitSlice.end = targetSamples;
        splitSlice.loopStart = juce::jlimit<juce::int64>(0, splitSlice.lengthInSamples, loopStartInSelectedRange - pieceStart);

        auto* destinationAudio = splitSlice.getAudioData();
        destinationAudio->setSize(numChannels, targetSamples, false, false, false);
        destinationAudio->clear();

        for (int channel = 0; channel < numChannels; ++channel)
            destinationAudio->copyFrom(channel, 0, selectedRangeAudio, channel, static_cast<int>(pieceStart), targetSamples);

        juce::ValueTree splitSliceTree(sliceId);
        splitSliceTree.setProperty(sliceNameId, splitSlice.getName(), nullptr);
        splitSliceTree.setProperty(sliceSourcePathId, splitSlice.sourcePath, nullptr);
        splitSliceTree.setProperty(sliceChannelsId, splitSlice.channels, nullptr);
        splitSliceTree.setProperty(sliceSamplerateId, splitSlice.samplerate, nullptr);
        splitSliceTree.setProperty(sliceBitrateId, static_cast<int>(splitSlice.bitDepth), nullptr);
        splitSliceTree.setProperty(sliceNumSamplesId, splitSlice.lengthInSamples, nullptr);
        splitSliceTree.setProperty(sliceStartSampleId, splitSlice.start, nullptr);
        splitSliceTree.setProperty(sliceEndSampleId, splitSlice.end, nullptr);
        splitSliceTree.setProperty(sliceLoopStartSampleId, splitSlice.loopStart, nullptr);
        splitSliceTree.setProperty(sliceAudioDataId, juce::var(createAudioDataBlock(splitSlice)), nullptr);
        newSliceTrees.push_back(splitSliceTree);

        pieceStart += targetSamples;
    }

    if (tailLength > 0)
    {
        const auto tailStart = pieceStart;

        Slice tailSlice;
        tailSlice.setName(nameBase + " (tail)");
        tailSlice.sourcePath = sourcePath;
        tailSlice.channels = numChannels;
        tailSlice.samplerate = sampleRate;
        tailSlice.bitDepth = bitDepth;
        tailSlice.lengthInSamples = tailLength;
        tailSlice.start = 0;
        tailSlice.end = tailLength;
        tailSlice.loopStart = juce::jlimit<juce::int64>(0, tailSlice.lengthInSamples, loopStartInSelectedRange - tailStart);

        auto* destinationAudio = tailSlice.getAudioData();
        destinationAudio->setSize(numChannels, tailLength, false, false, false);
        destinationAudio->clear();

        for (int channel = 0; channel < numChannels; ++channel)
            destinationAudio->copyFrom(channel, 0, selectedRangeAudio, channel, static_cast<int>(tailStart), tailLength);

        juce::ValueTree tailSliceTree(sliceId);
        tailSliceTree.setProperty(sliceNameId, tailSlice.getName(), nullptr);
        tailSliceTree.setProperty(sliceSourcePathId, tailSlice.sourcePath, nullptr);
        tailSliceTree.setProperty(sliceChannelsId, tailSlice.channels, nullptr);
        tailSliceTree.setProperty(sliceSamplerateId, tailSlice.samplerate, nullptr);
        tailSliceTree.setProperty(sliceBitrateId, static_cast<int>(tailSlice.bitDepth), nullptr);
        tailSliceTree.setProperty(sliceNumSamplesId, tailSlice.lengthInSamples, nullptr);
        tailSliceTree.setProperty(sliceStartSampleId, tailSlice.start, nullptr);
        tailSliceTree.setProperty(sliceEndSampleId, tailSlice.end, nullptr);
        tailSliceTree.setProperty(sliceLoopStartSampleId, tailSlice.loopStart, nullptr);
        tailSliceTree.setProperty(sliceAudioDataId, juce::var(createAudioDataBlock(tailSlice)), nullptr);
        newSliceTrees.push_back(tailSliceTree);
    }

    dataTree.removeChild(selectedIndex, undoManager);

    for (int i = 0; i < static_cast<int>(newSliceTrees.size()); ++i)
        dataTree.addChild(newSliceTrees[static_cast<std::size_t>(i)], selectedIndex + i, undoManager);

    dataTree.setProperty(selectedSliceId, selectedIndex, undoManager);

    return true;
}

bool StateHandler::normalizeSelectedSlice(juce::UndoManager* undoManager)
{
    ensureDataTree();

    auto sliceTree = getSelectedSliceTree();
    if (! sliceTree.isValid())
        return false;

    juce::AudioBuffer<float> audioData;
    double sampleRate = 0.0;
    if (! loadFullSelectedSliceAudio(audioData, sampleRate))
        return false;

    const auto numChannels = audioData.getNumChannels();
    const auto numSamples = audioData.getNumSamples();
    if (numChannels <= 0 || numSamples <= 0)
        return false;

    AudioUtil::normalizeAudioBuffer(audioData);

    Slice normalizedSlice;
    normalizedSlice.setName(sliceTree.getProperty(sliceNameId).toString());
    normalizedSlice.sourcePath = sliceTree.getProperty(sliceSourcePathId).toString();
    normalizedSlice.channels = numChannels;
    normalizedSlice.samplerate = sampleRate;
    normalizedSlice.bitDepth = static_cast<unsigned int>(static_cast<int>(sliceTree.getProperty(sliceBitrateId, 0)));
    normalizedSlice.lengthInSamples = numSamples;
    normalizedSlice.start = static_cast<juce::int64>(sliceTree.getProperty(sliceStartSampleId, 0));
    normalizedSlice.end = static_cast<juce::int64>(sliceTree.getProperty(sliceEndSampleId, numSamples));
    normalizedSlice.loopStart = static_cast<juce::int64>(sliceTree.getProperty(sliceLoopStartSampleId, 0));

    auto* destinationAudio = normalizedSlice.getAudioData();
    destinationAudio->setSize(numChannels, numSamples, false, false, false);
    destinationAudio->clear();

    for (int channel = 0; channel < numChannels; ++channel)
        destinationAudio->copyFrom(channel, 0, audioData, channel, 0, numSamples);

    sliceTree.setProperty(sliceAudioDataId, juce::var(createAudioDataBlock(normalizedSlice)), undoManager);

    return true;
}

bool StateHandler::mergeSelectedSliceWithSliceAbove(juce::UndoManager* undoManager, juce::String* errorMessage)
{
    ensureDataTree();

    const auto fail = [errorMessage](const juce::String& message)
    {
        if (errorMessage != nullptr)
            *errorMessage = message;
        return false;
    };

    const auto selectedIndex = getSelectedSliceIndex();
    if (selectedIndex <= 0)
        return fail("No slice is selected for merging.");

    const auto aboveIndex = selectedIndex - 1;
    auto aboveSliceTree = getSliceTree(aboveIndex);
    auto selectedSliceTree = getSelectedSliceTree();
    if (! aboveSliceTree.isValid() || ! selectedSliceTree.isValid())
        return fail("The selected slices could not be read.");

    juce::AudioBuffer<float> aboveAudio;
    juce::AudioBuffer<float> selectedAudio;
    double aboveSampleRate = 0.0;
    double selectedSampleRate = 0.0;

    if (! loadSliceRangeAudio(aboveSliceTree, aboveAudio, aboveSampleRate)
        || ! loadSliceRangeAudio(selectedSliceTree, selectedAudio, selectedSampleRate))
    {
        return fail("One of the slices could not be loaded for merging.");
    }

    const auto aboveChannels = aboveAudio.getNumChannels();
    const auto selectedChannels = selectedAudio.getNumChannels();
    const auto targetChannelCount = juce::jmax(aboveChannels, selectedChannels);
    const auto targetSampleRate = juce::jmax(aboveSampleRate, selectedSampleRate);
    if (targetChannelCount <= 0 || targetSampleRate <= 0.0)
        return fail("The selected slices do not contain usable audio data.");

    juce::AudioBuffer<float> aboveResampled;
    juce::AudioBuffer<float> selectedResampled;
    juce::AudioBuffer<float> aboveRendered;
    juce::AudioBuffer<float> selectedRendered;
    if (! AudioUtil::resampleAudioBuffer(aboveAudio, aboveSampleRate, targetSampleRate, aboveResampled)
        || ! AudioUtil::renderAudioBufferToChannelCount(aboveResampled, targetChannelCount, aboveRendered)
        || ! AudioUtil::resampleAudioBuffer(selectedAudio, selectedSampleRate, targetSampleRate, selectedResampled)
        || ! AudioUtil::renderAudioBufferToChannelCount(selectedResampled, targetChannelCount, selectedRendered))
        return fail("The selected slices could not be resampled for merging.");

    const auto mergedSamples = aboveRendered.getNumSamples() + selectedRendered.getNumSamples();
    if (mergedSamples <= 0)
        return fail("The merged slice would be empty.");

    const auto estimatedAudioBytes = static_cast<juce::int64>(targetChannelCount)
                                     * static_cast<juce::int64>(mergedSamples)
                                     * static_cast<juce::int64>(sizeof(float));
    if (estimatedAudioBytes > AudioFileLoader::maxLoadedAudioDataBytes)
    {
        const auto estimatedAudioMb = static_cast<double>(estimatedAudioBytes) / (1024.0 * 1024.0);
        return fail("The merged slice would require about "
                    + juce::String(estimatedAudioMb, 1)
                    + " MB of audio data, which exceeds the "
                    + juce::String(AudioFileLoader::maxLoadedAudioDataBytes / (1024 * 1024))
                    + " MB limit.");
    }

    juce::AudioBuffer<float> mergedAudio;
    mergedAudio.setSize(targetChannelCount, mergedSamples, false, false, true);
    mergedAudio.clear();

    const auto appendBuffer = [&mergedAudio](const juce::AudioBuffer<float>& source, int destStartSample)
    {
        const auto sourceChannels = source.getNumChannels();
        const auto sourceSamples = source.getNumSamples();
        if (sourceChannels <= 0 || sourceSamples <= 0)
            return;

        for (int channel = 0; channel < sourceChannels; ++channel)
            mergedAudio.copyFrom(channel, destStartSample, source, channel, 0, sourceSamples);
    };

    appendBuffer(aboveRendered, 0);
    appendBuffer(selectedRendered, aboveRendered.getNumSamples());

    Slice mergedSlice;
    const auto aboveName = aboveSliceTree.getProperty(sliceNameId).toString().trim();
    const auto selectedName = selectedSliceTree.getProperty(sliceNameId).toString().trim();
    mergedSlice.setName(selectedName + " (merged)");
    mergedSlice.sourcePath = aboveSliceTree.getProperty(sliceSourcePathId).toString();
    if (mergedSlice.sourcePath.isEmpty() || mergedSlice.sourcePath != selectedSliceTree.getProperty(sliceSourcePathId).toString())
        mergedSlice.sourcePath = "Merged";
    mergedSlice.channels = targetChannelCount;
    mergedSlice.samplerate = targetSampleRate;
    mergedSlice.bitDepth = static_cast<unsigned int>(juce::jmax(static_cast<int>(aboveSliceTree.getProperty(sliceBitrateId, 0)),
                                                                 static_cast<int>(selectedSliceTree.getProperty(sliceBitrateId, 0))));
    mergedSlice.lengthInSamples = mergedSamples;
    mergedSlice.start = 0;
    mergedSlice.end = mergedSlice.lengthInSamples;
    mergedSlice.loopStart = 0;

    auto* destinationAudio = mergedSlice.getAudioData();
    destinationAudio->setSize(targetChannelCount, mergedSamples, false, false, false);
    destinationAudio->clear();

    for (int channel = 0; channel < targetChannelCount; ++channel)
        destinationAudio->copyFrom(channel, 0, mergedAudio, channel, 0, static_cast<int>(mergedSamples));

    juce::ValueTree mergedTree(sliceId);
    mergedTree.setProperty(sliceNameId, mergedSlice.getName(), nullptr);
    mergedTree.setProperty(sliceSourcePathId, mergedSlice.sourcePath, nullptr);
    mergedTree.setProperty(sliceChannelsId, mergedSlice.channels, nullptr);
    mergedTree.setProperty(sliceSamplerateId, mergedSlice.samplerate, nullptr);
    mergedTree.setProperty(sliceBitrateId, static_cast<int>(mergedSlice.bitDepth), nullptr);
    mergedTree.setProperty(sliceNumSamplesId, mergedSlice.lengthInSamples, nullptr);
    mergedTree.setProperty(sliceStartSampleId, mergedSlice.start, nullptr);
    mergedTree.setProperty(sliceEndSampleId, mergedSlice.end, nullptr);
    mergedTree.setProperty(sliceLoopStartSampleId, mergedSlice.loopStart, nullptr);
    mergedTree.setProperty(sliceAudioDataId, juce::var(createAudioDataBlock(mergedSlice)), nullptr);

    dataTree.removeChild(selectedIndex, undoManager);
    dataTree.removeChild(aboveIndex, undoManager);
    dataTree.addChild(mergedTree, aboveIndex, undoManager);
    dataTree.setProperty(selectedSliceId, aboveIndex, undoManager);

    return true;
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

bool StateHandler::setSelectedSliceRange(const int startSample, const int endSample, juce::UndoManager* undoManager)
{
    ensureDataTree();

    auto sliceTree = getSelectedSliceTree();
    if (! sliceTree.isValid())
        return false;

    const auto totalSamples = static_cast<juce::int64>(sliceTree.getProperty(sliceNumSamplesId, 0));
    if (totalSamples <= 0)
        return false;

    auto clampedStart = juce::jlimit<juce::int64>(0, totalSamples, startSample);
    auto clampedEnd = juce::jlimit<juce::int64>(0, totalSamples, endSample);

    if (clampedEnd <= clampedStart)
    {
        if (clampedStart >= totalSamples)
            clampedStart = juce::jmax<juce::int64>(0, totalSamples - 1);

        clampedEnd = juce::jmin<juce::int64>(totalSamples, clampedStart + 1);
    }

    const auto currentStart = static_cast<juce::int64>(sliceTree.getProperty(sliceStartSampleId, 0));
    const auto currentEnd = static_cast<juce::int64>(sliceTree.getProperty(sliceEndSampleId, totalSamples));
    if (currentStart == clampedStart && currentEnd == clampedEnd)
        return false;

    sliceTree.setProperty(sliceStartSampleId, clampedStart, undoManager);
    sliceTree.setProperty(sliceEndSampleId, clampedEnd, undoManager);
    return true;
}

int StateHandler::getSelectedSliceIndex() const
{
    if (! dataTree.isValid())
        return -1;

    const auto selectedIndex = static_cast<int>(dataTree.getProperty(selectedSliceId, -1));
    return juce::isPositiveAndBelow(selectedIndex, dataTree.getNumChildren()) ? selectedIndex : -1;
}

bool StateHandler::moveSlice(const int fromIndex, const int toIndex)
{
    ensureDataTree();

    if (! juce::isPositiveAndBelow(fromIndex, dataTree.getNumChildren()) ||
        toIndex < 0 ||
        toIndex > dataTree.getNumChildren() ||
        fromIndex == toIndex)
        return false;

    const auto sliceTree = dataTree.getChild(fromIndex);
    dataTree.removeChild(fromIndex, nullptr);

    const auto insertionIndex = toIndex > fromIndex ? toIndex - 1 : toIndex;
    dataTree.addChild(sliceTree, insertionIndex, nullptr);

    const auto selectedIndex = getSelectedSliceIndex();
    if (selectedIndex == fromIndex)
        dataTree.setProperty(selectedSliceId, insertionIndex, nullptr);
    else if (selectedIndex > fromIndex && selectedIndex < toIndex)
        dataTree.setProperty(selectedSliceId, selectedIndex - 1, nullptr);
    else if (selectedIndex >= toIndex && selectedIndex < fromIndex)
        dataTree.setProperty(selectedSliceId, selectedIndex + 1, nullptr);

    return true;
}

void StateHandler::notifyListeners(const StateChange& change)
{
    listeners.call([&change](Listener& listener) { listener.stateChanged(change); });
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
        setDefaultStateValue(channelsId, channelOptions.front().value);
        setDefaultStateValue(samplerateId, samplerateOptions.front().value);
        setDefaultStateValue(megabreakFadeinId, megabreakFadeinOptions.front().value);
        setDefaultStateValue(megabreakFadeoutId, megabreakFadeoutOptions.front().value);
        setDefaultStateValue(defaultAudioFolderId, juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName());
        setDefaultStateValue(defaultExportFolderId, juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName());
        setDefaultStateValue(defaultProjectFolderId, juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName());
        return;
    }

    settingsTree = juce::ValueTree(settingsId);
    setDefaultStateValue(channelsId, channelOptions.front().value);
    setDefaultStateValue(samplerateId, samplerateOptions.front().value);
    setDefaultStateValue(megabreakFadeinId, megabreakFadeinOptions.front().value);
    setDefaultStateValue(megabreakFadeoutId, megabreakFadeoutOptions.front().value);
    setDefaultStateValue(defaultAudioFolderId, juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName());
    setDefaultStateValue(defaultExportFolderId, juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName());
    setDefaultStateValue(defaultProjectFolderId, juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName());
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

void StateHandler::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    notifyListeners(makeStateChangeFromTreeEvent(settingsTree, dataTree, sliceId, selectedSliceId, tree, property));
}

void StateHandler::valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& child)
{
    notifyListeners(makeStateChangeFromChildEvent(parent, child, sliceId, settingsId, dataId));
}

void StateHandler::valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int)
{
    notifyListeners(makeStateChangeFromChildEvent(parent, child, sliceId, settingsId, dataId));
}

void StateHandler::valueTreeChildOrderChanged(juce::ValueTree& parent, int, int)
{
    notifyListeners(parent.hasType(dataId)
                    ? makeStateChange(StateHandler::StateChange::sliceList)
                    : makeStateChange(StateHandler::StateChange::fullReload));
}

void StateHandler::valueTreeParentChanged(juce::ValueTree&)
{
    notifyListeners(makeStateChange(StateHandler::StateChange::fullReload));
}

void StateHandler::valueTreeRedirected(juce::ValueTree&)
{
    notifyListeners(makeStateChange(StateHandler::StateChange::fullReload));
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
