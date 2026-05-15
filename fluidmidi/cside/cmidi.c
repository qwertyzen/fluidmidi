#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "cbytearray.h"
#include "cmidiconst.h"
#include "cmidi_types.h"
#include "cmidi_codec.h"
#include "cmidi.h"

_Err cmidifile_read_header_info(cBytes infile, int16_t *format, int16_t *num_tracks, int16_t *tpb)
{
    char header[5] = "\0\0\0\0\0";
    _Err _errno = bytes_read(infile, 4, header);
    if (_errno != NO_ERR) {
        goto fn_fail;
    }
    if (strcmp(header, _FILE_HEADER)) {
        // LOGERR("Not a midi file.\n");
        goto fn_fail;
    }
    int32_t chunk_size = bytes_read_int32(infile);
    if (chunk_size != 6) {
        // LOGERR("Bad midi file encoding.\n");
        goto fn_fail;
    }
    *format = bytes_read_int16(infile);
    *num_tracks = bytes_read_int16(infile);
    *tpb = bytes_read_int16(infile);  // todo: Implement tpb or smpte units
    return NO_ERR;
fn_fail:
    return ERR;
}

_Err cmidifile_read_track_header_info(cBytes infile, int32_t *num_bytes)
{
    char header[5];
    header[4] = '\0';
    _Err _errno = bytes_read(infile, 4, header);
    if (_errno != NO_ERR) {
        goto fn_fail;
    }
    if (strcmp(header, _TRACK_HEADER)) {
        // LOGERR("Invalid track chunk.\n");
        goto fn_fail;
    }
    *num_bytes = bytes_read_int32(infile);
    return NO_ERR;
fn_fail:
    return ERR;
}

static _Err midi_read_message(cBytes infile, int *running_status, MidiRow *msg)
{
    uint8_t statusbyte, junkbyte;
    cBytes data = NULL;
    int len_bytes;
    _Err _errno = NO_ERR;

    msg->delta = bytes_read_varint(infile);
    _errno = bytes_read(infile, 1, &statusbyte);
    if (_errno != NO_ERR) {
        goto fn_exit;
    }
    if (is_channel_status(statusbyte)) {
        *running_status = statusbyte;
        msg->status = get_status(statusbyte);
        msg->channel = get_channel(statusbyte);
        msg->mtype = MTYPE_CHAN;
        switch(msg->status) {
            case _NOTE_OFF:
            case _NOTE_ON:
            case _CONTROL_CHANGE:
            case _PITCH_BEND:
            case _POLY_TOUCH:
                len_bytes = 3;
                data = bytes_create(len_bytes);
                if (!data) {
                    _errno = ERR;
                    goto fn_exit;
                }
                _errno = bytes_write(data, 1, &statusbyte);
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
                _errno = bytes_read(infile, len_bytes - 1, bytes_ptr(data) + 1);
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
                break;
            case _AFTER_TOUCH:
            case _PROGRAM_CHANGE:
                len_bytes = 2;
                data = bytes_create(len_bytes);
                if (!data) {
                    _errno = ERR;
                    goto fn_exit;
                }
                _errno = bytes_write(data, 1, &statusbyte);
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
                _errno = bytes_read(infile, len_bytes - 1, bytes_ptr(data) + 1);
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
                break;
        }
    } else if (is_data_byte(statusbyte)) {
        if (*running_status == -1) {
            // LOGERR("Running status byte does not exist.\n");
            _errno = ERR;
            goto fn_exit;
        }
        msg->status = get_status(*running_status);
        msg->channel = get_channel(*running_status);
        msg->mtype = MTYPE_CHAN;
        switch (msg->status) {
            case _NOTE_OFF:
            case _NOTE_ON:
            case _CONTROL_CHANGE:
            case _PITCH_BEND:
            case _POLY_TOUCH:
                len_bytes = 3;
                data = bytes_create(len_bytes);
                if (!data) {
                    _errno = ERR;
                    goto fn_exit;
                }
                _errno = bytes_write(data, 1, running_status);
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
                _errno = bytes_write(data, 1, &statusbyte);
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
                _errno = bytes_read(infile, len_bytes - 2, bytes_ptr(data) + 2);
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
                break;
            case _AFTER_TOUCH:
            case _PROGRAM_CHANGE:
                len_bytes = 2;
                data = bytes_create(len_bytes);
                if (!data) {
                    _errno = ERR;
                    goto fn_exit;
                }
                _errno = bytes_write(data, 1, running_status);
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
                _errno = bytes_write(data, 1, &statusbyte);
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
                break;
        }
    } else {                   // Sysex, meta messages, and escape sequences
        *running_status = -1;  // unset running status
        switch (statusbyte) {
            case _META_EVENT:
                msg->mtype = MTYPE_META;
                _errno = bytes_read(infile, 1, &msg->status);
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
                len_bytes = bytes_read_varint(infile);
                data = bytes_create(len_bytes);
                if (!data) {
                    _errno = ERR;
                    goto fn_exit;
                }
                _errno = bytes_read(infile, len_bytes, bytes_ptr(data));
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
                break;
            case _SYSTEM_EXCLUSIVE:
                msg->mtype = MTYPE_SYSEX;
                len_bytes = bytes_read_varint(infile) - 1;
                data = bytes_create(len_bytes);
                if (!data) {
                    _errno = ERR;
                    goto fn_exit;
                }
                _errno = bytes_read(infile, len_bytes, bytes_ptr(data));
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
                _errno = bytes_read(infile, 1, &junkbyte);
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
                if (junkbyte != _END_OF_EXCLUSIVE) {
                    // LOGERR("SysEx message not terminated");
                    _errno = bytes_seek(infile, -1, SEEK_CUR);
                    if (_errno != NO_ERR) {
                        goto fn_exit;
                    }
                }
                break;
            case _ESCAPE_SEQUENCE:
                msg->mtype = MTYPE_ESC;
                len_bytes = bytes_read_varint(infile);
                data = bytes_create(len_bytes);
                if (!data) {
                    _errno = ERR;
                    goto fn_exit;
                }
                _errno = bytes_read(infile, len_bytes, bytes_ptr(data));
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
                break;
        }
    }
    msg->data = data;
fn_exit:
#ifdef FLUIDMIDI_DEBUG
    printf("%s: ", __func__);
    print_row(msg);
#endif

    return _errno;
}

void cmiditable_time_rel_to_abs(MidiRow *track, long num_msgs)
{
    long i, cumulative_time = 0;
    MidiRow *msg = NULL;
    for (i = 0; i < num_msgs; i++) {
        msg = &track[i];
        cumulative_time += msg->delta;
        msg->time = cumulative_time;
        msg->delta = 0;
    }
}

void cmiditable_note_durations_calculate(MidiRow *track, long num_msgs)
{
    long i, j, found;
    MidiRow *msgi = NULL;
    MidiRow *msgj = NULL;
    for (i = 0; i < num_msgs - 1; i++) {
        msgi = &track[i];
        if (msgi->mtype != MTYPE_NON)
            continue;
        found = 0;
        for (j = i + 1; j < num_msgs; j++) {
            msgj = &track[j];
            if (msgj->mtype == MTYPE_NOF && msgj->track == msgi->track && msgj->channel == msgi->channel && msgj->noteval == msgi->noteval) {
                found = 1;
                msgi->midi_d = msgj->time - msgi->time;
                break;
            }
        }
        if (found == 0) {
            msgi->mtype = MTYPE_EMPTY;
        }
    }
}

_Err cmidifile_read_track_messages_decode(cBytes infile, int32_t chunk_size, MidiRow *track, int *pnum_msgs)
{
    int num_msgs = 0;
    int running_status = -1;
    long chunk_start = bytes_tell(infile);
    MidiRow *msg = NULL;
    _Err _errno = NO_ERR;

    while (bytes_tell(infile) < chunk_start + chunk_size) {
        msg = &track[num_msgs];
        _errno = midi_read_message(infile, &running_status, msg);
        if (_errno != NO_ERR) {
            goto fn_exit;
        }
        _errno = decode_row(msg);
        if (_errno != NO_ERR) {
            goto fn_exit;
        }
        num_msgs++;
    }
    *pnum_msgs = num_msgs;
fn_exit:
    return _errno;
}

_Err cmiditable_copy_all_cbytes(MidiRow* from, MidiRow *to, long num_msgs)
{
    _Err _errno = NO_ERR;
    long i;
    MidiRow *fromrow = NULL;
    MidiRow *torow = NULL;
    for (i = 0; i < num_msgs; i++) {
        fromrow = &from[i];
        torow = &to[i];
        if (fromrow->data == NULL) {
            torow->data = NULL;
            continue;
        }
        torow->data = bytes_copy(fromrow->data);
        if (torow->data == NULL) {
            _errno = ERR;
            goto fn_exit;
        }
    }
fn_exit:
    return _errno;
}

_Err cmiditable_free_all_byte_array(MidiRow *table, long num_msgs)
{
    long i;
    MidiRow *row = NULL;
    _Err err;
    for (i = 0; i < num_msgs; i++) {
        row = &table[i];
        if (row->data == NULL) {
            continue;
        }
        err = bytes_destroy(&row->data);
        if (err != NO_ERR) {
            return err;
        }
    }
    return NO_ERR;
}

_Err cmiditable_remove_decoded_data(MidiRow *table, long num_msgs)
{
    long i;
    _Err err;
    MidiRow *msg = NULL;
    for (i = 0; i < num_msgs; i++) {
        msg = &table[i];
        err = remove_decoded_row_data(msg);
        if (err != NO_ERR) {
            return ERR;
        }
    }
    return NO_ERR;
}

_Err cmiditable_encode_table_data(MidiRow *table, long num_msgs)
{
    long i;
    _Err err;
    MidiRow *msg = NULL;
    for (i = 0; i < num_msgs; i++) {
        msg = &table[i];
        err = encode_row(msg);
        if (err != NO_ERR) {
            return ERR;
        }
    }
    return NO_ERR;
}

_Err cmidirow_encode(MidiRow *row)
{
    _Err err = NO_ERR;
    if (row->data != NULL) {
        goto fn_exit;
    }
    err = encode_row(row);
    if (err!= NO_ERR) {
        err = ERR;
        goto fn_exit;
    }
fn_exit:
    return err;
}

void cmiditable_scale_time_resolution(MidiRow *table, long num_msgs, int from_tpb, int to_tpb)
{
    long i;
    MidiRow *row = NULL;
    for (i = 0; i < num_msgs; i++) {
        row = &table[i];
        row->time = row->time * to_tpb / from_tpb;
        row->delta = row->delta * to_tpb / from_tpb;
        row->midi_d = row->midi_d * to_tpb / from_tpb;
        row->score_d = row->score_d * to_tpb / from_tpb;
    }
}

void cmiditable_time_abs_to_rel(MidiRow *table, long num_msgs)
{
    long i, prev_time = 0;
    MidiRow *msg = NULL;
    for (i = 0; i < num_msgs; i++) {
        msg = &table[i];
        msg->delta = msg->time - prev_time;
        prev_time = msg->time;
    }
}

_Err cmidifile_write_header_info(cBytes outfile, int16_t format, int16_t num_tracks, int16_t tpb)
{
    _Err _errno = bytes_write(outfile, 4, _FILE_HEADER);
    if (_errno != NO_ERR) {
        goto fn_exit;
    }
    _errno = bytes_write_int32(outfile, 6);
    if (_errno != NO_ERR) {
        goto fn_exit;
    }
    _errno = bytes_write_int16(outfile, format);
    if (_errno != NO_ERR) {
        goto fn_exit;
    }
    _errno = bytes_write_int16(outfile, num_tracks);
    if (_errno != NO_ERR) {
        goto fn_exit;
    }
    _errno = bytes_write_int16(outfile, tpb);
    if (_errno != NO_ERR) {
        goto fn_exit;
    }
fn_exit:
    return _errno;
}

static _Err _cmiditable_render_bytes(MidiRow *track, long num_msgs, cBytes btrack)
{
    int running_status = -1;
    int i;
    uint8_t byte;
    MidiRow *msg = NULL;
    _Err _errno = NO_ERR;

    for (i = 0; i < num_msgs; i++) {
        msg = &track[i];
        if (msg->data == NULL) {
            LOGERR("Binary data not populated in table.");
            return ERR;
        }
        _errno = bytes_write_varint(btrack, msg->delta);
        if (_errno != NO_ERR) {
            goto fn_exit;
        }
        if (is_data_byte(msg->status)) {
            running_status = -1;
            byte = _META_EVENT;
            _errno = bytes_write(btrack, 1, &byte);
            if (_errno != NO_ERR) {
                goto fn_exit;
            }
            _errno = bytes_write(btrack, 1, &msg->status);
            if (_errno != NO_ERR) {
                goto fn_exit;
            }
            _errno = bytes_write_varint(btrack, bytes_size(msg->data));
            if (_errno != NO_ERR) {
                goto fn_exit;
            }
            _errno = bytes_write(btrack, bytes_size(msg->data), bytes_ptr(msg->data));
            if (_errno != NO_ERR) {
                goto fn_exit;
            }
        }
        else if (msg->status == _SYSTEM_EXCLUSIVE) {
            running_status = -1;
            _errno = bytes_write(btrack, 1, &msg->status);
            if (_errno != NO_ERR) {
                goto fn_exit;
            }
            _errno = bytes_write_varint(btrack, bytes_size(msg->data) + 1);
            if (_errno != NO_ERR) {
                goto fn_exit;
            }
            _errno = bytes_write(btrack, bytes_size(msg->data), bytes_ptr(msg->data));
            if (_errno != NO_ERR) {
                goto fn_exit;
            }
            byte = _END_OF_EXCLUSIVE;
            _errno = bytes_write(btrack, 1, &byte);
            if (_errno != NO_ERR) {
                goto fn_exit;
            }
        }
        else if (msg->status == _ESCAPE_SEQUENCE) {
            running_status = -1;
            _errno = bytes_write(btrack, 1, &msg->status);
            if (_errno != NO_ERR) {
                goto fn_exit;
            }
            _errno = bytes_write_varint(btrack, bytes_size(msg->data));
            if (_errno != NO_ERR) {
                goto fn_exit;
            }
            _errno = bytes_write(btrack, bytes_size(msg->data), bytes_ptr(msg->data));
            if (_errno != NO_ERR) {
                goto fn_exit;
            }
        }
        else {
            if (running_status == get_statusbyte(msg->status, msg->channel)) {
                _errno = bytes_write(btrack, bytes_size(msg->data) - 1, bytes_ptr(msg->data) + 1);
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
            }
            else {
                running_status = get_statusbyte(msg->status, msg->channel);
                _errno = bytes_write(btrack, bytes_size(msg->data), bytes_ptr(msg->data));
                if (_errno != NO_ERR) {
                    goto fn_exit;
                }
            }
        }
    }
    _errno = bytes_finalize(btrack);
fn_exit:
    return _errno;
}

_Err cmidifile_write_track_chunk(cBytes outfile, MidiRow *track, long num_msgs)
{
    _Err _errno = NO_ERR;
    cBytes track_chunk;
    track_chunk = bytes_create(num_msgs * 4);
    if (!track_chunk) {
        _errno = ERR;
        goto fn_exit;
    }
    _errno = bytes_write(outfile, 4, _TRACK_HEADER);
    if (_errno != NO_ERR) {
        goto fn_exit;
    }
    _errno = _cmiditable_render_bytes(track, num_msgs, track_chunk);
    if (_errno != NO_ERR) {
        goto fn_exit;
    }
    _errno = bytes_write_int32(outfile, bytes_size(track_chunk));
    if (_errno != NO_ERR) {
        goto fn_exit;
    }
    _errno = bytes_write(outfile, bytes_size(track_chunk), bytes_ptr(track_chunk));
    if (_errno != NO_ERR) {
        goto fn_exit;
    }
    _errno = bytes_destroy(&track_chunk);
fn_exit:
    return _errno;
}

double ctick2second(long tick, int tempo, int tpb)
{
    return 1e-6 * tick * tempo / tpb;
}

long csecond2tick(double sec, int tempo, int tpb)
{
    return (long) (1e6 * sec * tpb / tempo);
}

int cbpm2tempo(double bpm)
{
    return (int) (6e7 / bpm);
}

double ctempo2bpm(int tempo)
{
    return 6.0e7 / tempo;
}

static int comparator_row_time(const MidiRow *a, const MidiRow *b)
{
    return (a->time < b->time) ? -1 : ((a->time > b->time) ? 1 : 0);
}

static int comparator_row_time_track(const MidiRow *a, const MidiRow *b)
{
    int cmp = comparator_row_time(a, b);
    if (cmp != 0) return cmp;
    cmp = (a->track < b->track) ? -1 : ((a->track > b->track) ? 1 : 0);
    return cmp;
}

static int comparator_row_time_mtype(const MidiRow *a, const MidiRow *b)
{
    int cmp = comparator_row_time(a, b);
    if (cmp != 0) return cmp;
    cmp = (a->mtype < b->mtype) ? -1 : ((a->mtype > b->mtype) ? 1 : 0);
    return cmp;
}

typedef int (*comparator_fn) (const MidiRow *, const MidiRow *);

// # define HYBRID_SORT_PRUNE_LIMIT  (10)

// // Insertion Sort function for sorting small subarrays
// void insertionSort(MidiRow *arr[], long n, comparator_fn fn) {
//     long i, j;
//     MidiRow *key;
//     for (i = 1; i < n; i++) {
//         key = arr[i];
//         j = i - 1;

//         // Move elements of arr[0..i-1], that are greater than key, to one position ahead of their current position
//         while (j >= 0 && fn(arr[j], key) > 0) {
//             arr[j + 1] = arr[j];
//             j--;
//         }
//         arr[j + 1] = key;
//     }
// }

// Merge function for merging two sorted halves
static _Err _merge(MidiRow *arr[], long l, long m, long r, comparator_fn fn) {
    _Err err = NO_ERR;
    long i, j, k;
    long n1 = m - l + 1;
    long n2 = r - m;

    // Create temporary arrays to hold the two halves
    MidiRow **leftArray = (MidiRow **) calloc (n1, sizeof(MidiRow *));
    if (!leftArray) {
        err = ERR;
        goto fn_exit;
    }
    MidiRow **rightArray = (MidiRow **) calloc (n2, sizeof(MidiRow *));
    if (!rightArray) {
        free(leftArray);
        err = ERR;
        goto fn_exit;
    }
    // Copy data to temporary arrays
    memcpy(leftArray, arr + l, n1 * sizeof(MidiRow *));
    memcpy(rightArray, arr + m + 1, n2 * sizeof(MidiRow *));

    // Merge the temporary arrays back into arr[l..r]
    i = j = 0;
    k = l;
    while (i < n1 && j < n2) {
        if (fn(leftArray[i], rightArray[j]) <= 0) {
            arr[k] = leftArray[i];
            i++;
        } else {
            arr[k] = rightArray[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of leftArray[], if any
    while (i < n1) {
        arr[k] = leftArray[i];
        i++;
        k++;
    }

    // Copy the remaining elements of rightArray[], if any
    while (j < n2) {
        arr[k] = rightArray[j];
        j++;
        k++;
    }
    free(leftArray);
    free(rightArray);
fn_exit:
    return err;
}

// MergeSort function for sorting the array using Merge Sort
static _Err _mergesort_recur(MidiRow *arr[], long l, long r, comparator_fn fn) {
    _Err err = NO_ERR;
    if (l < r) {
        long m = l + (r - l) / 2;

        err = _mergesort_recur(arr, l, m, fn);
        if (err != NO_ERR) {
            return err;
        }
        err = _mergesort_recur(arr, m + 1, r, fn);
        if (err != NO_ERR) {
            return err;
        }
        err = _merge(arr, l, m, r, fn);
    }
    return err;
}

static _Err cmergesort(MidiRow table[], long num_msgs, comparator_fn fn)
{
    _Err err = NO_ERR;
    // Make a copy of the table
    size_t tsize = num_msgs * sizeof(MidiRow);
    MidiRow *temp = (MidiRow *) malloc (tsize);
    if (!temp) {
        err = ERR;
        goto fn_exit;
    }
    memcpy(temp, table, tsize);
    // Create an array of row pointers
    MidiRow **lptable = (MidiRow **) malloc (num_msgs * sizeof(MidiRow *));
    long i;
    for (i = 0; i < num_msgs; i++) {
        lptable[i] = &(temp[i]);
    }
    // Merge sort the array of row pointers
    _mergesort_recur(lptable, 0, num_msgs - 1, fn);
    // Copy the temp rows in new order to the original table
    for (i = 0; i < num_msgs; i++) {
        memcpy(&table[i], lptable[i], sizeof(MidiRow));
    }
    free(lptable);
    free(temp);
fn_exit:
    return err;
}

_Err cmiditable_sort(MidiRow table[], long num_msgs, SortComparator comp_e)
{
    comparator_fn fp = NULL;
    switch (comp_e) {
        case SC_SORT_DEFAULT:
        case SC_SORT_TIME_TRACK:
            fp = comparator_row_time_track;
            break;
        case SC_SORT_TIME_MTYPE:
            fp = comparator_row_time_mtype;
            break;
        default:
            return ERR;
    }
    return cmergesort(table, num_msgs, fp);
}

_Err cmiditable_merge_sorted(MidiRow *final, int num_tables, MidiRow **tbl_ptrs, int *table_lens, SortComparator comp_e)
{
    _Err err;
    if (!final) {
        goto fn_fail;
    }
    int i;
    long int insert_pos = 0;
    MidiRow *table = NULL;
    for (i = 0; i < num_tables; i++) {
        table = tbl_ptrs[i];
        memcpy(&final[insert_pos], table, table_lens[i] * sizeof(MidiRow));
        insert_pos += table_lens[i];
    }
    err = cmiditable_copy_all_cbytes(final, final, insert_pos);
    if (err != NO_ERR) {
        goto fn_fail;
    }
    return cmiditable_sort(final, insert_pos, comp_e);
fn_fail:
    return ERR;
}

_Err cmiditable_timecatenate(MidiRow *final, int num_tables, MidiRow **tbl_ptrs, int *table_lens, int32_t *max_times)
{
    if (!final) {
        goto fn_fail;
    }
    int i;
    long int insert_pos = 0, j;
    int32_t last_time = 0;
    MidiRow *table = NULL;
    _Err err;
    for (i = 0; i < num_tables; i++) {
        table = tbl_ptrs[i];
        memcpy(&final[insert_pos], table, table_lens[i] * sizeof(MidiRow));
        err = cmiditable_copy_all_cbytes(table, &final[insert_pos], table_lens[i]);
        if (err != NO_ERR) {
            goto fn_fail;
        }
        for (j = insert_pos; j < insert_pos + table_lens[i]; j++) {
            final[j].time += last_time;
        }
        last_time = max_times[i];
        insert_pos += table_lens[i];
    }
    return NO_ERR;
fn_fail:
    return ERR;
}

_Err cmiditable_sorted_insert(MidiRow final[], MidiRow target[], long target_len, MidiRow inserts[], long inserts_len)
{
    _Err err = NO_ERR;
    if (!final || !target || !inserts) {
        err = ERR;
        goto fn_exit;
    }
    long target_idx = 0;
    long insert_idx = 0;
    long final_idx = 0;
    long final_size = target_len + inserts_len;
    size_t row_size = sizeof(MidiRow);

    while (target_idx < target_len && insert_idx < inserts_len) {
        if (comparator_row_time_mtype(&target[target_idx], &inserts[insert_idx]) <= 0) {
            memcpy(&final[final_idx], &target[target_idx], row_size);
            target_idx++;
        } else {
            memcpy(&final[final_idx], &inserts[insert_idx], row_size);
            insert_idx++;
        }
        final_idx++;
    }

    while (target_idx < target_len) {
        memcpy(&final[final_idx], &target[target_idx], row_size);
        target_idx++;
        final_idx++;
    }

    while (insert_idx < inserts_len) {
        memcpy(&final[final_idx], &inserts[insert_idx], row_size);
        insert_idx++;
        final_idx++;
    }
    err = cmiditable_copy_all_cbytes(final, final, final_size);
fn_exit:
    return err;
}

int cmidirow_equality(MidiRow *a, MidiRow *b)
{
    if (a->time != b->time)
        return 0;
    if (a->track != b->track)
        return 0;
    if (a->channel != b->channel)
        return 0;
    if (a->mtype != b->mtype)
        return 0;
    if (a->status != b->status)
        return 0;
    if (a->program != b->program)
        return 0;
    if (a->control != b->control)
        return 0;
    if (a->cc_val != b->cc_val)
        return 0;
    if (a->noteval != b->noteval)
        return 0;
    if (a->notevel != b->notevel)
        return 0;
    if (a->midi_d != b->midi_d)
        return 0;
    if (a->score_d != b->score_d)
        return 0;
    if (a->pitch != b->pitch)
        return 0;
    if (a->delta != b->delta)
        return 0;
    return bytes_compare(a->data, b->data);
}
