#include "HeadlessBatchRunner.h"
#include "ChainExporter.h"

juce::String HeadlessBatchRunner::buildHelpText()
{
    return
        "OctaChainer 2 headless batch export\n"
        "\n"
        "Usage:\n"
        "  OctaChainer2.exe --help\n"
        "  OctaChainer2.exe --headless [options] --sourcefolder <folder>\n"
        "                       --targetfolder <folder> --chainbasename <name>\n"
        "                       --samplerate <rate> --bitdepth <depth>\n"
        "                       --channels <count>\n"
        "\n"
        "Options:\n"
        "  --headless            Run batch export without showing the UI\n"
        "  --help                Show this help text and exit\n"
        "  --sourcefolder        Source folder to scan recursively for audio files\n"
        "  --targetfolder        Output folder for rendered chain files\n"
        "  --chainbasename       Base filename for rendered chain files\n"
        "  --samplerate          Output sample rate (valid: 44.1k/44100, 48k/48000, 96k/96000)\n"
        "  --bitdepth            Output bit depth (valid: 16, 24)\n"
        "  --channels            Output channel count (valid: 1, 2)\n"
        "  --msslicefadein       Fade-in per slice in milliseconds (valid: 0, 1, 2, 3, 5, 10)\n"
        "  --msslicefadeout      Fade-out per slice in milliseconds (valid: 0, 1, 2, 3, 5, 10)\n"
        "  --chainslicecount     Number of slices per chain file (range: 1-120)\n"
        "  --evengrid            Pad slices in each chain to make an even grid\n"
        "  --embedmarkers        Write WAV cue markers at slice boundaries\n"
        "  --otfile              Write a matching .ot file alongside each WAV\n"
        "  --normalizeslice      Normalize each slice before chaining\n"
        "  --normalizechain      Normalize the rendered chain\n"
        "\n"
        "Notes:\n"
        "  Options accept either --name value or --name=value.\n"
        "  --normalizeslice and --normalizechain are mutually exclusive.\n"
        "  --help can be used on its own.\n"
        "\n"
        "Example:\n"
        "  OctaChainer2.exe --headless --sourcefolder \"C:\\InputWavs\" --targetfolder \"C:\\OutputChains\"\n"
        "                   --chainbasename MyChain --samplerate 48000 --bitdepth 24\n"
        "                   --channels 2 --chainslicecount 64 --normalizeslice\n";
}

bool HeadlessBatchRunner::isSupportedFadeMs(const int fadeMs)
{
    switch (fadeMs)
    {
    case 1:
    case 2:
    case 3:
    case 5:
    case 10:
        return true;
    default:
        return false;
    }
}

bool parseSampleRate(const juce::String& text, double& sampleRate)
{
    const auto normalized = text.trim().toLowerCase();

    if (normalized == "44.1k" || normalized == "44100")
    {
        sampleRate = 44100.0;
        return true;
    }

    if (normalized == "48k" || normalized == "48000")
    {
        sampleRate = 48000.0;
        return true;
    }

    if (normalized == "96k" || normalized == "96000")
    {
        sampleRate = 96000.0;
        return true;
    }

    return false;
}

bool isSupportedBitDepth(const int bitDepth)
{
    return bitDepth == 16 || bitDepth == 24;
}

bool isSupportedChannelCount(const int channels)
{
    return channels == 1 || channels == 2;
}

std::vector<juce::String> HeadlessBatchRunner::tokenizeCommandLine(const juce::String& commandLine)
{
    std::vector<juce::String> tokens;
    juce::String currentToken;
    bool insideQuotes = false;

    for (int i = 0; i < commandLine.length(); ++i)
    {
        const auto character = commandLine[i];

        if (character == '"')
        {
            insideQuotes = !insideQuotes;
            continue;
        }

        if (juce::CharacterFunctions::isWhitespace(character) && !insideQuotes)
        {
            if (currentToken.isNotEmpty())
            {
                tokens.push_back(currentToken);
                currentToken.clear();
            }

            continue;
        }

        currentToken += character;
    }

    if (currentToken.isNotEmpty())
        tokens.push_back(currentToken);

    return tokens;
}

bool HeadlessBatchRunner::tryParseInt(const juce::String& text, int& value)
{
    if (text.isEmpty())
        return false;

    for (int i = 0; i < text.length(); ++i)
    {
        const auto character = text[i];
        if (i == 0 && (character == '-' || character == '+'))
            continue;

        if (!juce::CharacterFunctions::isDigit(character))
            return false;
    }

    value = text.getIntValue();
    return true;
}

bool HeadlessBatchRunner::parseOptions(const juce::String& commandLine, HeadlessOptions& options,
    juce::String& errorMessage)
{
    const auto tokens = tokenizeCommandLine(commandLine);

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        auto token = tokens[i].trim();
        if (token.isEmpty())
            continue;

        juce::String value;
        const auto equalsIndex = token.indexOfChar('=');
        if (equalsIndex >= 0)
        {
            value = token.substring(equalsIndex + 1).trim();
            token = token.substring(0, equalsIndex).trim();
        }

        const auto requireValue = [&](juce::String& destination) -> bool
        {
            if (value.isEmpty())
            {
                if (i + 1 >= tokens.size())
                {
                    errorMessage = "Missing value for " + token;
                    return false;
                }

                value = tokens[++i].trim();
            }

            if (value.isEmpty())
            {
                errorMessage = "Missing value for " + token;
                return false;
            }

            destination = value;
            return true;
        };

        const auto requireIntValue = [&](int& destination) -> bool
        {
            juce::String parsedValue;
            if (!requireValue(parsedValue))
                return false;

            if (!tryParseInt(parsedValue, destination))
            {
                errorMessage = "Invalid integer value for " + token + ": " + parsedValue;
                return false;
            }

            return true;
        };

        if (token == "--headless")
            options.headless = true;
        else if (token == "--help")
            options.help = true;
        else if (token == "--sourcefolder")
        {
            if (!requireValue(options.sourceFolder))
                return false;
        }
        else if (token == "--targetfolder")
        {
            if (!requireValue(options.targetFolder))
                return false;
        }
        else if (token == "--chainbasename")
        {
            if (!requireValue(options.targetBaseName))
                return false;
        }
        else if (token == "--samplerate")
        {
            juce::String parsedValue;
            if (!requireValue(parsedValue))
                return false;

            if (!parseSampleRate(parsedValue, options.targetSampleRate))
            {
                errorMessage = "Invalid value for --samplerate: " + parsedValue;
                return false;
            }
        }
        else if (token == "--bitdepth")
        {
            if (!requireIntValue(options.targetBitDepth))
                return false;
        }
        else if (token == "--channels")
        {
            if (!requireIntValue(options.channels))
                return false;
        }
        else if (token == "--msslicefadein")
        {
            if (!requireIntValue(options.msSliceFadeIn))
                return false;
        }
        else if (token == "--msslicefadeout")
        {
            if (!requireIntValue(options.msSliceFadeOut))
                return false;
        }
        else if (token == "--chainslicecount")
        {
            if (!requireIntValue(options.chainSliceCount))
                return false;
        }
        else if (token == "--evengrid")
            options.evenGrid = true;
        else if (token == "--embedmarkers")
            options.embedMarkers = true;
        else if (token == "--otfile")
            options.otFile = true;
        else if (token == "--normalizeslice")
            options.normalizeSlice = true;
        else if (token == "--normalizechain")
            options.normalizeChain = true;
        else
        {
            errorMessage = "Unknown option: " + token;
            return false;
        }
    }

    return true;
}

bool HeadlessBatchRunner::validateOptions(const HeadlessOptions& options, juce::String& errorMessage)
{
    if (!options.help && !options.headless)
        return true;

    if (options.normalizeSlice && options.normalizeChain)
    {
        errorMessage = "Use either --normalizeslice or --normalizechain, not both.";
        return false;
    }

    if (!options.help)
    {
        if (options.sourceFolder.isEmpty())
        {
            errorMessage = "Missing required option: --sourcefolder";
            return false;
        }

        if (options.targetFolder.isEmpty())
        {
            errorMessage = "Missing required option: --targetfolder";
            return false;
        }

        if (options.targetBaseName.isEmpty())
        {
            errorMessage = "Missing required option: --chainbasename";
            return false;
        }

        if (options.targetSampleRate <= 0.0)
        {
            errorMessage = "Missing required option: --samplerate";
            return false;
        }

        if (options.targetBitDepth <= 0)
        {
            errorMessage = "Missing required option: --bitdepth";
            return false;
        }

        if (options.channels <= 0)
        {
            errorMessage = "Missing required option: --channels";
            return false;
        }
    }

    if (options.targetSampleRate > 0.0
        && options.targetSampleRate != 44100.0
        && options.targetSampleRate != 48000.0
        && options.targetSampleRate != 96000.0)
    {
        errorMessage = "--samplerate must be one of: 44.1k, 48k, 96k.";
        return false;
    }

    if (options.targetBitDepth > 0 && ! isSupportedBitDepth(options.targetBitDepth))
    {
        errorMessage = "--bitdepth must be 16 or 24.";
        return false;
    }

    if (options.channels > 0 && ! isSupportedChannelCount(options.channels))
    {
        errorMessage = "--channels must be 1 or 2.";
        return false;
    }

    if (options.msSliceFadeIn < -1)
    {
        errorMessage = "--msslicefadein must be one of: 1, 2, 3, 5, 10.";
        return false;
    }

    if (options.msSliceFadeIn > 0 && !isSupportedFadeMs(options.msSliceFadeIn))
    {
        errorMessage = "--msslicefadein must be one of: 1, 2, 3, 5, 10.";
        return false;
    }

    if (options.msSliceFadeOut < -1)
    {
        errorMessage = "--msslicefadeout must be one of: 1, 2, 3, 5, 10.";
        return false;
    }

    if (options.msSliceFadeOut > 0 && !isSupportedFadeMs(options.msSliceFadeOut))
    {
        errorMessage = "--msslicefadeout must be one of: 1, 2, 3, 5, 10.";
        return false;
    }

    if (options.chainSliceCount != -1 && (options.chainSliceCount < 1 || options.chainSliceCount > 120))
    {
        errorMessage = "--chainslicecount must be between 1 and 120.";
        return false;
    }

    return true;
}

std::vector<juce::File> HeadlessBatchRunner::collectAudioFiles(const juce::File& sourceFolder,
    const AudioFileLoader& loader)
{
    std::vector<juce::File> files;

    for (const auto& entry : juce::RangedDirectoryIterator(sourceFolder,
                                                           true,
                                                           loader.getSupportedFilePatterns(),
                                                           juce::File::findFiles))
    {
        files.push_back(entry.getFile());
    }

    std::sort(files.begin(), files.end(),
              [](const juce::File& lhs, const juce::File& rhs)
              {
                  return lhs.getFullPathName().compareIgnoreCase(rhs.getFullPathName()) < 0;
              });

    return files;
}

bool HeadlessBatchRunner::loadFolderIntoState(const juce::File& sourceFolder, StateHandler& stateHandler,
    juce::String& errorMessage)
{
    if (!sourceFolder.exists())
    {
        errorMessage = "Source folder does not exist: " + sourceFolder.getFullPathName();
        return false;
    }

    if (!sourceFolder.isDirectory())
    {
        errorMessage = "Source path is not a folder: " + sourceFolder.getFullPathName();
        return false;
    }

    AudioFileLoader loader;
    const auto files = collectAudioFiles(sourceFolder, loader);
    if (files.empty())
    {
        errorMessage = "No supported audio files were found in: " + sourceFolder.getFullPathName();
        return false;
    }

    auto remainingSlots = maxSliceCount - stateHandler.getNumSlices();
    int lastLoadedRow = -1;

    for (const auto& file : files)
    {
        juce::String loadError;
        const auto slices = loader.loadFileSlices(file, &loadError);
        if (slices.empty())
        {
            errorMessage = loadError.isNotEmpty()
                ? loadError
                : "Failed to load audio file: " + file.getFileName();
            return false;
        }

        for (const auto& slice : slices)
        {
            if (remainingSlots <= 0)
            {
                errorMessage = "The source folder exceeds the maximum of 1000 slices supported per batch run.";
                return false;
            }

            lastLoadedRow = stateHandler.addSlice(*slice, nullptr, false);
            --remainingSlots;
        }
    }

    if (lastLoadedRow < 0)
    {
        errorMessage = "No slices could be loaded from the source folder.";
        return false;
    }

    stateHandler.selectSlice(0);
    return true;
}

void HeadlessBatchRunner::applyHeadlessSettings(const HeadlessOptions& options, StateHandler& stateHandler)
{
    if (options.targetSampleRate > 0.0)
        stateHandler.setStateValue(StateHandler::samplerateId, static_cast<int>(options.targetSampleRate));

    if (options.targetBitDepth > 0)
        stateHandler.setStateValue(StateHandler::bitDepthId, options.targetBitDepth);

    if (options.channels == 1)
        stateHandler.setStateValue(StateHandler::channelsId, juce::var("mono"));
    else if (options.channels == 2)
        stateHandler.setStateValue(StateHandler::channelsId, juce::var("stereo"));

    stateHandler.setStateValue(StateHandler::normalizationId,
                               options.normalizeSlice ? juce::var("slices")
                               : options.normalizeChain ? juce::var("chain")
                               : juce::var("none"));
    stateHandler.setStateValue(StateHandler::evenGridId, options.evenGrid);
    stateHandler.setStateValue(StateHandler::embedMarkersId, options.embedMarkers);
    stateHandler.setStateValue(StateHandler::otFileId, options.otFile);

    if (options.msSliceFadeIn >= 0)
        stateHandler.setStateValue(StateHandler::fadeinId, options.msSliceFadeIn);

    if (options.msSliceFadeOut >= 0)
        stateHandler.setStateValue(StateHandler::fadeoutId, options.msSliceFadeOut);

    if (options.chainSliceCount > 0)
        stateHandler.setStateValue(StateHandler::chainMaxLengthId, options.chainSliceCount);
}

bool HeadlessBatchRunner::exportAllChains(const juce::File& targetFile, const StateHandler& stateHandler,
    juce::String& errorMessage)
{
    auto baseState = stateHandler.getState().createCopy();
    const auto settingsTree = baseState.getChildWithName(StateHandler::settingsId);
    const auto dataTree = baseState.getChildWithName(StateHandler::dataId);
    if (!settingsTree.isValid() || !dataTree.isValid())
    {
        errorMessage = "The batch export state is invalid.";
        return false;
    }

    const auto numSlices = dataTree.getNumChildren();
    const auto maxSlicesPerChain = juce::jmax(1, static_cast<int>(settingsTree.getProperty(
        StateHandler::chainMaxLengthId,
        static_cast<int>(StateHandler::chainMaxLengthValue.defaultValue))));
    const auto chainCount = juce::jmax(1, (numSlices + maxSlicesPerChain - 1) / maxSlicesPerChain);
    const auto targetSampleRate = static_cast<double>(stateHandler.getStateValue<int>(StateHandler::samplerateId, 44100));
    const auto bitDepth = stateHandler.getStateValue<int>(StateHandler::bitDepthId, 16);

    for (int chainIndex = 0; chainIndex < chainCount; ++chainIndex)
    {
        auto exportState = baseState.createCopy();
        auto exportDataTree = exportState.getChildWithName(StateHandler::dataId);
        if (exportDataTree.isValid())
            exportDataTree.setProperty(StateHandler::selectedSliceId, chainIndex * maxSlicesPerChain, nullptr);

        const auto exportFile = chainCount > 1
            ? targetFile.getSiblingFile(targetFile.getFileNameWithoutExtension()
                                        + "_" + juce::String(chainIndex + 1)
                                        + targetFile.getFileExtension())
            : targetFile;

        if (! ChainExporter::exportToFile(exportFile, exportState, targetSampleRate, bitDepth, &errorMessage))
        {
            if (chainCount > 1 && errorMessage.isNotEmpty())
                errorMessage = "Failed to save " + exportFile.getFileName() + ": " + errorMessage;
            return false;
        }
    }

    return true;
}

HeadlessBatchRunner::Result HeadlessBatchRunner::runFromCommandLine(const juce::String& commandLine)
{
    HeadlessOptions options;
    juce::String parseError;
    if (!parseOptions(commandLine, options, parseError))
    {
        return { true, false, 1, parseError + "\n\n" + buildHelpText() };
    }

    if (! options.help && ! options.headless)
        return {};

    juce::String validationError;
    if (!validateOptions(options, validationError))
    {
        return { true, false, 1, validationError + "\n\n" + buildHelpText() };
    }

    if (options.help)
        return { true, true, 0, buildHelpText() };

    StateHandler stateHandler;
    applyHeadlessSettings(options, stateHandler);

    juce::String errorMessage;
    if (!loadFolderIntoState(juce::File(options.sourceFolder), stateHandler, errorMessage))
        return { true, false, 1, errorMessage };

    const auto targetFolder = juce::File(options.targetFolder);
    if ((!targetFolder.exists() && !targetFolder.createDirectory()) || !targetFolder.isDirectory())
    {
        return { true, false, 1, "Could not create target folder: " + targetFolder.getFullPathName() };
    }

    const auto targetFile = targetFolder.getChildFile(options.targetBaseName).withFileExtension(".wav");
    if (!exportAllChains(targetFile, stateHandler, errorMessage))
        return { true, false, 1, errorMessage };

    const auto chainSliceCount = stateHandler.getStateValue<int>(StateHandler::chainMaxLengthId,
                                                                 static_cast<int>(StateHandler::chainMaxLengthValue.defaultValue));
    const auto chainCount = juce::jmax(1, (stateHandler.getNumSlices() + chainSliceCount - 1) / chainSliceCount);
    return { true, true, 0,
             "Exported "
             + juce::String(stateHandler.getNumSlices())
             + " slices into "
             + juce::String(chainCount)
             + " chain file(s) at "
             + targetFolder.getFullPathName() };
}
