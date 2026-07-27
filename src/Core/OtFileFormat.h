#pragma once

#include <array>
#include <cstdint>

/*
OT file layout, based on OctaTrack OS 1.40A.

Total size: 832 bytes

Offset   Size   Field
0x0000   16     Header
               Bytes: 46 4F 52 4D 00 00 00 00 44 50 53 31 53 4D 50 41
               ASCII: "FORM....DPS1SMPA"

0x0010    7     Unknown block
               Writer currently uses:
               00 00 00 00 00 02 00

0x0017    4     Tempo
               u32 big-endian
               Stored as BPM * 24

0x001B    4     Trim length
               u32 big-endian
               Written from BPM, sample rate, and total samples

0x001F    4     Loop length
               u32 big-endian
               Same value as trim length in current writer

0x0023    4     Timestretch
               u32 big-endian
               Enum values:
               0 = off
               2 = normal
               3 = beat

0x0027    4     Loop mode
               u32 big-endian
               Enum values:
               0 = off
               1 = on
               2 = pingpong

0x002B    2     Gain
               u16 big-endian
               Stored as gain*2 + 48

0x002D    1     Trig quant
               1 byte
               Values:
               0xFF = direct
               0x00 = pattern
               0x01..0x10 = stepped quant values

0x002E    4     Trim start
               u32 big-endian

0x0032    4     Trim end
               u32 big-endian

0x0036    4     Loop point
               u32 big-endian

0x003A  768     Slice table
               64 entries * 12 bytes each
               Each slice entry:
                 startPoint  u32 BE
                 endPoint    u32 BE
                 loopPoint   u32 BE
               Entry i starts at 0x003A + (i * 12)

0x033A    4     Slice count
               u32 big-endian

0x033E    2     Checksum
               u16 big-endian
               Sum of bytes 0x0010..0x033D, then keep low 16 bits
*/


class OtFileFormat
{
public:

    static constexpr std::size_t otFileSize = 832;
    static constexpr std::size_t maxSliceCount = 64;

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
