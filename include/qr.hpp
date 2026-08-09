#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

namespace qr
{

void encode(
    const std::vector<std::uint8_t>& data,
    const std::filesystem::path& output
);

std::vector<std::uint8_t> decode(
    const std::filesystem::path& image
);

} // namespace qr
