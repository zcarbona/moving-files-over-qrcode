#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace crypto
{

constexpr std::size_t ARGON2_MEMORY_KIB = 65536;
constexpr std::uint32_t ARGON2_ITERATIONS = 3;
constexpr std::size_t KEY_LEN = 32;
constexpr std::size_t SALT_LEN = 16;
constexpr std::size_t NONCE_LEN = 24;
constexpr std::size_t TAG_LEN = 16;

void initialize();

std::string ask_password(const char* prompt);

std::string hex_encode(const std::vector<std::uint8_t>& data);
std::vector<std::uint8_t> hex_decode(const std::string& hex);

std::vector<std::uint8_t> encrypt_chunk(
    const std::string& hex_data,
    const std::string& password,
    std::vector<std::uint8_t>& out_salt,
    std::vector<std::uint8_t>& out_nonce
);

std::vector<std::uint8_t> decrypt_chunk(
    const std::vector<std::uint8_t>& ciphertext,
    const std::string& password,
    const std::vector<std::uint8_t>& salt,
    const std::vector<std::uint8_t>& nonce
);

void generate_random(std::vector<std::uint8_t>& out, std::size_t len);

}
