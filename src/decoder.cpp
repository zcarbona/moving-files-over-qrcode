#include "../include/decoder.hpp"
#include "../include/file_io.hpp"
#include "../include/file_packet.hpp"
#include "../include/qr.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

int decode_files(const std::filesystem::path& input_path, std::string& extention)
{
    if (!std::filesystem::exists(input_path))
    {
        std::cerr
            << "Path does not exist: "
            << input_path
            << '\n';

        return 1;
    }

    if (!std::filesystem::is_directory(input_path))
    {
        std::cerr
            << "Decode path must be a directory: "
            << input_path
            << '\n';

        return 1;
    }

    std::vector<std::filesystem::path> qr_files;

    for (const auto& entry :
         std::filesystem::directory_iterator(input_path))
    {
        if (!entry.is_regular_file())
            continue;

        if (entry.path().extension() == ".pgm")
            qr_files.push_back(entry.path());
    }

    if (qr_files.empty())
    {
        std::cerr
            << "No .pgm QR codes found in: "
            << input_path
            << '\n';

        return 1;
    }

    std::sort(
        qr_files.begin(),
        qr_files.end()
    );

    std::cout
        << "QR path: "
        << input_path
        << '\n';

    std::cout
        << "Found "
        << qr_files.size()
        << " QR code(s).\n\n";

    std::vector<file_packet::FilePacket> decoded_packets;

    decoded_packets.reserve(qr_files.size());

    std::cout << "Decoding:\n";

    for (std::size_t i = 0; i < qr_files.size(); ++i)
    {
        const auto& qr_filename = qr_files[i];

        auto decoded_bytes =
            qr::decode(qr_filename.string());

        auto packet =
            file_packet::deserialize(decoded_bytes);

        decoded_packets.push_back(
            std::move(packet)
        );

        std::cout
            << "QR " << (i + 1) << "/"
            << qr_files.size()
            << " decoded: "
            << qr_filename.filename()
            << '\n';
    }

    std::cout
        << "\nReconstructing file...\n";

    auto reconstructed =
        file_packet::reconstruct_file(
            std::move(decoded_packets)
        );

    std::cout
        << "Reconstructed size: "
        << reconstructed.size()
        << " bytes\n";

    std::filesystem::path output_path =
        input_path / "decoded_file";
    
        output_path += extention;

    bool written =
        file_io::write_file(
            output_path,
            reconstructed
        );

    if (!written)
    {
        std::cerr
            << "Failed to write reconstructed file.\n";

        return 1;
    }

    std::cout
        << "\n========================================\n"
        << "DECODE SUCCESS\n"
        << "========================================\n"
        << "QR path : " << input_path << '\n'
        << "Output  : " << output_path << '\n'
        << "Size    : " << reconstructed.size() << " bytes\n"
        << "Chunks  : " << qr_files.size() << '\n'
        << "========================================\n";

    return 0;
}