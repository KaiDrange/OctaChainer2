#include "OtReader.h"

OtReader::OtReader() = default;

std::uint32_t OtReader::checksumBytes(const std::vector<std::uint8_t>& bytes)
{
    std::uint32_t checksum = 0;
    for (std::size_t i = 16; i < OtFileFormat::otFileSize - 2; ++i)
        checksum += bytes[i];
    return checksum;
}

std::uint16_t OtReader::readU16BE(const std::uint8_t* data, const std::size_t offset)
{
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(data[offset]) << 8)
                                      | static_cast<std::uint16_t>(data[offset + 1]));
}

std::uint32_t OtReader::readU32BE(const std::uint8_t* data, const std::size_t offset)
{
    return (static_cast<std::uint32_t>(data[offset]) << 24)
           | (static_cast<std::uint32_t>(data[offset + 1]) << 16)
           | (static_cast<std::uint32_t>(data[offset + 2]) << 8)
           | static_cast<std::uint32_t>(data[offset + 3]);
}

bool OtReader::isSupportedBitDepth(const int bitDepth)
{
    return bitDepth == 16 || bitDepth == 24;
}

void OtReader::showErrorAsync(const juce::String& title, const juce::String& message, juce::Component* associatedComponent)
{
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, title, message, {}, associatedComponent);
}

bool OtReader::readOtFileBytes(const juce::File& otFile, std::vector<std::uint8_t>& bytes, juce::String* errorMessage)
{
    if (! otFile.existsAsFile())
    {
        if (errorMessage != nullptr)
            *errorMessage = "The selected OT file does not exist.";
        return false;
    }

    juce::FileInputStream stream(otFile);
    if (! stream.openedOk())
    {
        if (errorMessage != nullptr)
            *errorMessage = "The OT file could not be opened.";
        return false;
    }

    if (stream.getTotalLength() != static_cast<juce::int64>(OtFileFormat::otFileSize))
    {
        if (errorMessage != nullptr)
            *errorMessage = "The selected OT file has an unexpected size.";
        return false;
    }

    bytes.resize(OtFileFormat::otFileSize);
    if (stream.read(bytes.data(), static_cast<int>(bytes.size())) != static_cast<int>(bytes.size()))
    {
        if (errorMessage != nullptr)
            *errorMessage = "The OT file could not be read.";
        return false;
    }

    return true;
}

bool OtReader::parseOtBytes(const std::vector<std::uint8_t>& bytes, ParsedOtFile& parsed, juce::String* errorMessage)
{
    if (bytes.size() != OtFileFormat::otFileSize)
    {
        if (errorMessage != nullptr)
            *errorMessage = "The selected OT file has an unexpected size.";
        return false;
    }

    if (! std::equal(OtFileFormat::headerBytes.begin(), OtFileFormat::headerBytes.end(), bytes.begin()))
    {
        if (errorMessage != nullptr)
            *errorMessage = "The selected file is not a valid OT file.";
        return false;
    }

    const auto storedChecksum = readU16BE(bytes.data(), OtFileFormat::otFileSize - 2);
    if (static_cast<std::uint16_t>(checksumBytes(bytes) & 0xFFFFu) != storedChecksum)
    {
        if (errorMessage != nullptr)
            *errorMessage = "The selected OT file failed checksum validation.";
        return false;
    }

    parsed.tempo = juce::roundToInt(static_cast<double>(readU32BE(bytes.data(), 23)) / 24.0);
    parsed.stretchSetting = static_cast<OtFileFormat::Stretch_t>(readU32BE(bytes.data(), 35));
    parsed.loopSetting = static_cast<OtFileFormat::Loop_t>(readU32BE(bytes.data(), 39));
    parsed.gain = juce::roundToInt(static_cast<double>(readU16BE(bytes.data(), 43)) - 48.0);
    parsed.trigQuantSetting = static_cast<OtFileFormat::TrigQuant_t>(bytes[45]);
    parsed.trimStart = readU32BE(bytes.data(), 46);
    parsed.trimEnd = readU32BE(bytes.data(), 50);

    const auto sliceCount = readU32BE(bytes.data(), 826);
    if (sliceCount > OtFileFormat::maxSliceCount)
    {
        if (errorMessage != nullptr)
            *errorMessage = "The selected OT file contains too many slices.";
        return false;
    }

    parsed.slices.clear();
    parsed.slices.reserve(sliceCount > 0 ? sliceCount : 1);

    for (std::uint32_t i = 0; i < sliceCount; ++i)
    {
        const auto offset = 58 + static_cast<std::size_t>(i) * 12;
        parsed.slices.push_back({
            readU32BE(bytes.data(), offset),
            readU32BE(bytes.data(), offset + 4),
            readU32BE(bytes.data(), offset + 8)
        });
    }

    if (parsed.slices.empty())
        parsed.slices.push_back({ parsed.trimStart, parsed.trimEnd, 0xFFFFFFFFu });

    return true;
}

bool OtReader::readAudioFileMetadata(const juce::File& file, int& sampleRate, int& bitDepth, int& numChannels,
                                     juce::int64& lengthInSamples, juce::String* errorMessage)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    const std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr)
    {
        if (errorMessage != nullptr)
            *errorMessage = "Unsupported audio file: " + file.getFileName();
        return false;
    }

    sampleRate = static_cast<int>(reader->sampleRate);
    bitDepth = static_cast<int>(reader->bitsPerSample);
    numChannels = static_cast<int>(reader->numChannels);
    lengthInSamples = reader->lengthInSamples;
    return true;
}

bool OtReader::readImportSettingsImpl(const juce::File& otFile, ImportSettings& settings, juce::String* errorMessage) const
{
    std::vector<std::uint8_t> bytes;
    ParsedOtFile parsed;

    if (! readOtFileBytes(otFile, bytes, errorMessage))
        return false;

    if (! parseOtBytes(bytes, parsed, errorMessage))
        return false;

    const auto wavFile = otFile.withFileExtension(".wav");
    if (! wavFile.existsAsFile())
    {
        if (errorMessage != nullptr)
            *errorMessage = "The corresponding audio file was not found: " + wavFile.getFileName();
        return false;
    }

    int sampleRate = 0;
    int bitDepth = 0;
    int numChannels = 0;
    juce::int64 lengthInSamples = 0;
    if (! readAudioFileMetadata(wavFile, sampleRate, bitDepth, numChannels, lengthInSamples, errorMessage))
        return false;

    const auto estimatedAudioBytes = static_cast<juce::int64>(juce::jmax(0, numChannels))
                                     * juce::jmax<juce::int64>(0, lengthInSamples)
                                     * static_cast<juce::int64>(sizeof(float));
    const auto importAllowanceBytes = static_cast<juce::int64>(juce::jmax<std::size_t>(1, parsed.slices.size()))
                                      * AudioFileLoader::maxLoadedAudioDataBytes;
    if (estimatedAudioBytes > importAllowanceBytes)
    {
        if (errorMessage != nullptr)
            *errorMessage = "The corresponding audio file is too large for the imported slice count.";
        return false;
    }

    if (! isSupportedBitDepth(bitDepth))
    {
        if (errorMessage != nullptr)
            *errorMessage = "The corresponding audio file uses an unsupported bit depth.";
        return false;
    }

    settings.otFile = otFile;
    settings.audioFile = wavFile;
    settings.sampleRate = sampleRate;
    settings.bitDepth = bitDepth;
    settings.audioChannels = numChannels;
    settings.audioLengthInSamples = lengthInSamples;
    settings.tempo = parsed.tempo;
    settings.gain = parsed.gain;
    settings.loopSetting = parsed.loopSetting;
    settings.stretchSetting = parsed.stretchSetting;
    settings.trigQuantSetting = parsed.trigQuantSetting;
    settings.sliceRanges.clear();

    for (const auto& slice : parsed.slices)
    {
        const auto startSample = static_cast<juce::int64>(slice.startPoint);
        const auto endSample = static_cast<juce::int64>(slice.endPoint);
        if (endSample <= startSample || endSample > lengthInSamples)
        {
            if (errorMessage != nullptr)
                *errorMessage = "The OT file contains an invalid slice range.";
            return false;
        }

        settings.sliceRanges.push_back(juce::Range(startSample, endSample));
    }

    return true;
}

bool OtReader::performImportImpl(const ImportSettings& settings, StateHandler& stateHandler, juce::String* errorMessage)
{
    std::vector<std::unique_ptr<Slice>> importedSlices;
    importedSlices.reserve(settings.sliceRanges.size());

    for (const auto& range : settings.sliceRanges)
    {
        auto loadedSlice = audioFileLoader.loadFileRegion(settings.audioFile, range, errorMessage);
        if (loadedSlice == nullptr)
            return false;

        importedSlices.push_back(std::move(loadedSlice));
    }

    stateHandler.removeAllSlices();
    stateHandler.setStateValue(StateHandler::samplerateId, settings.sampleRate);
    stateHandler.setStateValue(StateHandler::bitDepthId, settings.bitDepth);
    stateHandler.setStateValue(StateHandler::timestretchId,
                               settings.stretchSetting == OtFileFormat::Normal ? "normal"
                               : settings.stretchSetting == OtFileFormat::Beat ? "beat"
                               : "off");
    stateHandler.setStateValue(StateHandler::loopModeId,
                               settings.loopSetting == OtFileFormat::Loop ? "on"
                               : settings.loopSetting == OtFileFormat::PIPO ? "pingpong"
                               : "off");
    stateHandler.setStateValue(StateHandler::triqQuantId,
                               settings.trigQuantSetting == OtFileFormat::Pattern ? juce::var("pattern")
                               : settings.trigQuantSetting == OtFileFormat::Direct ? juce::var("direct")
                               : juce::var(static_cast<int>(settings.trigQuantSetting)));
    stateHandler.setStateValue(StateHandler::gainId, settings.gain);
    stateHandler.setStateValue(StateHandler::bpmId, settings.tempo);

    for (const auto& importedSlice : importedSlices)
        stateHandler.addSlice(*importedSlice, nullptr, false);

    if (! importedSlices.empty())
        stateHandler.selectSlice(static_cast<int>(importedSlices.size()) - 1);

    return true;
}

bool OtReader::loadImportSettings(const juce::File& otFile, ImportSettings& settings, juce::Component* associatedComponent)
{
    juce::String errorMessage;
    if (! readImportSettingsImpl(otFile, settings, &errorMessage))
    {
        showErrorAsync("Could not import OT file",
                       errorMessage.isNotEmpty() ? errorMessage : "The OT file could not be imported.",
                       associatedComponent);
        return false;
    }

    return true;
}

bool OtReader::performImport(const ImportSettings& settings, StateHandler& stateHandler, juce::Component* associatedComponent)
{
    juce::String errorMessage;
    if (! performImportImpl(settings, stateHandler, &errorMessage))
    {
        showErrorAsync("Could not import OT file",
                       errorMessage.isNotEmpty() ? errorMessage : "The OT file could not be imported.",
                       associatedComponent);
        return false;
    }

    return true;
}

void OtReader::importOtFile(const juce::File& otFile, StateHandler& stateHandler, juce::Component* associatedComponent)
{
    ImportSettings settings;
    if (! loadImportSettings(otFile, settings, associatedComponent))
        return;

    const auto self = shared_from_this();
    const juce::Component::SafePointer<juce::Component> safeComponent(associatedComponent);
    auto settingsPtr = std::make_shared<ImportSettings>(std::move(settings));
    auto* stateHandlerPtr = &stateHandler;

    if (stateHandler.getNumSlices() > 0)
    {
        juce::AlertWindow::showOkCancelBox(juce::AlertWindow::WarningIcon,
                                           "Import OT file",
                                           "The existing slices will be cleared. Are you sure?",
                                           "Import",
                                           "Cancel",
                                           associatedComponent,
                                           juce::ModalCallbackFunction::create([self, settingsPtr, stateHandlerPtr, safeComponent](int result) mutable
                                           {
                                               if (result != 1 || safeComponent == nullptr || stateHandlerPtr == nullptr)
                                                   return;

                                               self->performImport(*settingsPtr, *stateHandlerPtr, safeComponent.getComponent());
                                           }));
        return;
    }

    if (safeComponent != nullptr && stateHandlerPtr != nullptr)
        self->performImport(*settingsPtr, *stateHandlerPtr, safeComponent.getComponent());
}
