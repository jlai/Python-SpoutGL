SpoutGL
==============

A Python wrapper for [Spout](https://leadedge.github.io/) using [pybind11](https://github.com/pybind/pybind11)

Very much a work in progress, particular the packaging/installation.

Features compared to [Spout for Python](https://github.com/maybites/Spout-for-Python):
- Pybind11 instead of Boost which should hopefully make it easier to build for multiple Python versions
- Support for sendImage and receiveImage
- Covers more of the SpoutGL API
