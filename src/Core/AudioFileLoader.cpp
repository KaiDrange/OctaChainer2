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

std::vector<std::unique_ptr<Slice>> AudioFileLoader::loadFileSlices(const juce::File& file, juce::String* errorMessage)
{
    setError(errorMessage, {});

    if (!file.existsAsFile())
    {
        setError(errorMessage, "Audio file does not exist: " + file.getFullPathName());
        return {};
    }

    const std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr)
    {
        setError(errorMessage, "Unsupported audio file: " + file.getFileName());
        return {};
    }

    std::vector<juce::int64> cueOffsets;
    auto sliceRanges = std::vector<juce::Range<juce::int64>>{};
    const auto hasCuePoints = readWaveCueOffsets(file, cueOffsets);
    if (hasCuePoints)
        sliceRanges = buildSliceRangesFromCueOffsets(cueOffsets, reader->lengthInSamples);

    if (sliceRanges.empty())
        sliceRanges.emplace_back(0, reader->lengthInSamples);

    const auto totalSampleCount = std::accumulate(sliceRanges.begin(), sliceRanges.end(), static_cast<juce::int64>(0),
                                                  [](const juce::int64 sum, const auto& range)
                                                  {
                                                      return sum + (range.getEnd() - range.getStart());
                                                  });
    const auto estimatedAudioDataBytes = static_cast<juce::int64>(reader->numChannels)
                                         * totalSampleCount
                                         * static_cast<juce::int64>(sizeof(float));
    if (estimatedAudioDataBytes > maxLoadedAudioDataBytes)
    {
        setError(errorMessage,
                 "Audio file is too large to load safely into the slice list: " + file.getFileName()
                 + " (" + juce::String(estimatedAudioDataBytes / (1024 * 1024)) +
                 " MB estimated). Maximum supported size is "
                 + juce::String(maxLoadedAudioDataBytes / (1024 * 1024)) + "MB.");
        return {};
    }

    std::vector<std::unique_ptr<Slice>> slices;
    slices.reserve(sliceRanges.size());

    for (std::size_t i = 0; i < sliceRanges.size(); ++i)
    {
        const auto& range = sliceRanges[i];
        auto slice = std::make_unique<Slice>();
        if (!loadSliceFromReader(*reader, file, range, *slice, errorMessage))
            return {};

        if (hasCuePoints && sliceRanges.size() > 1)
            slice->name = file.getFileNameWithoutExtension() + " " + juce::String(static_cast<juce::int64>(i + 1));

        slices.push_back(std::move(slice));
    }

    return slices;
}

std::unique_ptr<Slice> AudioFileLoader::loadFileRegion(const juce::File& file, const juce::Range<juce::int64> sampleRange,
                                                       juce::String* errorMessage)
{
    auto slice = std::make_unique<Slice>();
    if (!loadFileRegion(file, sampleRange, *slice, errorMessage))
        return nullptr;
    return slice;
}

bool AudioFileLoader::loadFileRegion(const juce::File& file, const juce::Range<juce::int64>& sampleRange,
                                     Slice& destination, juce::String* errorMessage)
{
    setError(errorMessage, {});

    if (!file.existsAsFile())
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

    return loadSliceFromReader(*reader, file, sampleRange, destination, errorMessage);
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

std::uint32_t AudioFileLoader::readU32LE(const std::uint8_t* data)
{
    return static_cast<std::uint32_t>(data[0])
           | (static_cast<std::uint32_t>(data[1]) << 8)
           | (static_cast<std::uint32_t>(data[2]) << 16)
           | (static_cast<std::uint32_t>(data[3]) << 24);
}

bool AudioFileLoader::readBytes(juce::InputStream& stream, void* destination, const int bytesToRead)
{
    return stream.read(destination, bytesToRead) == bytesToRead;
}

bool AudioFileLoader::readWaveCueOffsets(const juce::File& file, std::vector<juce::int64>& cueOffsets)
{
        cueOffsets.clear();

        juce::FileInputStream stream(file);
        if (!stream.openedOk())
            return false;

        if (stream.getTotalLength() < 12)
            return false;

        std::array<std::uint8_t, 12> header{};
        if (! readBytes(stream, header.data(), header.size()))
            return false;

        if (! std::equal(riffChunkId.begin(), riffChunkId.end(), reinterpret_cast<const char*>(header.data()))
            || ! std::equal(waveFormatId.begin(), waveFormatId.end(), reinterpret_cast<const char*>(header.data() + 8)))
        {
            return false;
        }

        const auto totalLength = stream.getTotalLength();

        while (stream.getPosition() + 8 <= totalLength)
        {
            std::array<std::uint8_t, 8> chunkHeader{};
            if (! readBytes(stream, chunkHeader.data(), chunkHeader.size()))
                return false;

            const auto chunkSize = static_cast<juce::int64>(readU32LE(chunkHeader.data() + 4));
            const auto chunkDataStart = stream.getPosition();
            const auto chunkDataEnd = chunkDataStart + chunkSize;
            const auto chunkPadding = chunkSize & 1;

            if (chunkDataEnd > totalLength || chunkDataEnd + chunkPadding > totalLength)
                return false;

            if (std::equal(cueChunkId.begin(), cueChunkId.end(), reinterpret_cast<const char*>(chunkHeader.data())))
            {
                if (chunkSize < 4)
                    return false;

                std::array<std::uint8_t, 4> countBytes{};
                if (! readBytes(stream, countBytes.data(), countBytes.size()))
                    return false;

                const auto cueCount = readU32LE(countBytes.data());
                const auto cueDataSize = chunkSize - 4;
                if (cueCount > 0 && cueDataSize / 24 < cueCount)
                    return false;

                cueOffsets.reserve(cueOffsets.size() + cueCount);

                for (std::uint32_t i = 0; i < cueCount; ++i)
                {
                    std::array<std::uint8_t, 24> cuePoint{};
                    if (! readBytes(stream, cuePoint.data(), cuePoint.size()))
                        return false;

                    cueOffsets.push_back(readU32LE(cuePoint.data() + 20));
                }

                stream.setPosition(chunkDataEnd + chunkPadding);
                return true;
            }

            stream.setPosition(chunkDataEnd + chunkPadding);
        }

        return false;
}

std::vector<juce::Range<juce::int64>> AudioFileLoader::buildSliceRangesFromCueOffsets(
    const std::vector<juce::int64>& cueOffsets, const juce::int64 lengthInSamples)
{
    std::vector<juce::Range<juce::int64>> ranges;
    if (lengthInSamples <= 0)
        return ranges;

    std::vector<juce::int64> boundaries;
    boundaries.reserve(cueOffsets.size() + 2);
    boundaries.push_back(0);

    for (const auto offset : cueOffsets)
    {
        if (offset > 0 && offset < lengthInSamples)
            boundaries.push_back(offset);
    }

    boundaries.push_back(lengthInSamples);
    std::sort(boundaries.begin(), boundaries.end());
    boundaries.erase(std::unique(boundaries.begin(), boundaries.end()), boundaries.end());

    for (std::size_t i = 0; i + 1 < boundaries.size(); ++i)
    {
        const auto start = boundaries[i];
        const auto end = boundaries[i + 1];
        if (end > start)
            ranges.emplace_back(start, end);
    }

    return ranges;
}

bool AudioFileLoader::loadSliceFromReader(juce::AudioFormatReader& reader, const juce::File& file,
    const juce::Range<juce::int64>& sampleRange, Slice& destination, juce::String* errorMessage)
{
            if (reader.numChannels <= 0)
        {
            setError(errorMessage, "Audio file has no readable channels: " + file.getFileName());
            return false;
        }

        if (reader.lengthInSamples <= 0)
        {
            setError(errorMessage, "Audio file contains no samples: " + file.getFileName());
            return false;
        }

        auto rangeStart = static_cast<juce::int64>(0);
        auto rangeEnd = reader.lengthInSamples;

        if (! sampleRange.isEmpty())
        {
            rangeStart = juce::jlimit<juce::int64>(0, reader.lengthInSamples, sampleRange.getStart());
            rangeEnd = juce::jlimit<juce::int64>(rangeStart, reader.lengthInSamples, sampleRange.getEnd());

            if (rangeEnd <= rangeStart)
            {
                setError(errorMessage, "Dropped audio region contains no readable samples: " + file.getFileName());
                return false;
            }
        }

        const auto numSamplesToRead = rangeEnd - rangeStart;
        if (numSamplesToRead > static_cast<juce::int64>(std::numeric_limits<int>::max()))
        {
            setError(errorMessage, "Audio file is too big to load into memory: " + file.getFileName() + ".");
            return false;
        }

        const auto estimatedAudioDataBytes = static_cast<juce::int64>(reader.numChannels)
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

        const auto numChannels = static_cast<int>(reader.numChannels);
        const auto numSamples = static_cast<int>(numSamplesToRead);

        auto* audioData = destination.getAudioData();
        audioData->setSize(numChannels, numSamples);
        audioData->clear();

        if (! reader.read(audioData, 0, numSamples, rangeStart, true, true))
        {
            audioData->setSize(0, 0);
            setError(errorMessage, "Failed to read audio data from: " + file.getFileName());
            return false;
        }

        destination.samplerate = reader.sampleRate;
        destination.bitDepth = reader.bitsPerSample;
        destination.channels = numChannels;
        destination.lengthInSamples = numSamplesToRead;
        destination.start = 0;
        destination.end = destination.lengthInSamples;
        destination.loopStart = 0;
        destination.name = file.getFileName();
        destination.sourcePath = file.getFullPathName();

        return true;
}
