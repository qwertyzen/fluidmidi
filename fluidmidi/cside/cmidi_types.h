#ifndef __CMIDI_TYPES_H__
#define __CMIDI_TYPES_H__

#include <stdint.h>
#include "cbytearray.h"

typedef enum cMtype_e{
    MTYPE_EMPTY, MTYPE_META, MTYPE_TEMPO, MTYPE_TSIG, MTYPE_TRACK, MTYPE_SYSEX, MTYPE_ESC,
    MTYPE_PC, MTYPE_CC, MTYPE_POLYP, MTYPE_CHANP, MTYPE_CHAN,
    MTYPE_BEND1, MTYPE_NOF, MTYPE_BEND0, MTYPE_NON,
    MTYPE_REST,
    MTYPE_EOT,
    MTYPE_REC
} cMtype;

typedef struct midi_row_s {
    int32_t      time;      // Absolute time in midi ticks
    uint8_t      track;
    uint8_t      channel;
    uint8_t      mtype;     // A musiclab spec to identify message type, used in sorting order
    uint8_t      status;
    uint8_t      program;
    uint8_t      control;
    uint8_t      cc_val;
    uint8_t      noteval;
    uint8_t      notevel;
    uint32_t     midi_d;    // Duration of a note event in ticks (note off time - note on time)
    uint32_t     score_d;   // Duration of note as expressed in a musical score
    int32_t      pitch;     // Bend value
    uint32_t     delta;     // delta time in midi ticks
    cBytes       data;      // Raw midi data
} MidiRow;

void print_row(MidiRow *row);
void ctable_print(MidiRow *table, int num_rows, int print_bytes);
void ctable_compare_print(MidiRow *table1, MidiRow *table2, long num_rows);

#endif  // __CMIDI_TYPES_H__
