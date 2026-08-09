
#include "../include/file_io.hpp"
#include "../include/file_packet.hpp"
#include "../include/qr.hpp"

#include <filesystem>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

std::string make_qr_filename(std::size_t index)
{
    std::ostringstream oss;

    oss << "qr_"
        << std::setw(4)
        << std::setfill('0')
        << index
        << ".pgm";

    return oss.str();
}

void print_usage(const char* program)
{
    std::cerr
        << "Usage:\n"
        << "  " << program << " -e <file>\n"
        << "  " << program << " -d <path>\n\n"
        << "Examples:\n"
        << "  " << program << " -e image.png\n"
        << "  " << program << " -d ./qr_codes\n";
}

int encode_file(const std::filesystem::path& input_path)
{
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
        << "Chunk size: "
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
            << " generated: "
            << qr_filename
            << '\n';
    }

    std::cout
        << "\nEncoding complete.\n"
        << "Generated "
        << packets.size()
        << " QR code(s).\n";

    return 0;
}

int decode_files(const std::filesystem::path& input_path)
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

    /*
     * Find all .pgm QR files in the directory.
     */
    std::vector<std::filesystem::path> qr_files;

    for (const auto& entry :
         std::filesystem::directory_iterator(input_path))
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
            << "No .pgm QR codes found in: "
            << input_path
            << '\n';

        return 1;
    }

    /*
     * Sort QR files so that:
     *
     * qr_0000.pgm
     * qr_0001.pgm
     * qr_0002.pgm
     *
     * are decoded in the correct order.
     */
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

    std::cout
        << "Decoding:\n";

    for (std::size_t i = 0; i < qr_files.size(); ++i)
    {
        const auto& qr_filename =
            qr_files[i];

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

    /*
     * Get the original filename from the packet metadata.
     *
     * This assumes your FilePacket/reconstruct implementation
     * already preserves the filename. If it doesn't, change
     * this to whatever filename you want.
     */
    std::filesystem::path output_path =
        input_path / "decoded_file";

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

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 3)
        {
            print_usage(argv[0]);
            return 1;
        }

        const std::string mode = argv[1];
        const std::filesystem::path path = argv[2];

        /*
         * Encode:
         *
         * ./app -e image.png
         */
        if (mode == "-e")
        {
            return encode_file(path);
        }

        /*
         * Decode:
         *
         * ./app -d ./qr_codes
         */
        if (mode == "-d")
        {
            return decode_files(path);
        }

        std::cerr
            << "Unknown option: "
            << mode
            << "\n\n";

        print_usage(argv[0]);

        return 1;
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
