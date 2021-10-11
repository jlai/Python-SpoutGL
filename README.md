SpoutGL for Python
==================

An unofficial Python wrapper for [Spout](https://leadedge.github.io/) using [pybind11](https://github.com/pybind/pybind11)

Key features compared to some previous wrappers:
- Covers more of the SpoutGL API, including sendImage/receiveImage
- pybind11 instead of Boost Python which should make it easier to build for multiple Python versions
- pip/wheel support for Python 3.7+ on both 64-bit and 32-bit Windows
- No external dependencies

# Extras

## SpoutGL.helpers

Adds several functions that are useful in Blender and other environments

- getBytesPerPixel
- isBufferEmpty
- copyToFloat32
- copyToByteBuffer

## SpoutGL.enums

Exports supported GL format enums as integers:

- GL_RGBA
- GL_BGRA_EXT
- GL_RGB
- GL_BGR_EXT

# Examples

See the examples directory.

# License & Disclaimer

Licensed under the BSD license. See the LICENSE file for details.

This is not an officially supported Google product.
