#include "../include/file_io.hpp"
#include <format>
#include <string>
#include <iostream>

int main()
{
    bool exists = file_io::file_exists("../test.txt");
    //boolalpha to convert 1 to true
    std::cout << std::boolalpha << "File exists: "<< exists << '\n';

    auto data = file_io::read_file("../test.txt");
    //std::hex to convert to Hexadecimal, std::showbase to show 0x before the number
    std::cout << "File binary: \n"<<std::hex;
    //for each byte in data, print the byte as an hexadecimal number
    for (const auto& byte : data)
    {
        std::cout << static_cast<int>(byte) << ' ';
    }
    std::cout << '\n';



    
    // Convert user input into binary bytes
    std::string text;

    std::cout << "Enter text to write to file: ";
    std::getline(std::cin, text);

    std::vector<std::uint8_t> bytes(
        text.begin(),
        text.end()
    );

    // Display the bytes as decimal values
    for (const auto& byte : bytes)
    {
        std::cout << static_cast<int>(byte) << ' ';
    }

    std::cout << '\n';

    // Write the bytes to a file
    bool validate = file_io::write_file(
        "../test_copy.txt",
        bytes
    );

    std::cout << std::boolalpha
            << "File written: "
            << validate
            << '\n';
}