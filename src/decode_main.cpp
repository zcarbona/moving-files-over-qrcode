#include "../include/file_io.hpp"
#include "../include/file_packet.hpp"
#include "../include/qr.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    try
    {
        const std::filesystem::path qr_directory = "../";
        const std::filesystem::path output_file = "decode.png";

        // ============================================================
        // 1. Find all PGM files
        // ============================================================

        std::vector<std::filesystem::path> qr_files;

        for (const auto& entry :
             std::filesystem::directory_iterator(qr_directory))
        {
            if (!entry.is_regular_file())
                continue;

            if (entry.path().extension() == ".pgm")
            {
                qr_files.push_back(entry.path());
            }
        }

        if (qr_files.empty())
        {
            std::cerr
                << "No PGM files found.\n";

            return 1;
        }

        // Sort filenames for predictable processing.
        std::sort(
            qr_files.begin(),
            qr_files.end()
        );

        std::cout
            << "Found "
            << qr_files.size()
            << " QR images.\n\n";


        // ============================================================
        // 2. Decode every QR
        // ============================================================

        std::vector<file_packet::FilePacket> packets;

        packets.reserve(qr_files.size());

        for (std::size_t i = 0;
             i < qr_files.size();
             ++i)
        {
            const auto& qr_file = qr_files[i];

            std::cout
                << "["
                << (i + 1)
                << "/"
                << qr_files.size()
                << "] Decoding "
                << qr_file.filename()
                << " ... ";

            auto serialized =
                qr::decode(qr_file);

            std::cout
                << "decoded "
                << serialized.size()
                << " bytes";

            // ========================================================
            // Deserialize packet
            // ========================================================

            auto packet =
                file_packet::deserialize(serialized);

            std::cout
                << " | chunk "
                << packet.chunk_index
                << "/"
                << packet.total_chunks
                << '\n';

            packets.push_back(
                std::move(packet)
            );
        }


        // ============================================================
        // 3. Basic validation
        // ============================================================

        if (packets.empty())
        {
            std::cerr
                << "No packets were decoded.\n";

            return 1;
        }

        const auto packet_id =
            packets.front().packet_id;

        const auto total_chunks =
            packets.front().total_chunks;

        const auto filesize =
            packets.front().filesize;

        const auto filetype =
            packets.front().filetype;


        std::cout
            << "\nTransfer information:\n"
            << "Packet ID: "
            << packet_id
            << '\n'
            << "Total chunks: "
            << total_chunks
            << '\n'
            << "Original file size: "
            << filesize
            << " bytes\n"
            << "File type: "
            << filetype
            << "\n\n";


        // ============================================================
        // 4. Validate packet metadata
        // ============================================================

        if (packets.size() != total_chunks)
        {
            std::cerr
                << "ERROR: Expected "
                << total_chunks
                << " chunks but decoded "
                << packets.size()
                << ".\n";

            return 1;
        }

        for (const auto& packet : packets)
        {
            if (packet.packet_id != packet_id)
            {
                std::cerr
                    << "ERROR: Packet ID mismatch.\n";

                return 1;
            }

            if (packet.total_chunks != total_chunks)
            {
                std::cerr
                    << "ERROR: total_chunks mismatch.\n";

                return 1;
            }

            if (packet.filesize != filesize)
            {
                std::cerr
                    << "ERROR: filesize mismatch.\n";

                return 1;
            }

            if (packet.filetype != filetype)
            {
                std::cerr
                    << "ERROR: filetype mismatch.\n";

                return 1;
            }
        }


        // ============================================================
        // 5. Sort packets by chunk index
        // ============================================================

        std::sort(
            packets.begin(),
            packets.end(),
            [](const auto& a, const auto& b)
            {
                return a.chunk_index < b.chunk_index;
            }
        );


        // ============================================================
        // 6. Check for missing / duplicate chunks
        // ============================================================

        for (std::uint32_t i = 0;
             i < total_chunks;
             ++i)
        {
            if (packets[i].chunk_index != i)
            {
                std::cerr
                    << "ERROR: Missing or duplicate chunk.\n"
                    << "Expected chunk: "
                    << i
                    << '\n'
                    << "Received chunk: "
                    << packets[i].chunk_index
                    << '\n';

                return 1;
            }
        }


        // ============================================================
        // 7. Reconstruct file
        // ============================================================

        std::cout
            << "Reconstructing file...\n";

        std::vector<std::uint8_t> reconstructed;

        reconstructed.reserve(
            static_cast<std::size_t>(filesize)
        );

        for (const auto& packet : packets)
        {
            reconstructed.insert(
                reconstructed.end(),
                packet.data.begin(),
                packet.data.end()
            );
        }


        // ============================================================
        // 8. Verify reconstructed size
        // ============================================================

        if (reconstructed.size() != filesize)
        {
            std::cerr
                << "ERROR: Reconstructed size mismatch.\n"
                << "Expected: "
                << filesize
                << '\n'
                << "Actual: "
                << reconstructed.size()
                << '\n';

            return 1;
        }


        // ============================================================
        // 9. Write reconstructed file
        // ============================================================

        if (!file_io::write_file(
                output_file,
                reconstructed))
        {
            std::cerr
                << "ERROR: Failed to write "
                << output_file
                << '\n';

            return 1;
        }


        // ============================================================
        // 10. Success
        // ============================================================

        std::cout
            << "\n========================================\n"
            << "      DECODING SUCCESSFUL\n"
            << "========================================\n"
            << "QR files:       "
            << qr_files.size()
            << '\n'
            << "Chunks:         "
            << packets.size()
            << '\n'
            << "Recovered size: "
            << reconstructed.size()
            << " bytes\n"
            << "Output:         "
            << output_file
            << '\n'
            << "========================================\n";

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "\nERROR: "
            << e.what()
            << '\n';

        return 1;
    }
}