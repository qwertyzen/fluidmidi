#ifndef __CBYTEARRAY_H__
#define __CBYTEARRAY_H__

#include <stdint.h>
#include "utils.h"

typedef struct cbytes_s *cBytes;

cBytes    bytes_create(long size);
cBytes    bytes_copy(cBytes orig);
_Err      bytes_destroy(cBytes *pobj);
_Err      bytes_realloc(cBytes obj);
_Err      bytes_reallocn(cBytes array, long size);

long      bytes_tell(cBytes array);
uint8_t*  bytes_ptr(cBytes array);
long      bytes_size(cBytes array);
_Err      bytes_seek(cBytes array, long offset, long whence);
void      bytes_print(cBytes array);
void      bytes_print_txt(struct cbytes_s *array);

cBytes    bytes_fload(const char *fname);
_Err      bytes_read(cBytes array, int num_bytes, void *tobuf);
int32_t   bytes_read_int32(cBytes array);
int16_t   bytes_read_int16(cBytes array);
int       bytes_read_varint(cBytes array);

_Err      bytes_write(cBytes array, int num_bytes, void *frombuf);
_Err      bytes_write_varint(cBytes array, unsigned int value);
_Err      bytes_write_int32(cBytes array, int32_t value);
_Err      bytes_write_int16(cBytes array, int16_t value);
_Err      bytes_finalize(cBytes array);
_Err      bytes_fdump(cBytes array, const char *fname);
int       bytes_compare(struct cbytes_s *array, struct cbytes_s *cmp_to);

#endif  // __CBYTEARRAY_H__
