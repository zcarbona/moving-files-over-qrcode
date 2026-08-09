#include "../include/file_packet.hpp"

#include <stdexcept>

namespace file_packet
{

std::vector<std::uint8_t> serialize(const FilePacket& packet)
{
    std::vector<std::uint8_t> result;

    // Serialize file type length (4 bytes)
    const std::uint32_t filetype_length =
        static_cast<std::uint32_t>(packet.filetype.size());

    for (int i = 0; i < 4; ++i)
    {
        result.push_back(
            static_cast<std::uint8_t>(
                (filetype_length >> (i * 8)) & 0xFF
            )
        );
    }

    // Serialize file type
    for (char c : packet.filetype)
    {
        result.push_back(
            static_cast<std::uint8_t>(c)
        );
    }

    // Serialize file size (8 bytes)
    for (int i = 0; i < 8; ++i)
    {
        result.push_back(
            static_cast<std::uint8_t>(
                (packet.filesize >> (i * 8)) & 0xFF
            )
        );
    }

    // Serialize file data
    result.insert(
        result.end(),
        packet.data.begin(),
        packet.data.end()
    );

    return result;
}


FilePacket deserialize(const std::vector<std::uint8_t>& data)
{
    FilePacket packet;

    // We need at least:
    // 4 bytes for type length
    // 8 bytes for file size
    if (data.size() < 12)
    {
        throw std::runtime_error(
            "Invalid packet: packet is too small"
        );
    }

    // Deserialize file type length
    std::uint32_t filetype_length = 0;

    for (int i = 0; i < 4; ++i)
    {
        filetype_length |=
            static_cast<std::uint32_t>(data[i])
            << (i * 8);
    }

    // Check that type actually fits inside packet
    const std::size_t type_start = 4;
    const std::size_t type_end =
        type_start + filetype_length;

    if (type_end > data.size())
    {
        throw std::runtime_error(
            "Invalid packet: file type exceeds packet size"
        );
    }

    // Deserialize file type
    packet.filetype = std::string(
        data.begin() + type_start,
        data.begin() + type_end
    );

    // Deserialize file size
    packet.filesize = 0;

    const std::size_t filesize_start = type_end;

    if (filesize_start + 8 > data.size())
    {
        throw std::runtime_error(
            "Invalid packet: missing file size"
        );
    }

    for (int i = 0; i < 8; ++i)
    {
        packet.filesize |=
            static_cast<std::uint64_t>(
                data[filesize_start + i]
            )
            << (i * 8);
    }

    // Deserialize file data
    const std::size_t filedata_start =
        filesize_start + 8;

    packet.data = std::vector<std::uint8_t>(
        data.begin() + filedata_start,
        data.end()
    );

    // Validate file size
    if (packet.data.size() != packet.filesize)
    {
        throw std::runtime_error(
            "Invalid packet: file size does not match data size"
        );
    }

    return packet;
}

} // namespace file_packet