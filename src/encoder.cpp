#include "../include/encoder.hpp"
#include "../include/file_io.hpp"
#include "../include/file_packet.hpp"
#include "../include/qr.hpp"

#include <filesystem>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>



namespace
{
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
}

int encode_file(const std::filesystem::path& input_path,const std::string& filetype)
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

    auto original = file_io::read_file(input_path);

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
            filetype
        );

    std::cout
        << "Total chunks: "
        << packets.size()
        << "\n\n";

    std::cout << "Encoding:\n";

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