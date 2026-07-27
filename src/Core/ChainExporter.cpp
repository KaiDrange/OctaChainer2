#include "ChainExporter.h"

#include "AudioUtil.h"
#include "OtWriter.h"

OtFileFormat::Stretch_t ChainExporter::getOtStretchSetting(const juce::ValueTree& settingsTree)
{
    const auto stretchValue = settingsTree.getProperty(StateHandler::timestretchId, "off").toString();

    if (stretchValue == "normal")
        return OtFileFormat::Normal;

    if (stretchValue == "beat")
        return OtFileFormat::Beat;

    return OtFileFormat::NoStretch;
}

OtFileFormat::Loop_t ChainExporter::getOtLoopSetting(const juce::ValueTree& settingsTree)
{
    const auto loopValue = settingsTree.getProperty(StateHandler::loopModeId, "off").toString();

    if (loopValue == "on")
        return OtFileFormat::Loop;

    if (loopValue == "pingpong")
        return OtFileFormat::PIPO;

    return OtFileFormat::NoLoop;
}

OtFileFormat::TrigQuant_t ChainExporter::getOtTrigQuantSetting(const juce::ValueTree& settingsTree)
{
    const auto quantizeValue = settingsTree.getProperty(StateHandler::triqQuantId, "direct");

    if (quantizeValue.isString())
    {
        if (quantizeValue.toString() == "pattern")
            return OtFileFormat::Pattern;

        return OtFileFormat::Direct;
    }

    switch (static_cast<int>(quantizeValue))
    {
    case 1: return OtFileFormat::S_1;
    case 2: return OtFileFormat::S_2;
    case 3: return OtFileFormat::S_3;
    case 4: return OtFileFormat::S_4;
    case 6: return OtFileFormat::S_6;
    case 8: return OtFileFormat::S_8;
    case 12: return OtFileFormat::S_12;
    case 16: return OtFileFormat::S_16;
    case 24: return OtFileFormat::S_24;
    case 32: return OtFileFormat::S_32;
    case 48: return OtFileFormat::S_48;
    case 64: return OtFileFormat::S_64;
    case 96: return OtFileFormat::S_96;
    case 128: return OtFileFormat::S_128;
    case 192: return OtFileFormat::S_192;
    case 256: return OtFileFormat::S_256;
    default: break;
    }

    jassertfalse;
    return OtFileFormat::Direct;
}

bool ChainExporter::shouldWriteOtFile(const juce::ValueTree& exportState)
{
    const auto settingsTree = exportState.getChildWithName(StateHandler::settingsId);
    if (! settingsTree.isValid())
        return false;

    return settingsTree.getProperty(StateHandler::otFileId, StateHandler::otFileDefault);
}

bool ChainExporter::writeOtFile(const juce::File& wavFile,
                                const juce::ValueTree& exportState,
                                const Chain& exportChain,
                                juce::String* errorMessage)
{
    const auto settingsTree = exportState.getChildWithName(StateHandler::settingsId);
    if (! settingsTree.isValid())
    {
        if (errorMessage != nullptr)
            *errorMessage = "The export settings are invalid.";
        return false;
    }

    const auto totalSamples = exportChain.getAudioData().getNumSamples();
    const auto sampleRate = juce::roundToInt(exportChain.getSampleRate());
    if (totalSamples <= 0 || sampleRate <= 0)
    {
        if (errorMessage != nullptr)
            *errorMessage = "The rendered chain is invalid.";
        return false;
    }

    const auto gain = static_cast<double>(settingsTree.getProperty(
        StateHandler::gainId,
        StateHandler::gainValue.defaultValue));
    const auto bpm = static_cast<double>(settingsTree.getProperty(
        StateHandler::bpmId,
        StateHandler::bpmValue.defaultValue));
    auto writer = OtWriter(wavFile.withFileExtension(".ot"),
                           sampleRate,
                           getOtLoopSetting(settingsTree),
                           getOtStretchSetting(settingsTree),
                           getOtTrigQuantSetting(settingsTree),
                           gain,
                           bpm);

    for (const auto& segment : exportChain.getSegments())
        writer.addSlice(static_cast<std::uint32_t>(segment.startSample),
                        static_cast<std::uint32_t>(segment.startSample + segment.sampleCount));

    if (! writer.write(static_cast<std::uint32_t>(totalSamples)))
    {
        if (errorMessage != nullptr)
            *errorMessage = "The OT file could not be written.";
        return false;
    }

    return true;
}

bool ChainExporter::exportToFile(const juce::File& wavFile,
                                 const juce::ValueTree& exportState,
                                 const double targetSampleRate,
                                 const int bitDepth,
                                 juce::String* errorMessage)
{
    if (wavFile == juce::File{} || targetSampleRate <= 0.0)
    {
        if (errorMessage != nullptr)
            *errorMessage = "Invalid output file or sample rate.";
        return false;
    }

    Chain exportChain;
    juce::String renderError;
    const auto completed = exportChain.create(exportState, targetSampleRate, [] { return false; }, &renderError);

    if (! completed || ! exportChain.isValid())
    {
        if (errorMessage != nullptr)
            *errorMessage = renderError.isNotEmpty() ? renderError : "The chain could not be rendered.";
        return false;
    }

    const auto settingsTree = exportState.getChildWithName(StateHandler::settingsId);
    const auto embedMarkers = settingsTree.isValid()
                              && static_cast<bool>(settingsTree.getProperty(StateHandler::embedMarkersId,
                                                                            StateHandler::embedMarkersDefault));
    const auto cueOffsets = embedMarkers ? AudioUtil::buildCueOffsetsFromSegments(exportChain.getSegments())
                                         : std::vector<juce::int64>{};

    juce::String writeError;
    if (! AudioUtil::writeWavFile(wavFile,
                                  exportChain.getAudioData(),
                                  targetSampleRate,
                                  bitDepth,
                                  cueOffsets.empty() ? nullptr : &cueOffsets,
                                  &writeError))
    {
        if (errorMessage != nullptr)
            *errorMessage = writeError.isNotEmpty() ? writeError : "The WAV file could not be written.";
        return false;
    }

    if (shouldWriteOtFile(exportState) && ! writeOtFile(wavFile, exportState, exportChain, errorMessage))
        return false;

    return true;
}
