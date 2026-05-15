#ifndef __CSIDE_UTILS_H__
#define __CSIDE_UTILS_H__

/* Error code interface */
#define NO_ERR       0
#define ERR         -1

typedef int _Err;

/* Endianness function interface */
#include <stdint.h>

/* Portable byte swap functions (works on all OSes) */
static inline uint16_t swap16(uint16_t x) {
    return (x >> 8) | (x << 8);
}

static inline uint32_t swap32(uint32_t x) {
    return ((x >> 24) & 0x000000FF) |
           ((x >> 8)  & 0x0000FF00) |
           ((x << 8)  & 0x00FF0000) |
           ((x << 24) & 0xFF000000);
}

/* Define standard names */
#define be16toh(x) swap16(x)
#define be32toh(x) swap32(x)
#define htobe16(x) swap16(x)
#define htobe32(x) swap32(x)

/* ANSI Color code strings */
#define ANSIRED     "\033[31;1m"
#define ANSIBLUE    "\033[36;1m"
#define ANSIGREEN   "\033[32;1m"
#define ANSIYELLOW  "\033[33;1m"
#define ANSIMAGENTA "\033[35;1m"
#define ANSIEND     "\033[0m"

#ifdef FLUIDMIDI_DEBUG
#include <stdio.h>
#define LOGERR(...) do {  \
    fprintf(stderr, "LOGERR: %s, line %d, in %s: ", __FILE__, __LINE__, __func__);  \
    fprintf(stderr, __VA_ARGS__);  \
} while (0);

#define LOGDBG(...) do { \
    fprintf(stderr, ANSIYELLOW "DEBUG: %s, line %d, in %s: ", __FILE__, __LINE__, __func__);  \
    fprintf(stderr, __VA_ARGS__);  \
    fprintf(stderr, ANSIEND);  \
} while (0);
#else
#define LOGERR(...)
#define LOGDBG(...)
#endif

#endif  // __CSIDE_UTILS_H__
