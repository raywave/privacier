# privacier - cs:go kernel mode cheat

Simple CS:GO Cheat based on IOCTL driver with pretty simple features to practice in development.

## Goals

- [x] Using [lazy_importer](https://github.com/JustasMasiulis/lazy_importer) to get basic protection against reverse-engineers.
- [x] Using [skCrypt](https://github.com/skadro-official/skCrypter) to encrypt strings in binary file.
- [x] Using kernel-mode to make cheat much undetectable.

## Usage

1. Load driver (using test-signing or other ways)
2. Launch CS:GO
3. Launch executable file of cheat

## Keybinds

- `F1` - Glow ESP
- `F2` - Radar hack
- `F3` - Bunnyhop
- `End` - Close cheat

## Compiling

1. Install [vcpkg](https://vcpkg.io/en/getting-started.html)
2. Type `vcpkg install fmt:x64-windows` in `cmd.exe`
3. Type `vcpkg integrate install` in `cmd.exe`
4. Open project in Visual Studio

## Updating offsets

1. Download latest version of [haze-dumper](https://github.com/frk1/hazedumper-rs/releases/)
2. Unpack it to haze-dumper directory (without config.json, it's already made)
3. Launch CS:GO
4. Launch haze-dumper.exe
5. Copy everything from `csgo.hpp` to `offsets.hpp`

## Used libaries

- [discord-rpc](https://github.com/discord/discord-rpc) - basic usage of discord rpc.
- [fmt](https://github.com/fmtlib/fmt) - fast & fancy console output.
- [skCrypt](https://github.com/skadro-official/skCrypter) - string protection against reverse-engineers.
- [lazy-importer](https://github.com/JustasMasiulis/lazy_importer) - import protection against reverse-engineers.
