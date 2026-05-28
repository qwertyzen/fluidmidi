#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "synth_api.h"
#include "cmidi_codec.h"
#include "cmidiconst.h"
#include "cmidi_types.h"
#include "fluidsynth.h"
#include "cfsynth.h"

int fs_send_fluidmidi_now(fluid_synth_t *fs, MidiRow *table, long num_rows)
{
    long i;
    MidiRow *row;
    const unsigned char *data;
    for (i = 0; i < num_rows; i++) {
        row = &table[i];
        if (is_channel_status(row->status)) {
            data = bytes_ptr(row->data);
            fs_send_channel_message(fs, data, bytes_size(row->data));
        }
    }
    return FLUID_OK;
}
