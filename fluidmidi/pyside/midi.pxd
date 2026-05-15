# cython: language_level=3
cimport numpy as np
from .cside cimport MidiRow

cdef MidiRow* ndarray_ptr(np.ndarray table)
cdef np.ndarray create_empty_array(long num_rows)

cdef class FluidMidi:
    cdef np.ndarray arr
