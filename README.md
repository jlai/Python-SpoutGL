SpoutGL for Python
==================

An unofficial Python wrapper for [Spout](https://leadedge.github.io/) using [pybind11](https://github.com/pybind/pybind11)

Key features compared to some previous wrappers:
- Covers more of the SpoutGL API, including sendImage/receiveImage and setFrameSync/waitFrameSync
- pybind11 instead of Boost Python which should make it easier to build for multiple Python versions
- pip/wheel support for Python 3.7+ on both 64-bit and 32-bit Windows
- No external dependencies

# Installation

`pip install SpoutGL`

# Documentation

For now, refer to the [Spout SDK C++ documentation](https://spoutgl-site.netlify.app/#CClass:Spout)
and the examples in the [examples](https://github.com/jlai/Python-SpoutGL/tree/main/examples/) directory.

# Extras

These helper utilities are not part of the Spout SDK but added for convenience and performance.

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

# License & Disclaimer

Licensed under the BSD license. See the LICENSE file for details.

This is not an officially supported Google product.
