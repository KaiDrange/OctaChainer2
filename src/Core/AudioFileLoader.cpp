#include "AudioFileLoader.h"

AudioFileLoader::AudioFileLoader()
{
    formatManager.registerBasicFormats();
}

std::unique_ptr<Slice> AudioFileLoader::loadFile(const juce::File& file, juce::String* errorMessage)
{
    return loadFileRegion(file, {}, errorMessage);
}

bool AudioFileLoader::loadFile(const juce::File& file, Slice& destination, juce::String* errorMessage)
{
    return loadFileRegion(file, {}, destination, errorMessage);
}

std::unique_ptr<Slice> AudioFileLoader::loadFileRegion(const juce::File& file, const juce::Range<juce::int64> sampleRange,
                                                       juce::String* errorMessage)
{
    auto slice = std::make_unique<Slice>();

    if (! loadFileRegion(file, sampleRange, *slice, errorMessage))
        return nullptr;

    return slice;
}

bool AudioFileLoader::loadFileRegion(const juce::File& file, const juce::Range<juce::int64> sampleRange,
                                     Slice& destination, juce::String* errorMessage)
{
    setError(errorMessage, {});

    if (! file.existsAsFile())
    {
        setError(errorMessage, "Audio file does not exist: " + file.getFullPathName());
        return false;
    }

    const std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr)
    {
        setError(errorMessage, "Unsupported audio file: " + file.getFileName());
        return false;
    }

    if (reader->numChannels <= 0)
    {
        setError(errorMessage, "Audio file has no readable channels: " + file.getFileName());
        return false;
    }

    if (reader->lengthInSamples <= 0)
    {
        setError(errorMessage, "Audio file contains no samples: " + file.getFileName());
        return false;
    }

    auto rangeStart = static_cast<juce::int64>(0);
    auto rangeEnd = reader->lengthInSamples;

    if (! sampleRange.isEmpty())
    {
        rangeStart = juce::jlimit<juce::int64>(0, reader->lengthInSamples, sampleRange.getStart());
        rangeEnd = juce::jlimit<juce::int64>(rangeStart, reader->lengthInSamples, sampleRange.getEnd());

        if (rangeEnd <= rangeStart)
        {
            setError(errorMessage, "Dropped audio region contains no readable samples: " + file.getFileName());
            return false;
        }
    }

    const auto numSamplesToRead = rangeEnd - rangeStart;

    if (numSamplesToRead > static_cast<juce::int64>(std::numeric_limits<int>::max()))
    {
        setError(errorMessage,
                 "Audio file is too big to load into memory: " + file.getFileName() + ".");
        return false;
    }

    const auto estimatedAudioDataBytes = static_cast<juce::int64>(reader->numChannels)
                                         * numSamplesToRead
                                         * static_cast<juce::int64>(sizeof(float));
    if (estimatedAudioDataBytes > maxLoadedAudioDataBytes)
    {
        setError(errorMessage,
                 "Audio file is too large to load safely into the slice list: " + file.getFileName()
                 + " (" + juce::String(estimatedAudioDataBytes / (1024 * 1024)) +
                 " MB estimated). Maximum supported size is "
                 + juce::String(maxLoadedAudioDataBytes / (1024 * 1024)) + "MB.");
        return false;
    }

    const auto numChannels = static_cast<int>(reader->numChannels);
    const auto numSamples = static_cast<int>(numSamplesToRead);

    auto* audioData = destination.getAudioData();
    audioData->setSize(numChannels, numSamples);
    audioData->clear();

    if (! reader->read(audioData, 0, numSamples, rangeStart, true, true))
    {
        audioData->setSize(0, 0);
        setError(errorMessage, "Failed to read audio data from: " + file.getFileName());
        return false;
    }

    destination.samplerate = reader->sampleRate;
    destination.bitDepth = reader->bitsPerSample;
    destination.channels = numChannels;
    destination.lengthInSamples = numSamplesToRead;
    destination.start = 0;
    destination.end = destination.lengthInSamples;
    destination.loopStart = 0;
    destination.name = file.getFileName();
    destination.sourcePath = file.getFullPathName();

    return true;
}

juce::String AudioFileLoader::getSupportedFilePatterns() const
{
    return formatManager.getWildcardForAllFormats();
}

void AudioFileLoader::setError(juce::String* errorMessage, const juce::String& message)
{
    if (errorMessage != nullptr)
        *errorMessage = message;
}
