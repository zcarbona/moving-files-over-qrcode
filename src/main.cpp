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
        const std::string filetype = path.extension().string();
        std::string extention;

        if (mode == "-e")
            return encode_file(path, filetype);

        if (mode == "-d")
        {
            std::cout<<"please enter file extention:\n";
            std::cin>>extention;
            return decode_files(path,extention);
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