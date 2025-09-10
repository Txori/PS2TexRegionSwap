# PS2TexRegionSwap

**PS2TexRegionSwap** is a simple tool to convert PlayStation 2 texture packs between **PAL** and **NTSC** versions of a game.

Currently, it has been tested with *Shadow of the Colossus* (PAL: `SCES-53326` → NTSC: `SCUS-97472`), allowing [Sad Origami’s remastered textures](https://ko-fi.com/sadorigami) (PAL-only) to work with the NTSC version.

---

## Introduction

Sad Origami has been doing incredible work remastering the textures of *Shadow of the Colossus*, but his pack only supports the PAL version of the game.

**PS2TexRegionSwap** automates the process of mapping, renaming, and copying textures so they work on the NTSC release.

The tool is generic by design, and support for additional PS2 games will be added in the future.

---

## Requirements

- [nlohmann/json](https://github.com/nlohmann/json) (`include/nlohmann/json.hpp`)
- A C++17 compiler (e.g. [w64devkit](https://github.com/skeeto/w64devkit))

---

## Compilation

To build from source, run:
```g++ -O3 -std=c++17 -Iinclude -o PS2TexRegionSwap.exe PS2TexRegionSwap.cpp```

---

## Usage

1. Follow the texture pack author’s instructions to install the PAL texture pack into the `PCSX2/textures` folder.
2. Unzip the contents of **PS2TexRegionSwap** into the same `PCSX2/textures` folder.
3. Run `PS2TexRegionSwap.exe`.
4. Select the PAL language version you want to use in the NTSC game.
5. Wait for the process to complete.

You can now play the NTSC version with the PAL textures applied.

---

## Future Plans

- Add support for more PS2 games.
- Support conversion **in both directions** (PAL ↔ NTSC).
- Improve JSON mapping system for easier community contributions.
