# FQR Chunking System Specification

## 1. Goal

FQR must support files larger than the payload capacity of a single QR code.

The current implementation attempts:

```text
file
  ↓
serialize
  ↓
ONE QR
```

This fails for large files.

The new system must automatically split a file into multiple QR packets:

```text
file
  ↓
read bytes
  ↓
split into chunks
  ↓
create FilePacket for every chunk
  ↓
serialize each packet
  ↓
encode each packet into a QR
```

The receiver performs the reverse:

```text
QRs
  ↓
decode each QR
  ↓
deserialize each packet
  ↓
identify chunk
  ↓
order chunks
  ↓
combine data
  ↓
reconstruct original file
```

---

# 2. Current Problem

A test file produced:

```text
test.png
241092 bytes
```

ZXing rejected the complete file:

```text
Error 777: Input too long
```

This is expected because a single QR code cannot contain the entire file.

Do NOT attempt to solve this by increasing an arbitrary buffer.

The correct solution is multiple QR codes.

---

# 3. Requirements

The chunking system must:

* Automatically detect when a file requires multiple QR codes.
* Split the file into safe-sized chunks.
* Create one `FilePacket` per chunk.
* Store enough metadata to reconstruct the file.
* Serialize every packet.
* Encode every serialized packet into a QR.
* Decode every QR independently.
* Deserialize every packet.
* Detect the chunk number.
* Detect the total number of chunks.
* Reorder chunks if they are received out of order.
* Combine the chunks.
* Reconstruct the original file.
* Verify that the reconstructed size matches the original size.

Do NOT implement hashing yet.

Do NOT implement compression yet.

Do NOT implement encryption yet.

Keep the implementation simple and deterministic.

---

# 4. FilePacket Design

Extend the existing `FilePacket`.

Current conceptual structure:

```cpp
struct FilePacket
{
    std::string filetype;
    std::uint64_t filesize;
    std::vector<std::uint8_t> data;
};
```

Change it to:

```cpp
struct FilePacket
{
    std::uint32_t packet_id;

    std::uint32_t chunk_index;

    std::uint32_t total_chunks;

    std::string filetype;

    std::uint64_t filesize;

    std::vector<std::uint8_t> data;
};
```

## Field meanings

### `packet_id`

Identifies the transfer/file.

All chunks belonging to the same file must have the same `packet_id`.

For now, generate a simple ID automatically.

Do not implement cryptographic IDs.

### `chunk_index`

Identifies the position of this chunk.

The first chunk is:

```text
0
```

The second:

```text
1
```

and so on.

### `total_chunks`

Number of chunks required to reconstruct the complete file.

For example:

```text
total_chunks = 50
```

means valid chunk indexes are:

```text
0 ... 49
```

### `filetype`

Example:

```text
image/png
```

or:

```text
text/plain
```

Keep the existing field.

### `filesize`

Original complete file size.

This must NOT be the size of the individual chunk.

Example:

```text
original file = 241092 bytes
```

Every packet contains:

```text
filesize = 241092
```

### `data`

Contains only the bytes belonging to this chunk.

---

# 5. Serialization Format

The serialized packet must contain fields in a deterministic order.

Use:

```text
packet_id       4 bytes
chunk_index     4 bytes
total_chunks    4 bytes

filetype_length 4 bytes
filetype        N bytes

filesize        8 bytes

chunk_data      N bytes
```

Therefore:

```text
┌────────────────────┐
│ packet_id          │ 4
├────────────────────┤
│ chunk_index        │ 4
├────────────────────┤
│ total_chunks       │ 4
├────────────────────┤
│ filetype_length    │ 4
├────────────────────┤
│ filetype           │ variable
├────────────────────┤
│ filesize           │ 8
├────────────────────┤
│ chunk data         │ variable
└────────────────────┘
```

Use the existing little-endian serialization style.

Do not change the byte ordering convention.

---

# 6. Deserialization

`deserialize()` must reverse the exact serialization process.

It must:

1. Read `packet_id`.
2. Read `chunk_index`.
3. Read `total_chunks`.
4. Read `filetype_length`.
5. Read `filetype`.
6. Read `filesize`.
7. Treat the remaining bytes as chunk data.

The function must validate that enough bytes exist before accessing the vector.

Never blindly access:

```cpp
data[index]
```

without checking the size.

Malformed packets must throw an exception instead of causing undefined behavior.

---

# 7. Chunk Size

Do NOT use the complete QR capacity as the chunk size.

The packet contains metadata, and QR encoding itself has capacity limitations.

Create one configurable constant:

```cpp
constexpr std::size_t QR_CHUNK_SIZE = 2000;
```

Start conservatively.

Do not optimize this value yet.

The goal is reliability first.

The chunk size should be easy to change later.

---

# 8. Chunking Algorithm

Create a chunking function:

```cpp
std::vector<FilePacket> create_packets(
    const std::vector<std::uint8_t>& file_data,
    const std::string& filetype
);
```

The function must:

1. Determine the original file size.
2. Calculate the number of chunks.
3. Generate one `packet_id`.
4. Create packets sequentially.
5. Assign indexes from `0`.
6. Set `total_chunks`.
7. Copy the appropriate range of bytes into each packet.

Example:

```text
file size = 241092
chunk size = 2000

total chunks = 121
```

Packets:

```text
chunk 0   → bytes 0-1999
chunk 1   → bytes 2000-3999
chunk 2   → bytes 4000-5999
...
chunk 120 → remaining bytes
```

The last chunk may be smaller than `QR_CHUNK_SIZE`.

---

# 9. Chunk Reconstruction

Create:

```cpp
std::vector<std::uint8_t> reconstruct_file(
    std::vector<FilePacket> packets
);
```

The function must:

1. Ensure packets are not empty.
2. Verify they belong to the same `packet_id`.
3. Verify they have the same `total_chunks`.
4. Verify they describe the same original file size.
5. Sort packets by `chunk_index`.
6. Verify indexes are sequential.
7. Combine `data`.
8. Verify the final byte count equals `filesize`.
9. Return the reconstructed byte vector.

Example:

Input order may be:

```text
chunk 4
chunk 0
chunk 3
chunk 1
chunk 2
```

The reconstruction function must produce:

```text
chunk 0
chunk 1
chunk 2
chunk 3
chunk 4
```

before combining the data.

---

# 10. Duplicate Chunks

If two packets contain the same:

```text
packet_id
chunk_index
```

treat this as a duplicate.

For the first implementation, reject duplicates with an exception.

Example:

```text
chunk 0
chunk 1
chunk 1
chunk 2
```

must fail.

Do not silently overwrite packets.

---

# 11. Missing Chunks

If:

```text
total_chunks = 5
```

the receiver must receive:

```text
0
1
2
3
4
```

If:

```text
0
1
2
4
```

then chunk `3` is missing.

Reconstruction must fail.

Do not create a corrupted file.

---

# 12. QR Encoding

The QR layer should remain responsible only for:

```text
bytes ↔ QR image
```

It should NOT know about:

* files
* chunks
* FilePacket
* filenames
* reconstruction

Keep this separation:

```text
file_packet
    ↓
serialized bytes
    ↓
qr
```

The QR module should continue providing:

```cpp
void encode(
    const std::vector<std::uint8_t>& data,
    const std::filesystem::path& output
);

std::vector<std::uint8_t> decode(
    const std::filesystem::path& image
);
```

Do not put chunking logic inside `qr.cpp`.

---

# 13. QR Output

The current implementation creates a P5 PGM image.

Keep this working for now.

Do NOT introduce PNG/JPEG image encoding into the QR module yet.

The test files may be PNG, but the generated QR image is currently:

```text
qr_0000.pgm
qr_0001.pgm
qr_0002.pgm
...
```

---

# 14. Encoder Workflow

Create a high-level workflow:

```text
test.png
    ↓
file_io::read_file()
    ↓
create_packets()
    ↓
packet 0
packet 1
packet 2
...
packet N
    ↓
serialize()
    ↓
qr::encode()
    ↓
qr_0000.pgm
qr_0001.pgm
qr_0002.pgm
...
qr_N.pgm
```

The application should automatically create the required number of QR files.

No manual splitting by the user.

---

# 15. Decoder Workflow

The decoder should:

1. Read the QR images.
2. Decode each QR.
3. Deserialize each packet.
4. Store packets.
5. Reconstruct the file.

Conceptually:

```text
qr_0000.pgm
qr_0001.pgm
qr_0002.pgm
...
qr_N.pgm
      ↓
qr::decode()
      ↓
serialized packets
      ↓
deserialize()
      ↓
FilePackets
      ↓
reconstruct_file()
      ↓
decode.png
```

---

# 16. Main Test

Change `main.cpp` to test a real PNG.

Input:

```text
test.png
```

Generated QR files:

```text
qr_0000.pgm
qr_0001.pgm
...
```

Output:

```text
decode.png
```

The program must:

```text
1. Read test.png
2. Split into chunks
3. Print total chunks
4. Serialize every packet
5. Generate QR for every packet
6. Decode every QR
7. Deserialize every packet
8. Reconstruct the original bytes
9. Write decode.png
10. Compare original and decoded bytes
11. Print SUCCESS/FAILURE
```

---

# 17. Expected Output

For example:

```text
Input file: test.png
Original size: 241092 bytes

Chunk size: 2000 bytes
Total chunks: 121

Encoding:

QR 1/121 generated
QR 2/121 generated
QR 3/121 generated
...
QR 121/121 generated

Decoding:

QR 1/121 decoded
QR 2/121 decoded
QR 3/121 decoded
...
QR 121/121 decoded

Reconstructing file...

Reconstructed size: 241092 bytes

Original/reconstructed match: true

Decoded file written: decode.png

========================================
FULL ROUND TRIP SUCCESS
========================================
```

---

# 18. Error Handling

The program must fail safely when:

* A QR file cannot be opened.
* A QR cannot be decoded.
* A packet is malformed.
* A packet has an invalid chunk index.
* Packets belong to different transfers.
* `total_chunks` differs between packets.
* A duplicate chunk exists.
* A chunk is missing.
* The reconstructed size differs from `filesize`.
* A QR packet exceeds the QR encoder capacity.

Errors should be reported clearly.

Example:

```text
ERROR: Missing chunk 17
```

instead of silently producing a broken file.

---

# 19. Project Structure

Organize the project as:

```text
fqr/
├── CMakeLists.txt
├── include/
│   ├── file_io.hpp
│   ├── file_packet.hpp
│   └── qr.hpp
│
├── src/
│   ├── main.cpp
│   ├── file_io.cpp
│   ├── file_packet.cpp
│   └── qr.cpp
│
├── test.png
│
└── build/
```

Keep chunking/reconstruction functionality in `file_packet`.

Do not create unnecessary networking or GUI code.

---

# 20. Implementation Order

Implement in exactly this order.

## Step 1

Modify `FilePacket`:

```cpp
packet_id
chunk_index
total_chunks
filetype
filesize
data
```

## Step 2

Update `serialize()`.

## Step 3

Update `deserialize()`.

## Step 4

Add strict bounds checking to `deserialize()`.

## Step 5

Implement:

```cpp
create_packets()
```

## Step 6

Implement:

```cpp
reconstruct_file()
```

## Step 7

Unit-test packet creation and reconstruction without QR.

Test:

```text
original
→ packets
→ reconstructed
```

## Step 8

Connect packet serialization to `qr::encode()`.

## Step 9

Generate multiple QR images.

## Step 10

Decode multiple QR images.

## Step 11

Reconstruct the file.

## Step 12

Compare original and reconstructed bytes.

Only after all of this should additional features be considered.

---

# 21. Explicitly Do Not Implement Yet

Do NOT implement:

* hashing
* encryption
* compression
* networking
* database storage
* cloud storage
* GUI
* authentication
* QR animation
* error correction at the FQR protocol level
* resume support

Those are future features.

The immediate goal is only:

```text
LARGE FILE
    ↓
CHUNKS
    ↓
MULTIPLE QRs
    ↓
DECODE
    ↓
REASSEMBLE
    ↓
IDENTICAL FILE
```

---

# 22. Definition of Done

The chunking milestone is complete when this command:

```bash
./build/fqr
```

can take:

```text
test.png
```

of at least several hundred KB and produce:

```text
qr_0000.pgm
qr_0001.pgm
...
qr_N.pgm
```

then decode those QR files and produce:

```text
decode.png
```

where:

```cpp
original == decoded
```

is true.

The final output must report:

```text
FULL ROUND TRIP SUCCESS
```

Only then move to the next feature.
 
