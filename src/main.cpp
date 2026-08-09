#include "../include/file_io.hpp"
#include "../include/file_packet.hpp"
#include "../include/qr.hpp"

#include <iomanip>
#include <sstream>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <vector>

std::string make_qr_filename(std::size_t index)
{
    std::ostringstream oss;
    oss << "qr_"
        << std::setw(4) << std::setfill('0')
        << index << ".pgm";
    return oss.str();
}

int main()
{
    try
    {
        const std::filesystem::path input_path =
            "test.png";

        const std::filesystem::path decoded_path =
            "decode.png";

        if (!file_io::file_exists(input_path))
        {
            std::cerr
                << "Input file does not exist: "
                << input_path
                << '\n';

            return 1;
        }

        std::cout
            << "Input file: "
            << input_path
            << '\n';

        auto original =
            file_io::read_file(input_path);

        std::cout
            << "Original size: "
            << original.size()
            << " bytes\n";

        std::cout
            << "\nChunk size: "
            << file_packet::QR_CHUNK_SIZE
            << " bytes\n";

        auto packets =
            file_packet::create_packets(
                original,
                "image/png"
            );

        std::cout
            << "Total chunks: "
            << packets.size()
            << "\n\n";

        std::cout
            << "Encoding:\n";

        for (std::size_t i = 0; i < packets.size(); ++i)
        {
            auto serialized =
                file_packet::serialize(packets[i]);

            const std::string qr_filename =
                make_qr_filename(i);

            qr::encode(
                serialized,
                qr_filename
            );

            std::cout
                << "QR " << (i + 1) << "/"
                << packets.size()
                << " generated\n";
        }

        std::cout
            << "\nDecoding:\n";

        std::vector<file_packet::FilePacket> decoded_packets;
        decoded_packets.reserve(packets.size());

        for (std::size_t i = 0; i < packets.size(); ++i)
        {
            const std::string qr_filename =
                make_qr_filename(i);

            auto decoded_bytes =
                qr::decode(qr_filename);

            auto packet =
                file_packet::deserialize(decoded_bytes);

            decoded_packets.push_back(std::move(packet));

            std::cout
                << "QR " << (i + 1) << "/"
                << packets.size()
                << " decoded\n";
        }

        std::cout
            << "\nReconstructing file...\n";

        auto reconstructed =
            file_packet::reconstruct_file(
                std::move(decoded_packets)
            );

        std::cout
            << "\nReconstructed size: "
            << reconstructed.size()
            << " bytes\n";

        const bool match = (original == reconstructed);

        std::cout
            << "Original/reconstructed match: "
            << std::boolalpha
            << match
            << '\n';

        bool written =
            file_io::write_file(
                decoded_path,
                reconstructed
            );

        if (!written)
        {
            std::cerr
                << "Failed to write decoded PNG.\n";

            return 1;
        }

        std::cout
            << "Decoded file written: "
            << decoded_path
            << '\n';

        std::cout
            << "\n========================================\n"
            << (match ? "FULL ROUND TRIP SUCCESS" : "FULL ROUND TRIP FAILURE")
            << "\n========================================\n"
            << "Input     : " << input_path << '\n'
            << "Decoded   : " << decoded_path << '\n'
            << "Size      : " << original.size() << " bytes\n"
            << "Chunks    : " << packets.size() << '\n'
            << "========================================\n";

        return match ? 0 : 1;
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "ERROR: "
            << e.what()
            << '\n';

        return 1;
    }
}
