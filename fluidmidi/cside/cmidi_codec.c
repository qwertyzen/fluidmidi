// #include "cbytearray.h"
#include "utils.h"
#include "cmidiconst.h"
#include "cmidi_types.h"
#include "cmidi_codec.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

// Channel message status decode/encode functions
int get_status(uint8_t statusbyte)
{
    return statusbyte & 0xF0;
}

int get_channel(uint8_t statusbyte)
{
    return statusbyte & 0x0F;
}

int is_channel_status(uint8_t statusbyte)
{
    return ((_CHAN_L <= statusbyte) && (statusbyte <= _CHAN_H)) ? 1 : 0;
}

int is_data_byte(uint8_t statusbyte)
{
    return !(statusbyte & 0x80);
}

uint8_t get_statusbyte(uint8_t status, uint8_t channel)
{
    return status | channel;
}

// Channel message decode functions
static void decode_note_off(MidiRow *row)
{
    uint8_t *ptr = bytes_ptr(row->data);
    row->noteval = ptr[1];
    row->notevel = ptr[2];
    row->mtype = MTYPE_NOF;
}

static void decode_note_on(MidiRow *row)
{
    uint8_t *ptr = bytes_ptr(row->data);
    row->noteval = ptr[1];
    row->notevel = ptr[2];
    row->mtype = MTYPE_NON;
    if (ptr[2] == 0) {
        row->mtype = MTYPE_NOF;
    }
}

static void decode_control_change(MidiRow *row)
{
    uint8_t *ptr = bytes_ptr(row->data);
    row->control = ptr[1];
    row->cc_val = ptr[2];
    row->mtype = MTYPE_CC;
}

static void decode_program_change(MidiRow *row)
{
    uint8_t *ptr = bytes_ptr(row->data);
    row->program = ptr[1];
    row->mtype = MTYPE_PC;
}

static void decode_polyphonic_pressure(MidiRow *row)
{
    uint8_t *ptr = bytes_ptr(row->data);
    row->noteval = ptr[1];
    row->notevel = ptr[2];
    row->mtype = MTYPE_POLYP;
}

static void decode_channel_pressure(MidiRow *row)
{
    uint8_t *ptr = bytes_ptr(row->data);
    row->notevel = ptr[1];
    row->mtype = MTYPE_CHANP;
}

int32_t decode_pitch_bend_data(uint8_t *ptr)
{
    return ptr[1] | ((ptr[2] << 7) + _MIN_PITCHWHEEL);
}

static void decode_pitch_bend(MidiRow *row)
{
    uint8_t *ptr = bytes_ptr(row->data);
    row->pitch = decode_pitch_bend_data(ptr);
    row->mtype = row->pitch ? MTYPE_BEND1 : MTYPE_BEND0;
}

// Meta message decode/encode functions

// Codec funcs for _SEQUENCE_NUMBER
static void decode_meta_sequence_number(MidiRow *row)
{
    uint8_t *ptr = bytes_ptr(row->data);
    if (bytes_size(row->data) == 0)
        row->pitch = 0;
    else {
        row->pitch = (ptr[0] << 8) | ptr[1];
    }
}

static void decode_meta_track_name(MidiRow *row)
{
    row->mtype = MTYPE_TRACK;
}

// Codec funcs for _MIDI_PORT
static int decode_meta_midi_port(cBytes data)
{
    if (bytes_size(data) == 0)
        return 0;
    else
        return bytes_ptr(data)[0];
}

// Codec funcs for _END_OF_TRACK
static void decode_meta_end_of_track(MidiRow *row)
{
    row->mtype = MTYPE_EOT;
}

// Codec funcs for _TEMPO_SET
static void decode_meta_tempo_set(MidiRow *row)
{
    uint8_t *ptr = bytes_ptr(row->data);
    row->pitch = (ptr[0] << 16) | (ptr[1] << 8) | ptr[2];
    row->mtype = MTYPE_TEMPO;
}

// Codec funcs for _TIME_SIGNATURE
static void decode_meta_time_signature(MidiRow *row)
{
    uint8_t *ptr = bytes_ptr(row->data);
    row->program = ptr[0];
    row->control = (ptr[1] < 8) ? (1 << ptr[1]) : 4;
    row->cc_val = ptr[2];
    row->noteval = ptr[3];
    row->mtype = MTYPE_TSIG;
}

// Codec funcs for _KEY_SIGNATURE  (Not implemented)

// Codec funcs for _SYSTEM_EXCLUSIVE
static void decode_sysex(MidiRow *row)
{
    row->mtype = MTYPE_SYSEX;
}

// Codec funcs for _ESCAPE_SEQUENCE
static void decode_escape_sequence(MidiRow *row)
{
    row->mtype = MTYPE_ESC;
}

// Main decode function for row messages
_Err decode_row(MidiRow *row)
{
    switch (row->status) {
        case _NOTE_OFF:
            decode_note_off(row);
            break;
        case _NOTE_ON:
            decode_note_on(row);
            break;
        case _CONTROL_CHANGE:
            decode_control_change(row);
            break;
        case _PITCH_BEND:
            decode_pitch_bend(row);
            break;
        case _POLY_TOUCH:
            decode_polyphonic_pressure(row);
            break;
        case _AFTER_TOUCH:
            decode_channel_pressure(row);
            break;
        case _PROGRAM_CHANGE:
            decode_program_change(row);
            break;

        case _TRACK_NAME:
            decode_meta_track_name(row);
            break;
        case _END_OF_TRACK:
            decode_meta_end_of_track(row);
            break;
        case _TEMPO_SET:
            decode_meta_tempo_set(row);
            break;
        case _TIME_SIGNATURE:
            decode_meta_time_signature(row);
            break;

        case _SYSTEM_EXCLUSIVE:
            decode_sysex(row);
            break;
        case _ESCAPE_SEQUENCE:
            decode_escape_sequence(row);
            break;
    }
    return NO_ERR;
}

// Encoder functions
static _Err encode_note_off(MidiRow *row)
{
    row->data = bytes_create(3);
    if (row->data == NULL) {
        return ERR;
    }
    row->status = _NOTE_OFF;
    uint8_t* data = bytes_ptr(row->data);
    data[0] = get_statusbyte(row->status, row->channel);
    data[1] = row->noteval;
    data[2] = row->notevel;
    return NO_ERR;
}

static _Err encode_note_on(MidiRow *row)
{
    row->data = bytes_create(3);
    if (row->data == NULL) {
        return ERR;
    }
    row->status = _NOTE_ON;
    uint8_t* data = bytes_ptr(row->data);
    data[0] = get_statusbyte(row->status, row->channel);
    data[1] = row->noteval;
    data[2] = row->notevel;
    return NO_ERR;
}

static _Err encode_control_change(MidiRow *row)
{
    row->data = bytes_create(3);
    if (row->data == NULL) {
        return ERR;
    }
    row->status = _CONTROL_CHANGE;
    uint8_t* data = bytes_ptr(row->data);
    data[0] = get_statusbyte(row->status, row->channel);
    data[1] = row->control;
    data[2] = row->cc_val;
    return NO_ERR;
}

static _Err encode_pitch_bend(MidiRow *row)
{
    row->data = bytes_create(3);
    if (row->data == NULL) {
        return ERR;
    }
    row->status = _PITCH_BEND;
    uint8_t* data = bytes_ptr(row->data);
    data[0] = get_statusbyte(row->status, row->channel);
    int32_t pitch = row->pitch - _MIN_PITCHWHEEL;
    data[1] = pitch & 0x7f;
    data[2] = pitch >> 7;
    return NO_ERR;
}

static _Err encode_polyphonic_pressure(MidiRow *row)
{
    row->data = bytes_create(3);
    if (row->data == NULL) {
        return ERR;
    }
    row->status = _POLY_TOUCH;
    uint8_t* data = bytes_ptr(row->data);
    data[0] = get_statusbyte(row->status, row->channel);
    data[1] = row->noteval;
    data[2] = row->notevel;
    return NO_ERR;
}

static _Err encode_program_change(MidiRow *row)
{
    row->data = bytes_create(2);
    if (row->data == NULL) {
        return ERR;
    }
    row->status = _PROGRAM_CHANGE;
    uint8_t* data = bytes_ptr(row->data);
    data[0] = get_statusbyte(row->status, row->channel);
    data[1] = row->program;
    return NO_ERR;
}

static _Err encode_channel_pressure(MidiRow *row)
{
    row->data = bytes_create(2);
    if (row->data == NULL) {
        return ERR;
    }
    row->status = _AFTER_TOUCH;
    uint8_t* data = bytes_ptr(row->data);
    data[0] = get_statusbyte(row->status, row->channel);
    data[1] = row->notevel;
    return NO_ERR;
}

static _Err encode_meta_end_of_track(MidiRow *row)
{
    row->status = _END_OF_TRACK;
    row->data = bytes_create(0);
    if (row->data == NULL) {
        return ERR;
    }
    return NO_ERR;
}

static _Err encode_meta_tempo_set(MidiRow *row)
{
    row->data = bytes_create(3);
    if (row->data == NULL) {
        return ERR;
    }
    int32_t tempo = row->pitch;
    uint8_t *data = bytes_ptr(row->data);
    if (tempo > _DEFAULT_TEMPO_MAX) {
        return ERR;
    }
    data[0] = tempo >> 16;
    data[1] = tempo >> 8 & 0xff;
    data[2] = tempo & 0xff;
    return NO_ERR;
}

static _Err encode_meta_time_signature(MidiRow *row)
{
    row->data = bytes_create(4);
    if (row->data == NULL) {
        return ERR;
    }
    uint8_t *data = bytes_ptr(row->data);
    data[0] = row->program;
    data[1] = (int) log2(row->control);
    data[2] = row->cc_val;
    data[3] = row->noteval;
    return NO_ERR;
}

_Err encode_row(MidiRow *row)
{
    _Err err = NO_ERR;
    switch (row->mtype) {
        case MTYPE_NOF:
            err = encode_note_off(row);
            break;
        case MTYPE_NON:
            err = encode_note_on(row);
            break;
        case MTYPE_CC:
            err = encode_control_change(row);
            break;
        case MTYPE_BEND0:
        case MTYPE_BEND1:
            err = encode_pitch_bend(row);
            break;
        case MTYPE_POLYP:
            err = encode_polyphonic_pressure(row);
            break;
        case MTYPE_CHANP:
            err = encode_channel_pressure(row);
            break;
        case MTYPE_PC:
            err = encode_program_change(row);
            break;

        case MTYPE_EOT:
            err = encode_meta_end_of_track(row);
            break;
        case MTYPE_TEMPO:
            err = encode_meta_tempo_set(row);
            break;
        case MTYPE_TSIG:
            err = encode_meta_time_signature(row);
            break;
    }
    return err;
}

_Err remove_decoded_row_data(MidiRow *row)
{
    row->delta = 0;
    if (row->data == NULL) {
        return NO_ERR;
    }
    switch (row->mtype) {
        case MTYPE_NOF:
        case MTYPE_NON:
        case MTYPE_CC:
        case MTYPE_BEND0:
        case MTYPE_BEND1:
        case MTYPE_POLYP:
        case MTYPE_CHANP:
        case MTYPE_PC:
        case MTYPE_EOT:
        case MTYPE_TEMPO:
        case MTYPE_TSIG:
            bytes_destroy(&row->data);
    }
    return NO_ERR;
}
