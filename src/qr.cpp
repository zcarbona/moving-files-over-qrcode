#include "../include/qr.hpp"

#include <ZXing/BarcodeFormat.h>
#include <ZXing/CreateBarcode.h>
#include <ZXing/ReadBarcode.h>
#include <ZXing/WriteBarcode.h>

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace qr
{

void encode(
    const std::vector<std::uint8_t>& data,
    const std::filesystem::path& output
)
{
    ZXing::CreatorOptions options(
        ZXing::BarcodeFormat::QRCode
    );

    auto barcode =
        ZXing::CreateBarcodeFromBytes(
            data.data(),
            static_cast<int>(data.size()),
            options
        );

    if (!barcode.isValid())
    {
        throw std::runtime_error(
            "Failed to create QR barcode: " +
            barcode.error().msg()
        );
    }

    auto image =
        ZXing::WriteBarcodeToImage(barcode);

    if (image.format() != ZXing::ImageFormat::Lum)
    {
        throw std::runtime_error(
            "Unexpected image format"
        );
    }

    std::ofstream out(
        output,
        std::ios::binary
    );

    if (!out)
    {
        throw std::runtime_error(
            "Cannot open output file: " +
            output.string()
        );
    }

    // Write PGM header
    out << "P5\n"
        << image.width()
        << " "
        << image.height()
        << "\n255\n";

    const std::size_t pixelCount =
        static_cast<std::size_t>(image.width()) *
        static_cast<std::size_t>(image.height());

    out.write(
        reinterpret_cast<const char*>(image.data()),
        static_cast<std::streamsize>(pixelCount)
    );

    if (!out)
    {
        throw std::runtime_error(
            "Failed to write QR image"
        );
    }
}


std::vector<std::uint8_t> decode(
    const std::filesystem::path& image_path
)
{
    std::ifstream file(
        image_path,
        std::ios::binary
    );

    if (!file)
    {
        throw std::runtime_error(
            "Cannot open QR image: " +
            image_path.string()
        );
    }

    // ------------------------------------------------------------
    // Read PGM header
    // ------------------------------------------------------------

    std::string magic;

    file >> magic;

    if (magic != "P5")
    {
        throw std::runtime_error(
            "Unsupported image format. Expected P5 PGM."
        );
    }

    int width;
    int height;
    int max_value;

    file >> width;
    file >> height;
    file >> max_value;

    if (!file)
    {
        throw std::runtime_error(
            "Invalid PGM header."
        );
    }

    if (width <= 0 || height <= 0)
    {
        throw std::runtime_error(
            "Invalid image dimensions."
        );
    }

    if (max_value != 255)
    {
        throw std::runtime_error(
            "Unsupported PGM pixel depth."
        );
    }

    // Consume the whitespace after the header.
    file.get();

    // ------------------------------------------------------------
    // Read pixel data
    // ------------------------------------------------------------

    const std::size_t pixel_count =
        static_cast<std::size_t>(width) *
        static_cast<std::size_t>(height);

    std::vector<std::uint8_t> pixels(pixel_count);

    file.read(
        reinterpret_cast<char*>(pixels.data()),
        static_cast<std::streamsize>(pixel_count)
    );

    if (file.gcount() !=
        static_cast<std::streamsize>(pixel_count))
    {
        throw std::runtime_error(
            "Failed to read complete PGM image."
        );
    }

    // ------------------------------------------------------------
    // Give image to ZXing
    // ------------------------------------------------------------

    ZXing::ImageView image_view(
        pixels.data(),
        width,
        height,
        ZXing::ImageFormat::Lum
    );

    ZXing::ReaderOptions options;

    auto barcode =
        ZXing::ReadBarcode(
            image_view,
            options
        );

    if (!barcode.isValid())
    {
        throw std::runtime_error(
            "Failed to decode QR code."
        );
    }

    // ------------------------------------------------------------
    // Return decoded binary data
    // ------------------------------------------------------------

    const auto& bytes = barcode.bytes();

    return std::vector<std::uint8_t>(
        bytes.begin(),
        bytes.end()
    );
}

} // namespace qr