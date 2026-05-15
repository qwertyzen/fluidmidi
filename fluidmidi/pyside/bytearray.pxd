# cython: language_level=3
from .cside cimport cBytes, uintptr_t

cdef class pyByteArray:
    cdef cBytes array
