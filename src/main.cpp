
#include "../include/file_io.hpp"
#include "../include/qr.hpp"

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <vector>

int main()
{
    try
    {
        // ============================================================
        // Paths
        // ============================================================

        const std::filesystem::path input_path =
            "test.png";

        const std::filesystem::path qr_path =
            "qr.pgm";

        const std::filesystem::path decoded_path =
            "decode.png";


        // ============================================================
        // 1. Check input file
        // ============================================================

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


        // ============================================================
        // 2. Read PNG as raw binary
        // ============================================================

        auto original =
            file_io::read_file(input_path);

        std::cout
            << "Original PNG size: "
            << original.size()
            << " bytes\n";


        // ============================================================
        // 3. Encode PNG bytes into QR
        // ============================================================

        std::cout
            << "Encoding PNG into QR...\n";

        qr::encode(
            original,
            qr_path
        );

        std::cout
            << "QR generated: "
            << qr_path
            << '\n';


        // ============================================================
        // 4. Decode QR back into PNG bytes
        // ============================================================

        std::cout
            << "Decoding QR...\n";

        auto decoded =
            qr::decode(qr_path);

        std::cout
            << "Decoded data size: "
            << decoded.size()
            << " bytes\n";


        // ============================================================
        // 5. Verify binary data
        // ============================================================

        if (original != decoded)
        {
            std::cerr
                << "ERROR: decoded data does not "
                   "match the original PNG.\n";

            return 1;
        }

        std::cout
            << "QR round trip successful.\n";


        // ============================================================
        // 6. Write decoded bytes as decode.png
        // ============================================================

        bool written =
            file_io::write_file(
                decoded_path,
                decoded
            );

        if (!written)
        {
            std::cerr
                << "Failed to write decoded PNG.\n";

            return 1;
        }

        std::cout
            << "Decoded PNG written: "
            << decoded_path
            << '\n';


        // ============================================================
        // 7. Final result
        // ============================================================

        std::cout
            << "\n========================================\n"
            << "       PNG QR ROUND TRIP SUCCESS\n"
            << "========================================\n"
            << "Original : " << input_path << '\n'
            << "QR       : " << qr_path << '\n'
            << "Decoded  : " << decoded_path << '\n'
            << "Size     : " << original.size() << " bytes\n"
            << "========================================\n";

        return 0;
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