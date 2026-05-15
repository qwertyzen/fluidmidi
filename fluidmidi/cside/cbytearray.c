#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cbytearray.h"

struct cbytes_s {
    uint8_t *bytes;
    long size;
    long pos;
};

struct cbytes_s* bytes_create(long size)
{
    struct cbytes_s *arr = (struct cbytes_s *) calloc(1, sizeof(struct cbytes_s));
    if (!arr) {
        goto fn_exit;
    }
    arr->bytes = (uint8_t *) malloc (size);
    if (!arr->bytes) {
        arr = NULL;
        goto fn_exit;
    }
    arr->size = size;
    arr->pos = 0;
fn_exit:
    return arr;
}

struct cbytes_s* bytes_copy(struct cbytes_s *orig)
{
    long i;
    if (!orig) {
        return NULL;
    }
    struct cbytes_s *arr = bytes_create(orig->size);
    if (!arr) {
        return NULL;
    }
    arr->pos = orig->pos;
    for (i = 0; i < arr->size; i++) {
        arr->bytes[i] = orig->bytes[i];
    }
    return arr;
}

_Err bytes_destroy(struct cbytes_s **pobj)
{
    struct cbytes_s *obj = *pobj;
    if (!obj) {
        return ERR;
    }
    free(obj->bytes);
    free(obj);
    *pobj = NULL;
    return NO_ERR;
}

_Err bytes_reallocn(struct cbytes_s *array, long size)
{
    if (!array) {
        return ERR;
    }
    array->size = size;
    array->bytes = (uint8_t *) realloc(array->bytes, size);
    if (!array->bytes) {
        bytes_destroy(&array);
        return ERR;
    }
    return NO_ERR;
}

_Err bytes_realloc(struct cbytes_s *obj)
{
    return bytes_reallocn(obj, obj->size * 2);
}

struct cbytes_s* bytes_fload(const char* fname)
{
    FILE *file;
    long size;
    struct cbytes_s *bfile=NULL;
    file = fopen(fname, "rb");
    if (!file) {
        goto fn_exit;
    }
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    bfile = bytes_create(size);
    if (!bfile) {
        goto fn_exit;
    }

    size_t retval = fread(bfile->bytes, 1, size, file);
    if ((long) retval != size) {
        bfile = NULL;
    }
    fclose(file);
fn_exit:
    return bfile;
}

_Err bytes_fdump(struct cbytes_s *array, const char* fname)
{
    if (!array) {
        goto fn_fail;
    }
    FILE *file = fopen(fname, "wb");
    if (!file) {
        goto fn_fail;
    }
    fwrite(array->bytes, 1, array->size, file);
    fclose(file);
    return NO_ERR;
fn_fail:
    return ERR;
}

long bytes_tell(struct cbytes_s *array)
{
    if (!array) {
        return ERR;
    }
    return array->pos;
}

uint8_t* bytes_ptr(struct cbytes_s *array)
{
    if (!array) {
        return NULL;
    }
    return array->bytes;
}

long bytes_size(struct cbytes_s *array)
{
    if (!array) {
        return ERR;
    }
    return array->size;
}

_Err bytes_seek(struct cbytes_s *array, long offset, long whence)
{
    _Err _errno;
    if (!array) {
        _errno = ERR;
        goto fn_exit;
    }
    long start, final;
    switch (whence)
    {
    case SEEK_SET:
        start = 0;
        break;
    case SEEK_END:
        start = array->size;
        break;
    case SEEK_CUR:
        start = array->pos;
        break;
    default:
        _errno = ERR;
        goto fn_exit;
    }
    final = start + offset;
    if (final >= 0 && final < array->size) {
        array->pos = final;
        _errno = NO_ERR;
        goto fn_exit;
    }
    _errno = ERR;
fn_exit:
    return _errno;
}

_Err bytes_read(struct cbytes_s *array, int num_bytes, void *tobuf)
{
    if (!array) {
        goto fn_err;
    }
    if (array->pos + num_bytes > array->size) {
        goto fn_err;
    }
    memcpy(tobuf, array->bytes + array->pos, num_bytes);
    array->pos += num_bytes;

    return NO_ERR;
fn_err:
    return ERR;
}

_Err bytes_write(struct cbytes_s *array, int num_bytes, void * frombuf)
{
    _Err _errno = NO_ERR;
    if (!array) {
        _errno = ERR;
        goto fn_exit;
    }
    while (array->pos + num_bytes > array->size) {
        _errno = bytes_realloc(array);
        if (_errno != NO_ERR) {
            goto fn_exit;
        }
    }
    memcpy(array->bytes + array->pos, frombuf, num_bytes);
    array->pos += num_bytes;
fn_exit:
    return _errno;
}

_Err bytes_finalize(struct cbytes_s *array)
{
    if (!array) {
        return ERR;
    }
    array->bytes = (uint8_t *) realloc(array->bytes, array->pos);
    if (!array->bytes) {
        return ERR;
    }
    array->size = array->pos;
    array->pos = 0;
    return NO_ERR;
}

int32_t bytes_read_int32(struct cbytes_s *array)
{
    int32_t num;
    _Err _errno = bytes_read(array, 4, &num);
    if (_errno != NO_ERR) {
        return _errno;
    }
    num = be32toh(num);
    return num;
}

int16_t bytes_read_int16(struct cbytes_s *array)
{
    int16_t num;
    _Err _errno = bytes_read(array, 2, &num);
    if (_errno != NO_ERR) {
        return _errno;
    }
    num = be16toh(num);
    return num;
}

int bytes_read_varint(struct cbytes_s *array)
{
    int value = 0;
    uint8_t byte;
    _Err _errno = NO_ERR;
    do {
        _errno = bytes_read(array, 1, &byte);
        if (_errno != NO_ERR) {
            return _errno;
        }
        value = (value << 7) | (byte & 0x7f);
    } while (byte & 0x80);
    return value;
}

_Err bytes_write_varint(struct cbytes_s *array, unsigned int value)
{
    _Err _errno = NO_ERR;
    int i;
    uint8_t byte;
    struct cbytes_s* temp = bytes_create(4);
    byte = value & 0x7f;
    value >>= 7;
    _errno = bytes_write(temp, 1, &byte);
    if (_errno != NO_ERR) {
        goto fn_exit;
    }
    while (value) {
        byte = (value & 0x7f) | 0x80;
        _errno = bytes_write(temp, 1, &byte);
        if (_errno != NO_ERR) {
            goto fn_exit;
        }
        value >>= 7;
    }
    for (i = temp->pos - 1; i >= 0; i--) {
        _errno = bytes_write(array, 1, temp->bytes+i);
        if (_errno != NO_ERR) {
            goto fn_exit;
        }
    }
    bytes_destroy(&temp);
fn_exit:
    return _errno;
}

_Err bytes_write_int32(struct cbytes_s *array, int32_t value)
{
    value = htobe32(value);
    return bytes_write(array, 4, &value);
}

_Err bytes_write_int16(struct cbytes_s *array, int16_t value)
{
    value = htobe16(value);
    return bytes_write(array, 2, &value);
}

void bytes_print(struct cbytes_s *array)
{
    if (!array) {
        printf("NULL");
        return;
    }
    int i;
    for (i = 0; i < array->size; i++) {
        printf("%02X ", array->bytes[i]);
    }
}

void bytes_print_txt(struct cbytes_s *array)
{
    if (!array) {
        printf("<NULL>");
        return;
    }
    long i;
    for (i = 0; i < array->size; i++) {
        printf("%c", array->bytes[i]);
    }
}

int bytes_compare(struct cbytes_s *array, struct cbytes_s *cmp_to)
{
    uint8_t b1, b2;
    if (array == cmp_to) {
        return 1;
    }
    long size = bytes_size(array);
    if (size != bytes_size(cmp_to)) {
        return 2;
    }
    long i;
    for (i = 0; i < size; i++) {
        b1 = bytes_ptr(array)[i];
        b2 = bytes_ptr(cmp_to)[i];
        if (b1 < b2) {
            return -1;
        }
        else if (b1 > b2) {
            return 1;
        }
    }
    return 0;
}
