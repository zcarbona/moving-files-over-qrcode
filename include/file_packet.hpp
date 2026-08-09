#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace file_packet {

constexpr std::size_t QR_CHUNK_SIZE = 2000;

struct FilePacket{
    std::uint32_t packet_id;
    std::uint32_t chunk_index;
    std::uint32_t total_chunks;
    std::string filetype;
    std::uint64_t filesize;
    std::vector<std::uint8_t> data;
};

std::vector<std::uint8_t> serialize(const FilePacket& packet);
FilePacket deserialize(const std::vector<std::uint8_t>& data);

std::vector<FilePacket> create_packets(
    const std::vector<std::uint8_t>& file_data,
    const std::string& filetype
);

std::vector<std::uint8_t> reconstruct_file(
    std::vector<FilePacket> packets
);

} // namespace file_packet