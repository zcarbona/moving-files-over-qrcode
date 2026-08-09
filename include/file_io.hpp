#pragma once

#include <cstdint>
#include <vector>
#include <filesystem>
#include <string>


namespace file_io
{

std::vector<std::uint8_t> read_file(const std::filesystem::path& path);

bool write_file(const std::filesystem::path& path, const std::vector<std::uint8_t>& data);

std::uint64_t get_file_size(const std::filesystem::path& path);


bool file_exists(const std::filesystem::path& path);
}