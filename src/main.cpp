#include "../include/encoder.hpp"
#include "../include/decoder.hpp"

#include <filesystem>
#include <iostream>
#include <string>

void print_usage(const char* program)
{
    std::cerr
        << "Usage:\n"
        << "  " << program << " -e <file>\n"
        << "  " << program << " -d <directory>\n\n"
        << "Examples:\n"
        << "  " << program << " -e image.png\n"
        << "  " << program << " -d ./qr_codes\n";
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

        if (mode == "-e")
            return encode_file(path);

        if (mode == "-d")
            return decode_files(path);

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