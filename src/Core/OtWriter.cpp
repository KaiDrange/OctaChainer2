#include "OtWriter.h"

#include <utility>

OtWriter::OtWriter(juce::File outputFile,
                   const int sampleRate,
                   const OtFileFormat::Loop_t loopSetting,
                   const OtFileFormat::Stretch_t stretchSetting,
                   const OtFileFormat::TrigQuant_t trigQuantSetting,
                   const int gain,
                   const int tempo)
    : outputFile(std::move(outputFile)),
      sampleRate(sampleRate),
      tempo(tempo),
      gain(gain),
      loopSetting(loopSetting),
      stretchSetting(stretchSetting),
      trigQuantSetting(trigQuantSetting)
{
}

bool OtWriter::write(const std::uint32_t totalSamples) const
{
    if (sampleRate <= 0 || outputFile.getFullPathName().isEmpty())
        return false;

    if (!outputFile.getParentDirectory().createDirectory())
        return false;

    const auto payload = buildPayload(totalSamples);
    juce::FileOutputStream stream(outputFile);

    if (! stream.openedOk())
        return false;

    if (! stream.write(payload.data(), payload.size()))
        return false;

    stream.flush();
    return true;
}

void OtWriter::addSlice(const std::uint32_t start, const std::uint32_t end)
{
    slices.push_back(OtFileFormat::Slice{ start, end, 0xFFFFFFFFu });
}

std::array<std::uint8_t, OtFileFormat::otFileSize> OtWriter::buildPayload(const std::uint32_t totalSamples) const
{
    std::array<std::uint8_t, OtFileFormat::otFileSize > payload{};
    std::uint8_t* writePos = payload.data();

    std::memcpy(writePos, OtFileFormat::headerBytes.data(), OtFileFormat::headerBytes.size());
    writePos += OtFileFormat::headerBytes.size();

    std::memcpy(writePos, OtFileFormat::unknownBytes.data(), OtFileFormat::unknownBytes.size());
    writePos += OtFileFormat::unknownBytes.size();

    writeU32BE(writePos, static_cast<std::uint32_t>(tempo * 6));

    const auto barUnits = static_cast<std::uint32_t>(
                              static_cast<double>(tempo) * static_cast<double>(totalSamples)
                               / (static_cast<double>(sampleRate) * 60.0 * 4.0) + 0.5)
                          * 25u;

    writeU32BE(writePos, barUnits);
    writeU32BE(writePos, barUnits);
    writeU32BE(writePos, static_cast<std::uint32_t>(stretchSetting));
    writeU32BE(writePos, static_cast<std::uint32_t>(loopSetting));
    writeU16BE(writePos, static_cast<std::uint16_t>(gain + 48));

    *writePos++ = static_cast<std::uint8_t>(trigQuantSetting);

    writeU32BE(writePos, 0);
    writeU32BE(writePos, totalSamples);
    writeU32BE(writePos, 0);

    const auto sliceCount = (slices.size() > 1)
                                ? (slices.size() < 64 ? slices.size() : static_cast<std::size_t>(64))
                                : static_cast<std::size_t>(0);

    for (std::size_t i = 0; i < 64; ++i)
    {
        const OtFileFormat::Slice slice = i < sliceCount ? slices[i] : OtFileFormat::Slice{};
        writeU32BE(writePos, slice.startPoint);
        writeU32BE(writePos, slice.endPoint);
        writeU32BE(writePos, slice.loopPoint);
    }

    writeU32BE(writePos, sliceCount);

    const auto sum = checksum(payload.data());
    writeU16BE(writePos, sum);

    return payload;
}

void OtWriter::writeU16BE(std::uint8_t*& dest, const std::uint16_t value)
{
    *dest++ = static_cast<std::uint8_t>((value >> 8) & 0xFF);
    *dest++ = static_cast<std::uint8_t>(value & 0xFF);
}

void OtWriter::writeU32BE(std::uint8_t*& dest, const std::uint32_t value)
{
    *dest++ = static_cast<std::uint8_t>((value >> 24) & 0xFF);
    *dest++ = static_cast<std::uint8_t>((value >> 16) & 0xFF);
    *dest++ = static_cast<std::uint8_t>((value >> 8) & 0xFF);
    *dest++ = static_cast<std::uint8_t>(value & 0xFF);
}

std::uint16_t OtWriter::checksum(const std::uint8_t* data)
{
    std::uint32_t sum = 0;

    for (std::size_t i = 16; i < OtFileFormat::otFileSize; ++i)
        sum += data[i];

    return static_cast<std::uint16_t>(sum & 0xFFFFu);
}
