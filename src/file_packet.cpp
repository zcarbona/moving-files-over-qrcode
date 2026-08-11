#include "../include/file_packet.hpp"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "../include/crypto.hpp"

namespace file_packet
{

std::vector<std::uint8_t> serialize(const FilePacket& packet)
{
    std::vector<std::uint8_t> result;

    auto write_u32 = [&](std::uint32_t value)
    {
        for (int i = 0; i < 4; ++i)
        {
            result.push_back(
                static_cast<std::uint8_t>(
                    (value >> (i * 8)) & 0xFF
                )
            );
        }
    };

    auto write_u64 = [&](std::uint64_t value)
    {
        for (int i = 0; i < 8; ++i)
        {
            result.push_back(
                static_cast<std::uint8_t>(
                    (value >> (i * 8)) & 0xFF
                )
            );
        }
    };

    write_u32(packet.packet_id);
    write_u32(packet.chunk_index);
    write_u32(packet.total_chunks);

    const std::uint32_t filetype_length =
        static_cast<std::uint32_t>(packet.filetype.size());

    write_u32(filetype_length);

    for (char c : packet.filetype)
    {
        result.push_back(
            static_cast<std::uint8_t>(c)
        );
    }

    write_u64(packet.filesize);

    result.push_back(
        static_cast<std::uint8_t>(packet.encryption_enabled ? 1 : 0)
    );

    if (packet.encryption_enabled)
    {
        result.insert(
            result.end(),
            packet.salt.begin(),
            packet.salt.end()
        );

        result.insert(
            result.end(),
            packet.nonce.begin(),
            packet.nonce.end()
        );
    }

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
    packet.encryption_enabled = false;

    const std::size_t min_header_size =
        4 + 4 + 4 + 4 + 8;

    if (data.size() < min_header_size)
    {
        throw std::runtime_error(
            "Invalid packet: packet is too small"
        );
    }

    std::size_t offset = 0;

    auto read_u32 = [&](std::uint32_t& value)
    {
        if (offset + 4 > data.size())
        {
            throw std::runtime_error(
                "Invalid packet: not enough bytes for u32"
            );
        }

        value = 0;

        for (int i = 0; i < 4; ++i)
        {
            value |=
                static_cast<std::uint32_t>(data[offset + i])
                << (i * 8);
        }

        offset += 4;
    };

    auto read_u64 = [&](std::uint64_t& value)
    {
        if (offset + 8 > data.size())
        {
            throw std::runtime_error(
                "Invalid packet: not enough bytes for u64"
            );
        }

        value = 0;

        for (int i = 0; i < 8; ++i)
        {
            value |=
                static_cast<std::uint64_t>(data[offset + i])
                << (i * 8);
        }

        offset += 8;
    };

    read_u32(packet.packet_id);
    read_u32(packet.chunk_index);
    read_u32(packet.total_chunks);

    std::uint32_t filetype_length = 0;

    read_u32(filetype_length);

    if (offset + filetype_length > data.size())
    {
        throw std::runtime_error(
            "Invalid packet: file type exceeds packet size"
        );
    }

    packet.filetype = std::string(
        data.begin() + offset,
        data.begin() + offset + filetype_length
    );

    offset += filetype_length;

    read_u64(packet.filesize);

    if (offset < data.size())
    {
        const std::uint8_t flag = data[offset];

        if (flag == 0 || flag == 1)
        {
            packet.encryption_enabled = (flag == 1);
            offset += 1;

            if (packet.encryption_enabled)
            {
                if (offset + crypto::SALT_LEN + crypto::NONCE_LEN > data.size())
                {
                    throw std::runtime_error(
                        "Invalid packet: insufficient data for "
                        "encryption metadata"
                    );
                }

                packet.salt.assign(
                    data.begin() + offset,
                    data.begin() + offset + crypto::SALT_LEN
                );

                offset += crypto::SALT_LEN;

                packet.nonce.assign(
                    data.begin() + offset,
                    data.begin() + offset + crypto::NONCE_LEN
                );

                offset += crypto::NONCE_LEN;
            }
        }
    }

    if (offset > data.size())
    {
        throw std::runtime_error(
            "Invalid packet: data start exceeds packet size"
        );
    }

    packet.data = std::vector<std::uint8_t>(
        data.begin() + offset,
        data.end()
    );

    return packet;
}


std::vector<FilePacket> create_packets(
    const std::vector<std::uint8_t>& file_data,
    const std::string& filetype,
    const std::string& password
)
{
    if (file_data.empty())
    {
        throw std::runtime_error(
            "Cannot create packets from empty file data"
        );
    }

    if (filetype.empty())
    {
        throw std::runtime_error(
            "Cannot create packets with empty filetype"
        );
    }

    const std::uint64_t file_size =
        static_cast<std::uint64_t>(file_data.size());

    const std::size_t total_chunks =
        (file_size + QR_CHUNK_SIZE - 1) / QR_CHUNK_SIZE;

    if (total_chunks > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::runtime_error(
            "File too large: too many chunks"
        );
    }

    std::vector<FilePacket> packets;
    packets.reserve(total_chunks);

    const std::uint32_t packet_id = 1;

    for (std::size_t i = 0; i < total_chunks; ++i)
    {
        FilePacket packet;

        packet.packet_id = packet_id;
        packet.chunk_index = static_cast<std::uint32_t>(i);
        packet.total_chunks = static_cast<std::uint32_t>(total_chunks);
        packet.filetype = filetype;
        packet.filesize = file_size;
        packet.encryption_enabled = false;

        const std::size_t start = i * QR_CHUNK_SIZE;
        const std::size_t remaining =
            file_size - start;

        const std::size_t chunk_size =
            (remaining < QR_CHUNK_SIZE)
                ? remaining
                : QR_CHUNK_SIZE;

        packet.data = std::vector<std::uint8_t>(
            file_data.begin() + start,
            file_data.begin() + start + chunk_size
        );

        packets.push_back(std::move(packet));
    }

    return packets;
}


std::vector<std::uint8_t> reconstruct_file(
    std::vector<FilePacket> packets
)
{
    if (packets.empty())
    {
        throw std::runtime_error(
            "Cannot reconstruct from empty packets"
        );
    }

    std::sort(
        packets.begin(),
        packets.end(),
        [](const FilePacket& a, const FilePacket& b)
        {
            return a.chunk_index < b.chunk_index;
        }
    );

    for (std::size_t i = 0; i < packets.size(); ++i)
    {
        if (packets[i].packet_id != packets[0].packet_id)
        {
            throw std::runtime_error(
                "Invalid packet: packets belong to different transfers"
            );
        }

        if (packets[i].total_chunks != packets[0].total_chunks)
        {
            throw std::runtime_error(
                "Invalid packet: total_chunks differs between packets"
            );
        }

        if (packets[i].filesize != packets[0].filesize)
        {
            throw std::runtime_error(
                "Invalid packet: filesize differs between packets"
            );
        }
    }

    for (std::size_t i = 1; i < packets.size(); ++i)
    {
        if (packets[i].chunk_index == packets[i - 1].chunk_index)
        {
            throw std::runtime_error(
                "Invalid packet: duplicate chunk detected"
            );
        }
    }

    const std::uint32_t expected_total =
        packets[0].total_chunks;

    if (packets.size() != expected_total)
    {
        throw std::runtime_error(
            "Invalid packet: missing chunks. Expected " +
            std::to_string(expected_total) +
            ", got " +
            std::to_string(packets.size())
        );
    }

    for (std::size_t i = 0; i < packets.size(); ++i)
    {
        if (packets[i].chunk_index != static_cast<std::uint32_t>(i))
        {
            throw std::runtime_error(
                "Invalid packet: chunk indexes are not sequential"
            );
        }
    }

    std::vector<std::uint8_t> result;
    result.reserve(packets[0].filesize);

    for (const auto& packet : packets)
    {
        result.insert(
            result.end(),
            packet.data.begin(),
            packet.data.end()
        );
    }

    if (result.size() != packets[0].filesize)
    {
        throw std::runtime_error(
            "Invalid packet: reconstructed size does not match filesize"
        );
    }

    return result;
}


void decrypt_packet_data(
    FilePacket& packet,
    const std::string& password
)
{
    if (!packet.encryption_enabled || password.empty())
    {
        return;
    }

    std::vector<std::uint8_t> hex_data =
        crypto::decrypt_chunk(
            packet.data,
            password,
            packet.salt,
            packet.nonce
        );

    std::string hex_str(
        reinterpret_cast<const char*>(hex_data.data()),
        hex_data.size()
    );

    packet.data = crypto::hex_decode(hex_str);

    packet.encryption_enabled = false;
    packet.salt.clear();
    packet.nonce.clear();
}

} // namespace file_packet
