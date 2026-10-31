# Lossless native artwork streams

Each `.zlib` file contains **all source-derived RGB565 pixels**, in row-major
little-endian order, compressed with Zopfli 0.4.3 using 15 optimization iterations.
These are standard zlib-wrapped DEFLATE streams. The `.png` source images remain
untouched in `../board08/` and the repository concept-art directory.
The exact source crops and runtime resizes are documented in
[../board08/README.md](../board08/README.md).

`manifest.json` records dimensions, native-pixel SHA-256, compressed-stream
SHA-256, and compressed length for every runtime asset. Ordinary generation and
`--check` reconstruct the native pixels from the verified sources and compare
them byte for byte with the decompressed cache, checking stream completion,
exact output length, and absence of trailing data. They need only Pillow and
Python's standard zlib module, not Zopfli or an SVG rasterizer.

To regenerate compression after changing source pixels:

```sh
python3 -m pip install zopfli==0.4.3
python3 games/living-kingdom/tools/kingdom_art.py --recompress
```

Compression is an explicit offline step; normal builds use the verified cache.
Generated C++ data is split into individual `.inc` files, each checked to remain
below 500,000 bytes. No palette reduction is applied.

## Native memory and decoding

The current exact payload is reported by `kingdom_art.py --check` and the
per-asset manifest. It is approximately 1.07 MB of image streams, plus 18,432
bytes for four uncompressed 48×48 blank panel textures and 1,296 bytes for the
launcher in the shell's required indexed format. Metadata/pointer tables add a
small amount. Scenes/map/title are stored at half their physical dimensions
and enlarged exactly 2×; portraits/icons are stored at their physical size.

Firmware uses the ESP32 ROM `tinfl_decompress` implementation, with a shared
32 KiB dictionary, its bounded decoder state, and one 960-byte RGB565 row.
Host builds use system zlib and link with `-lz`. Both adapters accept only
bounded dimensions/input, reject excess output and incomplete/trailing streams,
and reconstruct the same rows before ordinary clipped/scaled blitting. No
full-image decompression scratch allocation or handwritten DEFLATE codec is
used. The renderer is serial, so the shared dictionary is reused between draws.
