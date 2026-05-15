from .cside cimport (
    _Err, NO_ERR,
    uint8_t, cBytes, uintptr_t,
    bytes_create, bytes_destroy, bytes_fload, bytes_write, bytes_copy,
    bytes_size, bytes_ptr, bytes_finalize, bytes_fdump, bytes_compare,
)
import numpy as np

cdef class pyByteArray:

    def __cinit__(self):
        self.array = NULL

    def __dealloc__(self):
        cdef _Err err = NO_ERR
        if self.array != NULL:
            err = bytes_destroy(&self.array)
        if err != NO_ERR:
            raise Exception('Failed to destroy cBytes')

    def __len__(self):
        cdef long len = bytes_size(self.array)
        return len

    @classmethod
    def create(cls, long size) -> pyByteArray:
        cdef _Err err
        cdef pyByteArray obj = cls()
        obj.array = bytes_create(size)
        if obj.array == NULL:
            raise Exception('Failed to create cBytes object')
        return obj

    @classmethod
    def fload(cls, str fname) -> pyByteArray:
        cdef _Err err
        cdef bytes c_fname = fname.encode('utf-8')
        cdef pyByteArray obj = cls()
        obj.array = bytes_fload(<const char *> c_fname)
        if obj.array == NULL:
            raise Exception('Failed to load file')
        return obj

    @classmethod
    def from_bytes(cls, bytes obj) -> pyByteArray:
        cdef int size = <int> len(obj)
        cdef char* c_string = obj
        cdef pyByteArray res = cls.create(size)
        cdef _Err err
        err = bytes_write(res.array, size, <void *> c_string)
        if err != NO_ERR:
            raise Exception('Failed to write bytes into array.')
        err = bytes_finalize(res.array)
        if err != NO_ERR:
            raise Exception('Failure to finalize bytearray')
        return res

    @classmethod
    def from_str(cls, str txt):
        cdef bytes ctxt = txt.encode('latin-1')
        return cls.from_bytes(ctxt)

    def get_bytes(self) -> bytes:
        cdef long size = bytes_size(self.array)
        cdef uint8_t* ptr = bytes_ptr(self.array)
        return ptr[:size]

    def _test_ptr(self) -> np.uintp:
        cdef uintptr_t cptr = <uintptr_t> bytes_ptr(self.array)
        return cptr

    def fwrite(self, str fname):
        cdef bytes c_fname = fname.encode('utf-8')
        err = bytes_fdump(self.array, c_fname)
        if err != NO_ERR:
            raise Exception('Failed to write to file')

    @classmethod
    def from_pyobj(cls, obj: object):
        cdef _Err err
        cdef pyByteArray newobj
        cdef bytes c
        cdef const char *cp_
        cdef uintptr_t ptr
        if isinstance(obj, np.uintp):
            if obj == 0:
                raise Exception("Got NULL pointer or non-existent bytes object")
            newobj = cls()
            ptr = obj
            newobj.array = bytes_copy(<cBytes> ptr)
            return newobj
        if isinstance(obj, str):
            return cls.from_str(obj)
        if isinstance(obj, bytes):
            return cls.from_bytes(obj)
        if not(isinstance(obj, (list, tuple))):
            raise Exception(f"Cannot create bytes object from type f{type(obj)}")

        newobj = cls.create(len(obj))
        for elem in obj:
            if isinstance(elem, int):
                if elem < 0 or elem >= 256:
                    raise Exception("Integer outside ascii range")
                c = chr(elem).encode('utf-8')
            elif isinstance(elem, str):
                c = elem.encode('utf-8')
            elif isinstance(elem, bytes):
                c = elem
            else:
                raise Exception(f"{type(obj)} contains object of incompatible type")
            cp_ = c
            err = bytes_write(newobj.array, <int> len(c), <void *> cp_)
            if err != NO_ERR:
                raise Exception("Failed to write bytes into array")
        err = bytes_finalize(newobj.array)
        if err != NO_ERR:
            raise Exception("Failed to finalize byte array")
        return newobj

    def __eq__(self, other):
        cdef bytes c
        cdef const char* cp_
        cdef cBytes temp
        cdef int cmp
        cdef pyByteArray testbytes
        if isinstance(other, pyByteArray):
            testbytes = other
        else:
            testbytes = self.__class__.from_pyobj(other)
        cmp = bytes_compare(self.array, testbytes.array)
        if cmp:
            return False
        else:
            return True

    def __str__(self) -> str:
        cdef long size = bytes_size(self.array)
        cdef char* ptr = <char*> bytes_ptr(self.array)
        return ptr[:size].decode('latin-1')
