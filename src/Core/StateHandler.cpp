#include "StateHandler.h"

namespace
{
    const juce::Identifier stateTypeId { "octaChainerState" };
    const juce::Identifier normalizationModePropertyId { "normalizationMode" };
}

StateHandler::StateHandler()
    : valueTree(stateTypeId)
{
    valueTree.setProperty(normalizationModePropertyId,
                          static_cast<int>(NormalizationMode::None),
                          nullptr);
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

StateHandler::NormalizationMode StateHandler::getNormalizationMode() const
{
    return static_cast<NormalizationMode>(static_cast<int>(valueTree.getProperty(normalizationModePropertyId,
                                                                                  static_cast<int>(NormalizationMode::None))));
}

bool StateHandler::setNormalizationMode(NormalizationMode mode, juce::UndoManager* undoManager)
{
    if (getNormalizationMode() == mode)
        return false;

    valueTree.setProperty(normalizationModePropertyId, static_cast<int>(mode), undoManager);
    return true;
}

std::vector<StateHandler::NormalizationOption> StateHandler::getNormalizationOptions()
{
    return
    {
        { NormalizationMode::None, getNormalizationLabel(NormalizationMode::None) },
        { NormalizationMode::Slice, getNormalizationLabel(NormalizationMode::Slice) },
        { NormalizationMode::Chain, getNormalizationLabel(NormalizationMode::Chain) }
    };
}

juce::String StateHandler::getNormalizationLabel(const NormalizationMode mode)
{
    switch (mode)
    {
        case NormalizationMode::None:  return "No normalization";
        case NormalizationMode::Slice: return "Normalize slices";
        case NormalizationMode::Chain: return "Normalize chain";
    }
    return "No normalization";
}

int StateHandler::getNormalizationItemId(NormalizationMode mode)
{
    return static_cast<int>(mode) + 1;
}

StateHandler::NormalizationMode StateHandler::normalizationModeFromItemId(int itemId)
{
    if (itemId <= 0)
        return NormalizationMode::None;

    return static_cast<NormalizationMode>(itemId - 1);
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
