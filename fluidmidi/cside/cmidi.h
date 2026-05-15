#ifndef __CMIDI_H__
#define __CMIDI_H__

#include "utils.h"
#include "cbytearray.h"
#include "cmidi_types.h"

_Err cmidifile_read_header_info(cBytes infile, int16_t *format, int16_t *num_tracks, int16_t *tpb);
_Err cmidifile_read_track_header_info(cBytes infile, int32_t *num_bytes);
_Err cmidifile_read_track_messages_decode(cBytes infile, int32_t chunk_size, MidiRow *track, int *pnum_msgs);
_Err cmiditable_copy_all_cbytes(MidiRow* from, MidiRow *to, long num_msgs);
_Err cmiditable_free_all_byte_array(MidiRow *table, long num_msgs);
void cmiditable_time_rel_to_abs(MidiRow *track, long num_msgs);
void cmiditable_note_durations_calculate(MidiRow *track, long num_msgs);
_Err cmiditable_remove_decoded_data(MidiRow *table, long num_msgs);
_Err cmiditable_encode_table_data(MidiRow *table, long num_msgs);
_Err cmidirow_encode(MidiRow *row);
void cmiditable_scale_time_resolution(MidiRow *table, long num_msgs, int from_tpb, int to_tpb);

void cmiditable_time_abs_to_rel(MidiRow *table, long num_msgs);
_Err cmidifile_write_header_info(cBytes outfile, int16_t format, int16_t num_tracks, int16_t tpb);
_Err cmidifile_write_track_chunk(cBytes outfile, MidiRow *track, long num_msgs);

double ctick2second(long tick, int tempo, int tpb);
long   csecond2tick(double sec, int tempo, int tpb);
int    cbpm2tempo(double bpm);
double ctempo2bpm(int tempo);

typedef enum {
    SC_SORT_DEFAULT,
    SC_SORT_TIME_TRACK,
    SC_SORT_TIME_MTYPE
} SortComparator;

_Err cmiditable_sort(MidiRow table[], long num_msgs, SortComparator comp_e);
_Err cmiditable_merge_sorted(MidiRow *final, int num_tables, MidiRow **tbl_ptrs, int *table_lens, SortComparator comp_e);
_Err cmiditable_timecatenate(MidiRow *final, int num_tables, MidiRow **tbl_ptrs, int *table_lens, int32_t *max_times);
_Err cmiditable_sorted_insert(MidiRow *final, MidiRow *target, long target_len, MidiRow *inserts, long inserts_len);

int  cmidirow_equality(MidiRow *a, MidiRow *b);

#endif  // __CMIDI_H__
