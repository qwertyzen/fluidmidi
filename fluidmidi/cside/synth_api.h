#ifndef __SYNTH_API_H__
#define __SYNTH_API_H__

#include "cmidi_types.h"
#include "fluidsynth.h"

int fs_send_fluidmidi_now(fluid_synth_t *fs, MidiRow *row, long num_rows);

#endif  // __SYNTH_API_H__
