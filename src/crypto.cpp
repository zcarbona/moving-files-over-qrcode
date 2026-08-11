#include "../include/crypto.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <sodium.h>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

namespace
{

void hide_terminal_echo(bool hide)
{
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);

    if (hStdin == INVALID_HANDLE_VALUE)
        return;

    DWORD mode;

    if (!GetConsoleMode(hStdin, &mode))
        return;

    if (hide)
        mode &= ~ENABLE_ECHO_INPUT;
    else
        mode |= ENABLE_ECHO_INPUT;

    SetConsoleMode(hStdin, mode);
#else
    termios tty;

    if (tcgetattr(STDIN_FILENO, &tty) != 0)
        return;

    if (hide)
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;

    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

std::string read_password_line()
{
    std::string password;
    std::getline(std::cin, password);

    return password;
}

void ensure_sodium_initialized()
{
    static bool initialized = []() -> bool
    {
        if (sodium_init() < 0)
        {
            throw std::runtime_error(
                "Failed to initialize libsodium"
            );
        }

        return true;
    }();

    (void)initialized;
}

} // namespace

namespace crypto
{

std::string ask_password(const char* prompt)
{
    if (!prompt || !prompt[0])
        prompt = "Enter password: ";

    hide_terminal_echo(true);

    std::cout << prompt << std::flush;

    std::string password = read_password_line();

    hide_terminal_echo(false);

    std::cout << '\n';

    return password;
}

std::string hex_encode(const std::vector<std::uint8_t>& data)
{
    static const char* hex_chars = "0123456789abcdef";

    std::string result;
    result.reserve(data.size() * 2);

    for (std::uint8_t byte : data)
    {
        result.push_back(hex_chars[(byte >> 4) & 0x0F]);
        result.push_back(hex_chars[byte & 0x0F]);
    }

    return result;
}

std::vector<std::uint8_t> hex_decode(const std::string& hex)
{
    if (hex.size() % 2 != 0)
    {
        throw std::runtime_error(
            "Invalid hex string: odd length"
        );
    }

    std::vector<std::uint8_t> result;
    result.reserve(hex.size() / 2);

    for (std::size_t i = 0; i < hex.size(); i += 2)
    {
        auto hex_value = [](char c) -> int
        {
            if (c >= '0' && c <= '9')
                return c - '0';

            if (c >= 'a' && c <= 'f')
                return c - 'a' + 10;

            if (c >= 'A' && c <= 'F')
                return c - 'A' + 10;

            return -1;
        };

        const int high = hex_value(hex[i]);
        const int low = hex_value(hex[i + 1]);

        if (high < 0 || low < 0)
        {
            throw std::runtime_error(
                "Invalid hex character in decrypted data: "
                "incorrect password or corrupted data"
            );
        }

        result.push_back(
            static_cast<std::uint8_t>((high << 4) | low)
        );
    }

    return result;
}

std::vector<std::uint8_t> encrypt_chunk(
    const std::string& hex_data,
    const std::string& password,
    std::vector<std::uint8_t>& out_salt,
    std::vector<std::uint8_t>& out_nonce
)
{
    ensure_sodium_initialized();

    out_salt.resize(crypto::SALT_LEN);
    out_nonce.resize(crypto::NONCE_LEN);

    randombytes_buf(
        out_salt.data(),
        out_salt.size()
    );

    randombytes_buf(
        out_nonce.data(),
        out_nonce.size()
    );

    std::vector<std::uint8_t> key(crypto::KEY_LEN);

    // Derive a key from the password using Argon2id.
    if (crypto_pwhash(
            key.data(),
            crypto::KEY_LEN,
            password.data(),
            password.size(),
            out_salt.data(),
            crypto::ARGON2_ITERATIONS,
            static_cast<std::size_t>(
                crypto::ARGON2_MEMORY_KIB
            ) * 1024,
            crypto_pwhash_ALG_ARGON2ID13
        ) != 0)
    {
        sodium_memzero(key.data(), key.size());

        throw std::runtime_error(
            "Key derivation failed: insufficient memory"
        );
    }

    const std::size_t plaintext_len = hex_data.size();

    const std::size_t ciphertext_len =
        plaintext_len + crypto::TAG_LEN;

    std::vector<std::uint8_t> ciphertext(ciphertext_len);

    // Encrypt the hexadecimal representation.
    if (crypto_aead_xchacha20poly1305_ietf_encrypt(
            ciphertext.data(),
            nullptr,
            reinterpret_cast<const std::uint8_t*>(
                hex_data.data()
            ),
            plaintext_len,
            nullptr,
            0,
            nullptr,
            out_nonce.data(),
            key.data()
        ) != 0)
    {
        sodium_memzero(key.data(), key.size());

        throw std::runtime_error(
            "Encryption failed"
        );
    }

    sodium_memzero(key.data(), key.size());

    return ciphertext;
}

std::vector<std::uint8_t> decrypt_chunk(
    const std::vector<std::uint8_t>& ciphertext,
    const std::string& password,
    const std::vector<std::uint8_t>& salt,
    const std::vector<std::uint8_t>& nonce
)
{
    ensure_sodium_initialized();

    if (salt.size() != crypto::SALT_LEN)
    {
        throw std::runtime_error(
            "Decryption failed: invalid salt length"
        );
    }

    if (nonce.size() != crypto::NONCE_LEN)
    {
        throw std::runtime_error(
            "Decryption failed: invalid nonce length"
        );
    }

    if (ciphertext.size() < crypto::TAG_LEN)
    {
        throw std::runtime_error(
            "Error: incorrect password or corrupted QR data."
        );
    }

    std::vector<std::uint8_t> key(crypto::KEY_LEN);

    // Derive the exact same key from the password and salt.
    if (crypto_pwhash(
            key.data(),
            crypto::KEY_LEN,
            password.data(),
            password.size(),
            salt.data(),
            crypto::ARGON2_ITERATIONS,
            static_cast<std::size_t>(
                crypto::ARGON2_MEMORY_KIB
            ) * 1024,
            crypto_pwhash_ALG_ARGON2ID13
        ) != 0)
    {
        sodium_memzero(key.data(), key.size());

        throw std::runtime_error(
            "Error: incorrect password or corrupted QR data."
        );
    }

    const std::size_t plaintext_len =
        ciphertext.size() - crypto::TAG_LEN;

    std::vector<std::uint8_t> plaintext(plaintext_len);

    // Authentication + decryption.
    if (crypto_aead_xchacha20poly1305_ietf_decrypt(
            plaintext.data(),
            nullptr,
            nullptr,
            ciphertext.data(),
            ciphertext.size(),
            nullptr,
            0,
            nonce.data(),
            key.data()
        ) != 0)
    {
        sodium_memzero(key.data(), key.size());

        throw std::runtime_error(
            "Error: incorrect password or corrupted QR data."
        );
    }

    sodium_memzero(key.data(), key.size());

    return plaintext;
}

void generate_random(
    std::vector<std::uint8_t>& out,
    std::size_t len
)
{
    ensure_sodium_initialized();

    out.resize(len);

    randombytes_buf(
        out.data(),
        len
    );
}

void initialize()
{
    ensure_sodium_initialized();
}

} // namespace crypto