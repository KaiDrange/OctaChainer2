#include "AudioUtil.h"

void AudioUtil::normalizeAudioBuffer(juce::AudioBuffer<float>& buffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();
    if (numChannels <= 0 || numSamples <= 0)
        return;

    float peak = 0.0f;
    for (int channel = 0; channel < numChannels; ++channel)
        peak = juce::jmax(peak, buffer.getMagnitude(channel, 0, numSamples));

    if (! std::isfinite(peak) || peak <= 0.0f)
        return;

    buffer.applyGain(1.0f / peak);
}

bool AudioUtil::resampleAudioBuffer(const juce::AudioBuffer<float>& source, const double sourceSampleRate,
                                    const double targetSampleRate, juce::AudioBuffer<float>& destination)
{
    if (source.getNumChannels() <= 0 || source.getNumSamples() <= 0 || sourceSampleRate <= 0.0 || targetSampleRate <= 0.0)
        return false;

    if (juce::approximatelyEqual(sourceSampleRate, targetSampleRate))
    {
        destination.makeCopyOf(source);
        return true;
    }

    const auto outputSampleCount = juce::jmax(1, juce::roundToInt(static_cast<double>(source.getNumSamples())
                                                                   * targetSampleRate / sourceSampleRate));
    destination.setSize(source.getNumChannels(), outputSampleCount, false, false, true);
    destination.clear();

    const auto speedRatio = sourceSampleRate / targetSampleRate;
    for (int channel = 0; channel < source.getNumChannels(); ++channel)
    {
        juce::LagrangeInterpolator interpolator;
        interpolator.reset();
        interpolator.process(speedRatio,
                             source.getReadPointer(channel),
                             destination.getWritePointer(channel),
                             outputSampleCount);
    }

    return true;
}

bool AudioUtil::renderAudioBufferToChannelCount(const juce::AudioBuffer<float>& source, const int targetChannelCount,
                                                juce::AudioBuffer<float>& destination)
{
    if (source.getNumChannels() <= 0 || source.getNumSamples() <= 0 || targetChannelCount <= 0)
        return false;

    if (source.getNumChannels() == targetChannelCount)
    {
        destination.makeCopyOf(source);
        return true;
    }

    destination.setSize(targetChannelCount, source.getNumSamples(), false, false, true);
    destination.clear();

    if (targetChannelCount == 1)
    {
        const auto mixGain = 1.0f / static_cast<float>(source.getNumChannels());
        for (int channel = 0; channel < source.getNumChannels(); ++channel)
            destination.addFrom(0, 0, source, channel, 0, source.getNumSamples(), mixGain);

        return true;
    }

    if (source.getNumChannels() == 1)
    {
        for (int channel = 0; channel < targetChannelCount; ++channel)
            destination.copyFrom(channel, 0, source, 0, 0, source.getNumSamples());

        return true;
    }

    for (int channel = 0; channel < targetChannelCount; ++channel)
    {
        const auto sourceChannel = juce::jmin(channel, source.getNumChannels() - 1);
        destination.copyFrom(channel, 0, source, sourceChannel, 0, source.getNumSamples());
    }

    return true;
}

std::vector<juce::int64> AudioUtil::buildCueOffsetsFromSegments(const std::vector<Chain::Segment>& segments)
{
    std::vector<juce::int64> cueOffsets;
    cueOffsets.reserve(segments.size());

    for (const auto& segment : segments)
    {
        if (segment.sampleCount > 0 && segment.startSample >= 0)
            cueOffsets.push_back(segment.startSample);
    }

    return cueOffsets;
}

std::uint32_t AudioUtil::readU32LE(const std::uint8_t* data)
{
    return static_cast<std::uint32_t>(data[0])
           | (static_cast<std::uint32_t>(data[1]) << 8)
           | (static_cast<std::uint32_t>(data[2]) << 16)
           | (static_cast<std::uint32_t>(data[3]) << 24);
}

void AudioUtil::writeFourCC(std::uint8_t*& dest, const std::array<char, 4>& id)
{
    std::memcpy(dest, id.data(), id.size());
    dest += id.size();
}

void AudioUtil::writeU16LE(std::uint8_t*& dest, const std::uint16_t value)
{
    *dest++ = static_cast<std::uint8_t>(value & 0xFFu);
    *dest++ = static_cast<std::uint8_t>((value >> 8) & 0xFFu);
}

void AudioUtil::writeU32LE(std::uint8_t*& dest, const std::uint32_t value)
{
    *dest++ = static_cast<std::uint8_t>(value & 0xFFu);
    *dest++ = static_cast<std::uint8_t>((value >> 8) & 0xFFu);
    *dest++ = static_cast<std::uint8_t>((value >> 16) & 0xFFu);
    *dest++ = static_cast<std::uint8_t>((value >> 24) & 0xFFu);
}

bool AudioUtil::writePcmSample(std::uint8_t*& dest, const float sample, const int bitDepth)
{
    const auto clampedSample = juce::jlimit(-1.0f, 1.0f, sample);

    if (bitDepth == 16)
    {
        const auto pcm = static_cast<std::int16_t>(juce::jlimit(-32768, 32767,
                                                                  juce::roundToInt(clampedSample * 32767.0f)));
        writeU16LE(dest, static_cast<std::uint16_t>(pcm));
        return true;
    }

    if (bitDepth == 24)
    {
        const auto pcm = juce::jlimit(-8388608, 8388607,
                                       juce::roundToInt(clampedSample * 8388607.0f));
        const auto encoded = static_cast<std::uint32_t>(pcm);
        *dest++ = static_cast<std::uint8_t>(encoded & 0xFFu);
        *dest++ = static_cast<std::uint8_t>((encoded >> 8) & 0xFFu);
        *dest++ = static_cast<std::uint8_t>((encoded >> 16) & 0xFFu);
        return true;
    }

    return false;
}

bool AudioUtil::writeWavFile(const juce::File& file, const juce::AudioBuffer<float>& audioData, const double sampleRate,
                             const int bitDepth, const std::vector<juce::int64>* cueOffsets,
                             juce::String* errorMessage)
{
    if (errorMessage != nullptr)
        errorMessage->clear();

    if (file == juce::File{})
    {
        if (errorMessage != nullptr)
            *errorMessage = "No output file was selected.";
        return false;
    }

    if (sampleRate <= 0.0 || audioData.getNumChannels() <= 0 || audioData.getNumSamples() <= 0)
    {
        if (errorMessage != nullptr)
            *errorMessage = "The rendered audio is invalid.";
        return false;
    }

    if (bitDepth != 16 && bitDepth != 24)
    {
        if (errorMessage != nullptr)
            *errorMessage = "Unsupported WAV bit depth.";
        return false;
    }

    const auto numChannels = audioData.getNumChannels();
    const auto numSamples = audioData.getNumSamples();
    const auto bytesPerSample = bitDepth / 8;
    const auto sampleRateInt = static_cast<std::uint32_t>(juce::roundToInt(sampleRate));
    const auto byteRate64 = static_cast<std::uint64_t>(sampleRateInt)
                            * static_cast<std::uint64_t>(numChannels)
                            * static_cast<std::uint64_t>(bytesPerSample);
    const auto dataChunkSize64 = static_cast<std::uint64_t>(numChannels)
                                 * static_cast<std::uint64_t>(numSamples)
                                 * static_cast<std::uint64_t>(bytesPerSample);
    if (dataChunkSize64 > 0xFFFFFFFFull)
    {
        if (errorMessage != nullptr)
            *errorMessage = "The rendered WAV is too large to write safely.";
        return false;
    }

    std::vector<juce::int64> validCueOffsets;
    if (cueOffsets != nullptr)
    {
        validCueOffsets.reserve(cueOffsets->size());
        for (const auto offset : *cueOffsets)
        {
            if (offset >= 0 && offset < numSamples)
                validCueOffsets.push_back(offset);
        }

        std::sort(validCueOffsets.begin(), validCueOffsets.end());
        validCueOffsets.erase(std::unique(validCueOffsets.begin(), validCueOffsets.end()), validCueOffsets.end());
    }

    const auto cueChunkSize64 = validCueOffsets.empty() ? 0ull : 4ull + static_cast<std::uint64_t>(validCueOffsets.size()) * 24ull;
    const auto riffChunkSize64 = 4ull
                                 + (8ull + 16ull)
                                 + (8ull + dataChunkSize64)
                                 + (cueChunkSize64 > 0 ? 8ull + cueChunkSize64 : 0ull);
    if (riffChunkSize64 > 0xFFFFFFFFull)
    {
        if (errorMessage != nullptr)
            *errorMessage = "The rendered WAV is too large to write safely.";
        return false;
    }

    const auto totalSize = static_cast<size_t>(riffChunkSize64 + 8ull);
    if (totalSize > static_cast<size_t>(0x7FFFFFFF))
    {
        if (errorMessage != nullptr)
            *errorMessage = "The rendered WAV is too large to write safely.";
        return false;
    }

    std::vector<std::uint8_t> buffer(totalSize);
    std::uint8_t* writePos = buffer.data();

    writeFourCC(writePos, riffChunkId);
    writeU32LE(writePos, static_cast<std::uint32_t>(riffChunkSize64));
    writeFourCC(writePos, waveFormatId);

    writeFourCC(writePos, fmtChunkId);
    writeU32LE(writePos, 16u);
    writeU16LE(writePos, pcmFormat);
    writeU16LE(writePos, static_cast<std::uint16_t>(numChannels));
    writeU32LE(writePos, sampleRateInt);
    writeU32LE(writePos, static_cast<std::uint32_t>(byteRate64));
    writeU16LE(writePos, static_cast<std::uint16_t>(numChannels * bytesPerSample));
    writeU16LE(writePos, static_cast<std::uint16_t>(bitDepth));

    writeFourCC(writePos, dataChunkId);
    writeU32LE(writePos, static_cast<std::uint32_t>(dataChunkSize64));

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            if (! writePcmSample(writePos, audioData.getSample(channel, sampleIndex), bitDepth))
            {
                if (errorMessage != nullptr)
                    *errorMessage = "Unsupported WAV bit depth.";
                return false;
            }
        }
    }

    if (! validCueOffsets.empty())
    {
        writeFourCC(writePos, cueChunkId);
        writeU32LE(writePos, static_cast<std::uint32_t>(cueChunkSize64));
        writeU32LE(writePos, static_cast<std::uint32_t>(validCueOffsets.size()));

        for (std::size_t i = 0; i < validCueOffsets.size(); ++i)
        {
            writeU32LE(writePos, static_cast<std::uint32_t>(i + 1));
            const auto cueOffset = static_cast<std::uint32_t>(validCueOffsets[i]);
            writeU32LE(writePos, cueOffset);
            writeFourCC(writePos, dataChunkId);
            writeU32LE(writePos, 0u);
            writeU32LE(writePos, 0u);
            writeU32LE(writePos, cueOffset);
        }
    }

    return file.replaceWithData(buffer.data(), static_cast<std::size_t>(buffer.size()));
}

bool AudioUtil::readWaveCueOffsets(const juce::File& file, std::vector<juce::int64>& cueOffsets)
{
    cueOffsets.clear();

    juce::FileInputStream stream(file);
    if (! stream.openedOk())
        return false;

    if (stream.getTotalLength() < 12)
        return false;

    std::array<std::uint8_t, 12> header{};
    const auto headerSize = static_cast<int>(header.size());
    if (stream.read(header.data(), headerSize) != headerSize)
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
        constexpr auto chunkHeaderSize = static_cast<int>(chunkHeader.size());
        if (stream.read(chunkHeader.data(), chunkHeaderSize) != chunkHeaderSize)
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
            constexpr auto countBytesSize = static_cast<int>(countBytes.size());
            if (stream.read(countBytes.data(), countBytesSize) != countBytesSize)
                return false;

            const auto cueCount = readU32LE(countBytes.data());
            const auto cueDataSize = chunkSize - 4;
            if (cueCount > 0 && cueDataSize / 24 < cueCount)
                return false;

            cueOffsets.reserve(cueOffsets.size() + static_cast<size_t>(cueCount));

            for (std::uint32_t i = 0; i < cueCount; ++i)
            {
                std::array<std::uint8_t, 24> cuePoint{};
                constexpr auto cuePointSize = static_cast<int>(cuePoint.size());
                if (stream.read(cuePoint.data(), cuePointSize) != cuePointSize)
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

std::vector<juce::Range<juce::int64>> AudioUtil::buildSliceRangesFromCueOffsets(
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

// Resamples and sums channels to mono if necessary. Note: if target channel count is greater than source channel count,
// the output will be the same as the source channel count. It will be cheaper to upmix at the latest stage possible. Either
// in the audio engine for playback or when writing to a file.
std::shared_ptr<AudioClip> AudioUtil::renderPlaybackClip(AudioClip clip, const double targetSamplerate, const int targetChannelCount)
{
    if (targetSamplerate <= 0 || targetChannelCount <= 0)
    {
        DBG("Invalid target sample rate or channel count for rendering audio clip");
        return nullptr;
    }

    const auto& source = clip.getAudioData();
    const int sourceChannelCount = source.getNumChannels();
    const int sourceSampleCount = source.getNumSamples();

    const int outputChannelCount = juce::jmin(sourceChannelCount, targetChannelCount);
    const bool needsResample = ! juce::approximatelyEqual(clip.getSampleRate(), targetSamplerate);
    const bool needsDownmixToMono = outputChannelCount < sourceChannelCount;

    if (!needsResample && !needsDownmixToMono)
        return std::make_shared<AudioClip>(std::move(clip));

    const int outputSampleCount = needsResample
        ? juce::jmax(1, static_cast<int>(static_cast<double>(sourceSampleCount) * targetSamplerate / clip.getSampleRate()))
        : sourceSampleCount;
    juce::AudioBuffer<float> output;
    output.setSize(outputChannelCount, outputSampleCount, false, false, true);
    output.clear();

    const double speedRatio = clip.getSampleRate() / targetSamplerate;

    // Resample only
    if (!needsDownmixToMono)
    {
        for (int ch = 0; ch < outputChannelCount; ++ch)
        {
            if (!needsResample)
            {
                output.copyFrom(ch, 0, source, ch, 0, sourceSampleCount);
                continue;
            }

            juce::LagrangeInterpolator interpolator;
            interpolator.reset();
            interpolator.process(speedRatio,
                                 source.getReadPointer(ch),
                                 output.getWritePointer(ch),
                                 outputSampleCount);
        }

        return std::make_shared<AudioClip>(std::move(output),
                                                   needsResample ? targetSamplerate : clip.getSampleRate());
    }

    // Downmix only or resample + downmix, both to mono.
    juce::AudioBuffer<float> monoScratch;
    monoScratch.setSize(1, outputSampleCount, false, false, true);
    monoScratch.clear();

    const float mixGain = 1.0f / static_cast<float>(sourceChannelCount);

    for (int ch = 0; ch < sourceChannelCount; ++ch)
    {
        if (!needsResample)
        {
            output.addFrom(0, 0, source, ch, 0, sourceSampleCount, mixGain);
            continue;
        }

        monoScratch.clear();

        juce::LagrangeInterpolator interpolator;
        interpolator.reset();
        interpolator.process(speedRatio,
                             source.getReadPointer(ch),
                             monoScratch.getWritePointer(0),
                             outputSampleCount);

        output.addFrom(0, 0, monoScratch, 0, 0, outputSampleCount, mixGain);
    }

    return std::make_shared<AudioClip>(std::move(output),
                                               needsResample ? targetSamplerate : clip.getSampleRate());
}
