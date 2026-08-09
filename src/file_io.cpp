#include "file_io.hpp"

#include <fstream>
#include <stdexcept>
#include <vector>

namespace fqr::file {

bool file_exists(const std::filesystem::path& path)
{
    try
    {
        return std::filesystem::exists(path);
    }
    catch (const std::filesystem::filesystem_error&)
    {
        return false;
    }
}

std::vector<std::uint8_t> read_file(
    const std::filesystem::path& path
)
{
    std::ifstream file(path, std::ios::binary);

    if (!file)
    {
        throw std::runtime_error(
            "Failed to open file for reading: " + path.string()
        );
    }

    file.seekg(0, std::ios::end);

    const auto size = file.tellg();

    if (size < 0)
    {
        throw std::runtime_error(
            "Failed to determine file size: " + path.string()
        );
    }

    file.seekg(0, std::ios::beg);

    std::vector<std::uint8_t> data(
        static_cast<std::size_t>(size)
    );

    file.read(
        reinterpret_cast<char*>(data.data()),
        static_cast<std::streamsize>(size)
    );

    if (!file)
    {
        throw std::runtime_error(
            "Failed to read file: " + path.string()
        );
    }

    return data;
}


bool write_file(const std::filesystem::path& path,const std::vector<std::uint8_t>& data){
    
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error(
            "Failed to open file for writing: " + path.string()
        );
    }

    file.write(
        reinterpret_cast<const char*>(data.data()),
        static_cast<std::streamsize>(data.size())
    );

    if(!file) {
        throw std::runtime_error(
            "Failed to write file: " + path.string()
        );
    }

    return true;
}

std::uint64_t get_file_size(const std::filesystem::path& path){
    try{
        return std::filesystem::file_size(path);
    }
    catch(const std::filesystem::filesystem_error& e){
        throw std::runtime_error(
            "Failed to get file size: " + path.string() + " Error: " + e.what()
        );
    }
}

}