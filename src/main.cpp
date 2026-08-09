#include "../include/file_io.hpp"
#include "../include/file_packet.hpp"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

int main()
{
    const std::filesystem::path input_path = "../test.txt";
    const std::filesystem::path output_path = "../test_copy.txt";

    // ============================================================
    // 1. Check if the file exists
    // ============================================================

    bool exists = file_io::file_exists(input_path);

    std::cout << std::boolalpha
              << "File exists: "
              << exists
              << '\n';

    if (!exists)
    {
        std::cerr << "Input file does not exist.\n";
        return 1;
    }


    // ============================================================
    // 2. Read the original file
    // ============================================================

    auto data = file_io::read_file(input_path);

    std::cout << "Original file size: "
              << data.size()
              << " bytes\n";


    // ============================================================
    // 3. Display original file bytes
    // ============================================================

    std::cout << "Original file binary:\n";

    for (const auto& byte : data)
    {
        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<int>(byte)
            << ' ';
    }

    std::cout << std::dec << "\n\n";


    // ============================================================
    // 4. Create FilePacket
    // ============================================================

    file_packet::FilePacket original;

    original.filetype = "text/plain";
    original.filesize = data.size();
    original.data = data;


    // ============================================================
    // 5. Serialize FilePacket -> binary vector
    // ============================================================

    auto serialized = file_packet::serialize(original);

    std::cout << "Serialized packet size: "
              << serialized.size()
              << " bytes\n";


    // ============================================================
    // 6. Deserialize binary vector -> FilePacket
    // ============================================================

    auto restored = file_packet::deserialize(serialized);

    std::cout << "Restored file type: "
              << restored.filetype
              << '\n';

    std::cout << "Restored file size: "
              << restored.filesize
              << " bytes\n";


    // ============================================================
    // 7. Verify the packet
    // ============================================================

    bool packet_valid =
        original.filetype == restored.filetype &&
        original.filesize == restored.filesize &&
        original.data == restored.data;

    std::cout << std::boolalpha
              << "Packet round trip valid: "
              << packet_valid
              << "\n\n";

    if (!packet_valid)
    {
        std::cerr << "Packet round trip failed.\n";
        return 1;
    }


    // ============================================================
    // 8. Write restored data to a new file
    // ============================================================

    bool written = file_io::write_file(
        output_path,
        restored.data
    );

    std::cout << std::boolalpha
              << "File written: "
              << written
              << '\n';


    return 0;
}