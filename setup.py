from setuptools import setup
from distutils.command.build import build as BuildCommand

# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from pybind11 import get_cmake_dir
import pathlib
import shutil
import os.path

import sys

__version__ = "0.0.4"

is_64bits = sys.maxsize > 2**32

spout_lib_base_dir = "vendor/SpoutGL/Binaries"
spout_lib_dir = os.path.join(
    spout_lib_base_dir, "x64" if is_64bits else "Win32")

ext_modules = [
    Pybind11Extension("SpoutGL._spoutgl",
                      # Note:
                      #   Sort input source files if you glob sources to ensure bit-for-bit
                      #   reproducible builds (https://github.com/pybind/python_example/pull/53)
                      ["src/PySpoutGL.cpp"],

                      # Pass in the version to the compiled code
                      define_macros=[('VERSION_INFO', __version__)],

                      cxx_std=17,
                      include_dirs=['vendor/SpoutGL'],
                      libraries=['spout'],
                      library_dirs=[spout_lib_dir],

                      # Generate VS pdb symbols for debugging
                      # extra_link_args=["/DEBUG:FULL"]
                      ),
]

HERE = pathlib.Path(__file__).parent
README = (HERE / "README.md").read_text()


class CopyDllsAndBuildCommand(BuildCommand):
    """Copies arch-specific DLLs to the SpoutGL package directory"""

    def run(self):
        shutil.copyfile(os.path.join(spout_lib_dir, "Spout.dll"),
                        "src/SpoutGL/Spout.dll")
        super().run()


setup(
    name="SpoutGL",
    version=__version__,
    author="Jason Lai",
    license="BSD-3-Clause",
    url="https://github.com/jlai/Python-SpoutGL",
    description="Wrapper around Spout frame streaming library for Windows",
    long_description=README,
    long_description_content_type="text/markdown",

    classifiers=[
        "Development Status :: 3 - Alpha",
        "Operating System :: Microsoft :: Windows",
        "License :: OSI Approved :: BSD License",
        "Topic :: Multimedia :: Graphics :: Capture",
        "Topic :: Multimedia :: Graphics :: Viewers"
    ],

    ext_modules=ext_modules,
    extras_require={"test": []},
    cmdclass={
        "build": CopyDllsAndBuildCommand
    },
    zip_safe=False,
    python_requires=">3.7",

    packages=["SpoutGL"],
    package_dir={"": "src"},
    package_data={
        "": ["*.dll"]
    },
    include_package_data=True
)
