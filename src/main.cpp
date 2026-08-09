#include "../include/file_io.hpp"

#include <iostream>

int main()
{
    bool exists = file_io::file_exists("../CMakeLists.txt");

    std::cout << std::boolalpha
              << "File exists: "
              << exists
              << '\n';

    return 0;
}