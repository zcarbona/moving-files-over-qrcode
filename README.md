# FQR — File Transfer Over QR Codes

FQR is a C++20 application for transferring files through QR codes.

The application divides file data into packets, optionally encrypts each packet with a password, encodes the packets into QR codes, and can decode those QR codes to reconstruct the original file.

## Features

* Encode files into QR codes.
* Decode QR codes back into files.
* Binary-safe file handling.
* File packetization and reconstruction.
* Optional password-based encryption.
* Each packet can be encrypted independently before QR encoding.
* Password is entered directly in the terminal.
* Password input is hidden while typing.
* Password can be left empty by simply pressing **Enter**.
* Empty password means encryption uses the default empty string (`""`).
* QR encoding and decoding using ZXing-C++.
* C++20 implementation.
* CMake-based build system.
* Native Windows build using Microsoft Visual C++.
* Linux/macOS support through `pkg-config`.
* Automatic ZXing-C++ acquisition on Windows through CMake `FetchContent`.

---

# Running FQR

FQR uses command-line arguments to select between encoding and decoding.

The general syntax is:

```text
fqr -e <file>
fqr -d <directory> <extension>
```

The first argument must be either:

* `-e` — encode a file
* `-d` — decode QR codes

---

## Encode

To encode a file:

### Linux / macOS

```bash
./build/fqr -e "path/to/file"
```

Example:

```bash
./build/fqr -e src/test.png
```

FQR will ask for a password:

```text
Enter password:
```

The password is hidden while typing.

### Password-protected encoding

Enter a password and press **Enter**:

```text
Enter password:
```

The entered password is used to encrypt the file chunks before they are converted into QR codes.

### Encoding without a password

If you do not want to use a password, simply leave the password empty and press **Enter**:

```text
Enter password:
```

This uses the default password value:

```text
""
```

No additional command-line argument is required.

### Windows

```bat
build\fqr.exe -e "path\to\file"
```

Example:

```bat
build\fqr.exe -e "C:\Users\Ali\Pictures\test.png"
```

The file extension is automatically detected from the input file.

For example:

```bash
./build/fqr -e image.png
```

automatically uses:

```text
.png
```

as the file type.

---

## Decode

To decode QR codes:

### Linux / macOS

```bash
./build/fqr -d "path/to/qr_codes" ".extension"
```

Example:

```bash
./build/fqr -d "./qr_codes" ".png"
```

FQR will ask for the password used during encoding:

```text
Enter password:
```

If the QR data was encrypted, enter the same password used when encoding.

If no password was used during encoding, simply press **Enter** to use the default empty password:

```text
""
```

### Windows

```bat
build\fqr.exe -d "path\to\qr_codes" ".extension"
```

Example:

```bat
build\fqr.exe -d ".\qr_codes" ".png"
```

The extension argument tells FQR what extension to use for the reconstructed file.

For example:

```bash
./build/fqr -d ./qr_codes .png
```

produces a reconstructed file using:

```text
.png
```

---

# Password Behavior

FQR does not require a password.

When encoding or decoding, the program asks for a password directly in the terminal.

```text
Enter password:
```

### With a password

```text
Enter password: my-secret-password
```

The password is used for encryption/decryption.

### Without a password

Simply press **Enter** without typing anything:

```text
Enter password:
```

The password is then:

```text
""
```

This is the default value.

When decoding an encrypted file, the same password used during encoding must be provided. An incorrect password or corrupted QR data will cause decryption to fail.

---

# Encryption Pipeline

When encryption is enabled, file data follows this general pipeline:

```text
Raw File
   │
   ▼
File Chunks
   │
   ▼
Hex Encoding
   │
   ▼
Password-Based Encryption
   │
   ▼
QR Data
   │
   ▼
QR Code
```

During decoding, the process is reversed:

```text
QR Code
   │
   ▼
QR Data
   │
   ▼
Password-Based Decryption
   │
   ▼
Hex Decoding
   │
   ▼
File Chunks
   │
   ▼
Original File
```

Each chunk is encrypted independently using the same user-provided password.

If no password is provided, the password value is simply the empty string:

```text
""
```

---

# Command-Line Arguments

| Command | Arguments                 | Description                                               |
| ------- | ------------------------- | --------------------------------------------------------- |
| `-e`    | `<file>`                  | Encode the specified file into QR codes                   |
| `-d`    | `<directory> <extension>` | Decode QR codes from a directory and reconstruct the file |

### Examples

Encode with password:

```bash
./build/fqr -e image.png
```

Encode without password:

```text
Enter password:
```

Press **Enter**.

Decode with password:

```bash
./build/fqr -d ./qr_codes .png
```

Decode without password:

```text
Enter password:
```

Press **Enter**.

Windows:

```bat
build\fqr.exe -e image.png
build\fqr.exe -d .\qr_codes .png
```

---

# Current Status

FQR is currently under active development.

The project currently contains:

* File I/O
* File packetization
* QR handling
* Encoder
* Decoder
* Optional password-based encryption
* Password input through the terminal
* Empty-password support
* CMake build system
* Native Windows/MSVC build
* Linux/macOS dependency detection
* Automatic ZXing-C++ acquisition on Windows
