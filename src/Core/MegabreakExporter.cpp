#include "MegabreakExporter.h"
#include "ChainExporter.h"

juce::Range<juce::int64> MegabreakExporter::getPartRange(const juce::ValueTree& sliceTree,
                                                         const int partIndex,
                                                         const int partCount)
{
    const auto numSamples = static_cast<juce::int64>(sliceTree.getProperty(StateHandler::sliceNumSamplesId, 0));
    if (numSamples <= 0 || partCount <= 0)
        return {};

    const auto rangeStart = juce::jlimit<juce::int64>(0, numSamples, sliceTree.getProperty(StateHandler::sliceStartSampleId, 0));
    auto rangeEnd = juce::jlimit(rangeStart, numSamples,
                                 static_cast<juce::int64>(sliceTree.getProperty(StateHandler::sliceEndSampleId, numSamples)));
    if (rangeEnd <= rangeStart)
        rangeEnd = juce::jmin<juce::int64>(numSamples, rangeStart + 1);

    const auto trimmedLength = rangeEnd - rangeStart;
    if (trimmedLength <= 0)
        return { rangeStart, juce::jmin<juce::int64>(numSamples, rangeStart + 1) };

    const auto partStartOffset = (trimmedLength * partIndex) / partCount;
    auto partEndOffset = partIndex == partCount - 1
        ? trimmedLength
        : (trimmedLength * (partIndex + 1)) / partCount;

    if (partEndOffset <= partStartOffset)
        partEndOffset = juce::jmin(trimmedLength, partStartOffset + 1);

    return { rangeStart + partStartOffset, rangeStart + partEndOffset };
}

bool MegabreakExporter::prepareExportState(const juce::ValueTree& exportState, const int partIndex, const int partCount)
{
    auto settingsTree = exportState.getChildWithName(StateHandler::settingsId);
    auto dataTree = exportState.getChildWithName(StateHandler::dataId);
    if (! settingsTree.isValid() || ! dataTree.isValid() || partCount <= 0 || ! juce::isPositiveAndBelow(partIndex, partCount))
        return false;

    const auto sliceCount = dataTree.getNumChildren();
    if (sliceCount <= 0)
        return false;

    settingsTree.setProperty(StateHandler::normalizationId, "none", nullptr);
    settingsTree.setProperty(StateHandler::evenGridId, false, nullptr);
    settingsTree.setProperty(StateHandler::otFileId, true, nullptr);
    settingsTree.setProperty(StateHandler::chainMaxLengthId, sliceCount, nullptr);
    dataTree.setProperty(StateHandler::selectedSliceId, 0, nullptr);

    for (int sliceIndex = 0; sliceIndex < sliceCount; ++sliceIndex)
    {
        auto sliceTree = dataTree.getChild(sliceIndex);
        if (! sliceTree.isValid())
            continue;

        const auto partRange = getPartRange(sliceTree, partIndex, partCount);
        if (partRange.getEnd() <= partRange.getStart())
            continue;

        sliceTree.setProperty(StateHandler::sliceStartSampleId, partRange.getStart(), nullptr);
        sliceTree.setProperty(StateHandler::sliceEndSampleId, partRange.getEnd(), nullptr);
    }

    return true;
}

bool MegabreakExporter::exportToFiles(const juce::File& baseFile,
                                      const juce::ValueTree& baseState,
                                      const double targetSampleRate,
                                      const int bitDepth,
                                      const int partCount,
                                      juce::String* errorMessage)
{
    if (errorMessage != nullptr)
        errorMessage->clear();

    if (baseFile == juce::File{} || ! baseState.isValid() || targetSampleRate <= 0.0 || bitDepth <= 0 || partCount <= 0)
    {
        if (errorMessage != nullptr)
            *errorMessage = "Invalid megabreak export settings.";
        return false;
    }

    for (int partIndex = 0; partIndex < partCount; ++partIndex)
    {
        auto exportState = baseState.createCopy();
        if (! prepareExportState(exportState, partIndex, partCount))
        {
            if (errorMessage != nullptr)
                *errorMessage = "The megabreak export settings are invalid.";
            return false;
        }

        const auto exportFile = baseFile.getSiblingFile(baseFile.getFileNameWithoutExtension() + "_" + juce::String(partIndex + 1)
                                                       + baseFile.getFileExtension());

        if (! ChainExporter::exportToFile(exportFile, exportState, targetSampleRate, bitDepth, errorMessage))
        {
            if (errorMessage != nullptr && errorMessage->isNotEmpty())
                *errorMessage = "Failed to save " + exportFile.getFileName() + ": " + *errorMessage;
            return false;
        }
    }

    return true;
}
