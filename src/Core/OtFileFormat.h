#pragma once

#include <array>
#include <cstdint>

class OtFileFormat
{
public:

    static constexpr std::size_t otFileSize = 832;

    enum Loop_t
    {
        NoLoop = 0,
        Loop = 1,
        PIPO = 2
    };

    enum Stretch_t
    {
        NoStretch = 0,
        Normal = 2,
        Beat = 3
    };

    enum TrigQuant_t
    {
        Direct = 0xFF,
        Pattern = 0,
        S_1 = 1,
        S_2 = 2,
        S_3 = 3,
        S_4 = 4,
        S_6 = 5,
        S_8 = 6,
        S_12 = 7,
        S_16 = 8,
        S_24 = 9,
        S_32 = 10,
        S_48 = 11,
        S_64 = 12,
        S_96 = 13,
        S_128 = 14,
        S_192 = 15,
        S_256 = 16
    };

#pragma pack(push, 1)
    struct Slice
    {
        std::uint32_t startPoint = 0;
        std::uint32_t endPoint = 0;
        std::uint32_t loopPoint = 0;
    };
#pragma pack(pop)

    static constexpr std::array<std::uint8_t, 16> headerBytes
    {
        0x46, 0x4F, 0x52, 0x4D,
        0x00, 0x00, 0x00, 0x00,
        0x44, 0x50, 0x53, 0x31,
        0x53, 0x4D, 0x50, 0x41
    };

    static constexpr std::array<std::uint8_t, 7> unknownBytes
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00
    };
};
