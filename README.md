# fnt2bin
A tool for converting Playdate Arcade Fonts `.fnt` files to 1-bit binary format

## Usage
```
fnt2bin <file_name>.fnt
```

This will generate a 1-bit binary `font.bin` file with font data.

In the future I'll implement a feature to output PNGs for each individual character

## Building
Use the provided scripts, `build-win.ps1` for Windows and `make` for Linux. Or just build using your favorite compiler. This code has no dependencies
