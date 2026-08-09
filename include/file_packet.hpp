#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace file_packet {

    struct FilePacket{
        std::string filetype;
        std::uint64_t filesize;
        std::vector<std::uint8_t> data;
    };
    std::vector<std::uint8_t> serialize(const FilePacket& packet);
    FilePacket deserialize(const std::vector<std::uint8_t>& data);
} // namespace file_packet