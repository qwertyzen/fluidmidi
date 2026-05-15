#ifndef __CMIDI_CODEC_H__
#define __CMIDI_CODEC_H__

#include <stdint.h>
#include "cmidi_types.h"

int get_status(uint8_t statusbyte);
int get_channel(uint8_t statusbyte);
int is_channel_status(uint8_t statusbyte);
int is_data_byte(uint8_t statusbyte);
uint8_t get_statusbyte(uint8_t status, uint8_t channel);

_Err decode_row(MidiRow *row);
_Err encode_row(MidiRow *row);
_Err remove_decoded_row_data(MidiRow *row);

int32_t decode_pitch_bend_data(uint8_t *ptr);

#endif  // __CMIDI_CODEC_H__
