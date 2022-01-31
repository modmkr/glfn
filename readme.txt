
A font rasterizer. I used it to create (rasterize) fonts for my engine: https://github.com/modmaker1/TerrainEngine.

It is a GUI tool allowing you to rasterize TrueType fonts installed to your system (Windows only).

TGA, BMP, binary, and text outputs are currently supported.

Usage:
- When you made changes to the typeface, press the Update button (it is in the bottom, so you'll have to scroll the panel), so the changes are applied.
- When you want to export the rasterized font, press Save... It will typically require you to export two files: the font definition (text) file (so that you have the character mapping), and the image. You can use the binary format (see glfn.h for the definition), which incorporates both.

TODO: This probably has to be rewritten to use some 3-rd party font rasterizing library. Now it uses the internal Windows font rendering engine which sometimes gives unexpected artifacts like when you turn on the Antialiasing option.
