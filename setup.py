import os, sys
from setuptools import Extension, find_packages
from Cython.Build import cythonize
from setuptools import setup
import numpy

DEBUG = False

include_dirs = ['fluidmidi/cside']

extra_compile_args = []

if sys.platform == 'win32':
    # MSVC-style flags
    extra_compile_args += ['/W3', '/Od', '/Zi']  # /W3 = warnings, /Od = no optimization
    if DEBUG:
        extra_compile_args += ['/D', 'FLUIDMIDI_DEBUG']
else:
    # GCC/Clang flags for Unix-like systems
    extra_compile_args += ['-Wall', '-Wextra', '-g', '-O0']
    if DEBUG:
        extra_compile_args += ['-DFLUIDMIDI_DEBUG']

ext_bytearray = Extension(
    'fluidmidi.pyside.bytearray',
    sources=[
        'fluidmidi/pyside/bytearray.pyx',
        'fluidmidi/cside/cbytearray.c',
    ],
    include_dirs=include_dirs,
    extra_compile_args=extra_compile_args,
)

ext_midi = Extension(
    'fluidmidi.pyside.midi',
    sources=[
        'fluidmidi/pyside/midi.pyx',
        'fluidmidi/cside/cmidi.c',
        'fluidmidi/cside/cmidi_codec.c',
        'fluidmidi/cside/ctable_printer.c',
        'fluidmidi/cside/cbytearray.c',
        'fluidmidi/cside/cmidiconst.c'
    ],
    include_dirs=[numpy.get_include()] + include_dirs,
    define_macros=[("NPY_NO_DEPRECATED_API", "NPY_1_7_API_VERSION")],
    extra_compile_args=extra_compile_args,
)

setup_kwargs = {
    'name': 'fluidmidi',
    'version': '0.1.0',
    'author': 'Anustuv Pal',
    'author_email': 'anustuv@gmail.com',
    'description': 'A high-level MIDI library using numpy arrays.',
    'ext_modules': cythonize(
        [ext_bytearray, ext_midi],
        compiler_directives={"language_level": "3"},
        ),
    'packages': find_packages(
        where='.'),
    'install_requires': [
        'numpy', 'ipympl', 'matplotlib', 'ipywidgets',
    ],
}

setup(**setup_kwargs)
