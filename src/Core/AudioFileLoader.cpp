#include "AudioFileLoader.h"

AudioFileLoader::AudioFileLoader()
{
    formatManager.registerBasicFormats();
}

std::unique_ptr<Slice> AudioFileLoader::loadFile(const juce::File& file, juce::String* errorMessage)
{
    auto slice = std::make_unique<Slice>();

    if (! loadFile(file, *slice, errorMessage))
        return nullptr;

    return slice;
}

bool AudioFileLoader::loadFile(const juce::File& file, Slice& destination, juce::String* errorMessage)
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

    if (reader->lengthInSamples > static_cast<juce::int64>(std::numeric_limits<int>::max()))
    {
        setError(errorMessage, "Audio file is too long to load into memory: " + file.getFileName());
        return false;
    }

    const auto numChannels = static_cast<int>(reader->numChannels);
    const auto numSamples = static_cast<int>(reader->lengthInSamples);

    auto* audioData = destination.getAudioData();
    audioData->setSize(numChannels, numSamples);
    audioData->clear();

    if (! reader->read(audioData, 0, numSamples, 0, true, true))
    {
        audioData->setSize(0, 0);
        setError(errorMessage, "Failed to read audio data from: " + file.getFileName());
        return false;
    }

    destination.samplerate = reader->sampleRate;
    destination.bitDepth = reader->bitsPerSample;
    destination.channels = numChannels;
    destination.lengthInSamples = reader->lengthInSamples;
    destination.start = 0;
    destination.end = destination.lengthInSamples / destination.channels;
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
