# cython: language_level=3
from libc.stdint cimport uint8_t, int32_t, int16_t, uint32_t, uintptr_t

cdef extern from 'utils.h':
    cdef int NO_ERR
    cdef int ERR
    ctypedef int _Err

cdef extern from 'cbytearray.h':
    cdef struct byte_array_s:
        pass
    ctypedef byte_array_s *cBytes

    cBytes   bytes_create(long size)
    cBytes   bytes_copy(cBytes obj)
    _Err     bytes_destroy(cBytes *pobj)

    uint8_t* bytes_ptr(cBytes array)
    long     bytes_size(cBytes array)

    cBytes   bytes_fload(const char *fname)
    long     bytes_tell(cBytes array)
    _Err     bytes_read(cBytes array, int num_bytes, void *tobuf)
    int32_t  bytes_read_int32(cBytes array)
    int16_t  bytes_read_int16(cBytes array)
    int      bytes_read_varint(cBytes array)

    _Err     bytes_write(cBytes array, int num_bytes, void *frombuf)
    _Err     bytes_write_varint(cBytes array, unsigned int value)
    _Err     bytes_write_int32(cBytes array, int32_t value)
    _Err     bytes_write_int16(cBytes array, int16_t value)
    _Err     bytes_fdump(cBytes array, const char *fname)
    _Err     bytes_finalize(cBytes array)
    int      bytes_compare(cBytes array, cBytes cmp_to)

cdef extern from 'cmidi_types.h':
    ctypedef enum cMtype:
        MTYPE_EMPTY, MTYPE_META, MTYPE_TEMPO, MTYPE_TSIG, MTYPE_TRACK, MTYPE_SYSEX, MTYPE_ESC,
        MTYPE_PC, MTYPE_CC, MTYPE_POLYP, MTYPE_CHANP, MTYPE_CHAN,
        MTYPE_BEND1, MTYPE_NOF, MTYPE_BEND0, MTYPE_NON,
        MTYPE_REST,
        MTYPE_EOT,
        MTYPE_REC

    cdef struct midi_row_s:
        int32_t       time      # Absolute time in midi ticks
        uint8_t       track
        uint8_t       channel
        uint8_t       mtype     # A musiclab spec to identify message type, used in sorting order
        uint8_t       status
        uint8_t       program
        uint8_t       control
        uint8_t       cc_val
        uint8_t       noteval
        uint8_t       notevel
        uint32_t      midi_d    # Duration of a note event in ticks (note off time - note on time)
        uint32_t      score_d   # Duration of note as expressed in a musical score
        int32_t       pitch     # Bend value
        uint32_t      delta     # delta time in midi ticks
        void*         data      # Raw midi data as ByteArray

    ctypedef midi_row_s MidiRow

    void print_row(MidiRow *row)
    void ctable_print(MidiRow *table, int num_rows, int print_bytes)
    void ctable_compare_print(MidiRow *table1, MidiRow *table2, long num_rows)

cdef extern from "cmidi.h":
    _Err   cmidifile_read_header_info(cBytes infile, int16_t *format, int16_t *num_tracks, int16_t *tpb)
    _Err   cmidifile_read_track_header_info(cBytes infile, int32_t *num_bytes)
    _Err   cmidifile_read_track_messages_decode(cBytes infile, int32_t chunk_size, MidiRow *track, int *pnum_msgs)
    void   cmiditable_time_rel_to_abs(MidiRow *track, long num_msgs)
    void   cmiditable_note_durations_calculate(MidiRow *track, long num_msgs)
    _Err   cmiditable_remove_decoded_data(MidiRow *table, long num_msgs)
    _Err   cmiditable_encode_table_data(MidiRow *table, long num_msgs)
    _Err   cmidirow_encode(MidiRow *row)
    void   cmiditable_scale_time_resolution(MidiRow *table, long num_msgs, int from_tpb, int to_tpb)

    ctypedef enum SortComparator:
        SC_SORT_DEFAULT,
        SC_SORT_TIME_TRACK,
        SC_SORT_TIME_MTYPE

    _Err   cmiditable_sort(MidiRow *table, long num_msgs, SortComparator comp_e)
    _Err   cmiditable_merge_sorted(MidiRow *final, int num_tables, MidiRow **tbl_ptrs, int *table_lens, SortComparator comp_e)
    _Err   cmiditable_timecatenate(MidiRow *final, int num_tables, MidiRow **tbl_ptrs, int *table_lens, int32_t *max_times)
    _Err   cmiditable_sorted_insert(MidiRow *final, MidiRow *target, long target_len, MidiRow *inserts, long inserts_len)
    int    cmidirow_equality(MidiRow *a, MidiRow *b)

    double ctick2second(long tick, int tempo, int tpb)
    long   csecond2tick(double sec, int tempo, int tpb)
    int    cbpm2tempo(double bpm)
    double ctempo2bpm(int tempo)

    void   cmiditable_time_abs_to_rel(MidiRow *table, long num_msgs)
    _Err   cmidifile_write_header_info(cBytes outfile, int16_t format, int16_t num_tracks, int16_t tpb)
    _Err   cmidifile_write_track_chunk(cBytes outfile, MidiRow *track, long num_msgs)

    _Err   cmiditable_copy_all_cbytes(MidiRow *fromtable, MidiRow *totable, long num_msgs)
    _Err   cmiditable_free_all_byte_array(MidiRow *table, long num_msgs)

cdef extern from "cmidi_codec.h":
    int is_channel_status(uint8_t statusbyte)

cdef extern from "cmidiconst.h":
    const char** program_changes
    const char** control_changes

    cdef const int _DEFAULT_TICKS_PER_BEAT
    cdef const int _DEFAULT_NOTE_OFF_VEL
    cdef const int _DEFAULT_NOTEVEL
    cdef const int _NOTE_OFF
    cdef const int _NOTE_ON
    cdef const int _CONTROL_CHANGE
    cdef const int _PROGRAM_CHANGE
    cdef const int _PITCH_BEND
    cdef const int _POLY_TOUCH
    cdef const int _AFTER_TOUCH
    cdef const int _RT_TIMING_CLOCK
    cdef const int _RT_UNDEFINED
    cdef const int _RT_START
    cdef const int _RT_CONTINUE
    cdef const int _RT_STOP
    cdef const int _RT_UNDEFINED2
    cdef const int _RT_ACTIVE_SENSING
    cdef const int _RT_SYSTEM_RESET
    cdef const int _META_EVENT
    cdef const int _SYSTEM_EXCLUSIVE
    cdef const int _END_OF_EXCLUSIVE
    cdef const int _ESCAPE_SEQUENCE
    cdef const int _SEQUENCE_NUMBER
    cdef const int _TEXT
    cdef const int _COPYRIGHT
    cdef const int _TRACK_NAME
    cdef const int _INSTRUMENT_NAME
    cdef const int _LYRICS
    cdef const int _MARKER
    cdef const int _CUE_MARKER
    cdef const int _DEVICE_NAME
    cdef const int _CHANNEL_PREFIX
    cdef const int _MIDI_PORT
    cdef const int _END_OF_TRACK
    cdef const int _TEMPO_SET
    cdef const int _SMPTE_OFFSET
    cdef const int _TIME_SIGNATURE
    cdef const int _KEY_SIGNATURE
    cdef const int _SEQUENCER_SPECIFIC
    cdef const int _MIN_PITCHWHEEL
    cdef const int _MAX_PITCHWHEEL
    cdef const int _DEFAULT_TSIG_CLOCKS_PER_CLICK
    cdef const int _DEFAULT_TSIG_32nd_NOTES_PER_BEAT
    cdef const int _DEFAULT_TEMPO_MAX

    cdef const int _HIRES_COMBINED_MAX
    cdef const int _HIRES_MSB_LSB_DIFF

    cdef const int _CC_BANK_SELECT
    cdef const int _CC_MODULATION_WHEEL
    cdef const int _CC_BREATH_CONTROLLER
    cdef const int _CC_FOOT_CONTROLLER
    cdef const int _CC_PORTAMENTO_TIME
    cdef const int _CC_DATA_ENTRY
    cdef const int _CC_CHANNEL_VOLUME
    cdef const int _CC_BALANCE
    cdef const int _CC_PAN
    cdef const int _CC_EXPRESSION_CONTROLLER
    cdef const int _CC_EFFECT_CONTROL_1
    cdef const int _CC_EFFECT_CONTROL_2
    cdef const int _CC_GEN_CONTROLLER_1
    cdef const int _CC_GEN_CONTROLLER_2
    cdef const int _CC_GEN_CONTROLLER_3
    cdef const int _CC_GEN_CONTROLLER_4
    cdef const int _CC_LSB_BANK_SELECT
    cdef const int _CC_LSB_MODULATION_WHEEL
    cdef const int _CC_LSB_BREATH_CONTROLLER
    cdef const int _CC_LSB_FOOT_CONTROLLER
    cdef const int _CC_LSB_PORTAMENTO_TIME
    cdef const int _CC_LSB_DATA_ENTRY
    cdef const int _CC_LSB_CHANNEL_VOLUME
    cdef const int _CC_LSB_BALANCE
    cdef const int _CC_LSB_PAN
    cdef const int _CC_LSB_EXPRESSION_CONTROLLER
    cdef const int _CC_LSB_EFFECT_CONTROL_1
    cdef const int _CC_LSB_EFFECT_CONTROL_2
    cdef const int _CC_LSB_GEN_CONTROLLER_1
    cdef const int _CC_LSB_GEN_CONTROLLER_2
    cdef const int _CC_LSB_GEN_CONTROLLER_3
    cdef const int _CC_LSB_GEN_CONTROLLER_4
    cdef const int _CC_SWITCH_SUSTAIN
    cdef const int _CC_SWITCH_PORTAMENTO
    cdef const int _CC_SWITCH_SOSTENUTO
    cdef const int _CC_SWITCH_SOFT_PEDAL
    cdef const int _CC_SWITCH_LEGATO
    cdef const int _CC_SWITCH_HOLD_2
    cdef const int _CC_SOUND_CONTROLLER_1
    cdef const int _CC_SOUND_CONTROLLER_2
    cdef const int _CC_SOUND_CONTROLLER_3
    cdef const int _CC_SOUND_CONTROLLER_4
    cdef const int _CC_SOUND_CONTROLLER_5
    cdef const int _CC_SOUND_CONTROLLER_6
    cdef const int _CC_SOUND_CONTROLLER_7
    cdef const int _CC_SOUND_CONTROLLER_8
    cdef const int _CC_SOUND_CONTROLLER_9
    cdef const int _CC_SOUND_CONTROLLER_10
    cdef const int _CC_GEN_CONTROLLER_5
    cdef const int _CC_GEN_CONTROLLER_6
    cdef const int _CC_GEN_CONTROLLER_7
    cdef const int _CC_GEN_CONTROLLER_8
    cdef const int _CC_PORTAMENTO_CONTROL
    cdef const int _CC_HIGH_RES_VELOCITY_PREFIX
    cdef const int _CC_EFFECTS_1_DEPTH
    cdef const int _CC_EFFECTS_2_DEPTH
    cdef const int _CC_EFFECTS_3_DEPTH
    cdef const int _CC_EFFECTS_4_DEPTH
    cdef const int _CC_EFFECTS_5_DEPTH
    cdef const int _CC_DATA_INCREMENT
    cdef const int _CC_DATA_DECREMENT
    cdef const int _CC_NRPN_LSB
    cdef const int _CC_NRPN_MSB
    cdef const int _CC_RPN_LSB
    cdef const int _CC_RPN_MSB
    cdef const int _CC_CH_ALL_SOUND_OFF
    cdef const int _CC_CH_RESET_ALL_CONTROLLERS
    cdef const int _CC_CH_LOCAL_CONTROL
    cdef const int _CC_CH_ALL_NOTES_OFF
    cdef const int _CC_CH_OMNI_OFF
    cdef const int _CC_CH_OMNI_ON
    cdef const int _CC_CH_MONO_ON
    cdef const int _CC_CH_POLY_ON
    cdef const int _CC_UNDEFINED_1
    cdef const int _CC_UNDEFINED_2
    cdef const int _CC_UNDEFINED_3
    cdef const int _CC_UNDEFINED_4
    cdef const int _CC_UNDEFINED_5
    cdef const int _CC_UNDEFINED_6
    cdef const int _CC_UNDEFINED_7
    cdef const int _CC_UNDEFINED_8
    cdef const int _CC_UNDEFINED_9
    cdef const int _CC_UNDEFINED_10
    cdef const int _CC_UNDEFINED_11
    cdef const int _CC_UNDEFINED_12
    cdef const int _CC_UNDEFINED_13
    cdef const int _CC_UNDEFINED_14
    cdef const int _CC_UNDEFINED_15
    cdef const int _CC_UNDEFINED_16
    cdef const int _CC_UNDEFINED_17
    cdef const int _CC_UNDEFINED_18
    cdef const int _CC_UNDEFINED_19
    cdef const int _CC_UNDEFINED_20
    cdef const int _CC_UNDEFINED_21
    cdef const int _CC_UNDEFINED_22
    cdef const int _CC_UNDEFINED_23
    cdef const int _CC_UNDEFINED_24
    cdef const int _CC_UNDEFINED_25
    cdef const int _CC_UNDEFINED_26
    cdef const int _CC_UNDEFINED_27
    cdef const int _CC_UNDEFINED_28
    cdef const int _CC_UNDEFINED_29
    cdef const int _CC_UNDEFINED_30
    cdef const int _CC_UNDEFINED_31
    cdef const int _CC_UNDEFINED_32
    cdef const int _CC_UNDEFINED_33
    cdef const int _CC_UNDEFINED_34
    cdef const int _CC_UNDEFINED_35
    cdef const int _CC_UNDEFINED_36
    cdef const int _CC_UNDEFINED_37
    cdef const int _CC_UNDEFINED_38
    cdef const int _CC_UNDEFINED_39
    cdef const int _CC_UNDEFINED_40
    cdef const int _CC_UNDEFINED_41
    cdef const int _CC_UNDEFINED_42
    cdef const int _CC_UNDEFINED_43
    cdef const int _CC_UNDEFINED_44
    cdef const int _CC_UNDEFINED_45
    cdef const int _CC_UNDEFINED_46
    cdef const int _CC_UNDEFINED_47
    cdef const int _CC_UNDEFINED_48
    cdef const int _CC_UNDEFINED_49
    cdef const int _CC_UNDEFINED_50
    cdef const int _CC_UNDEFINED_51
    cdef const int _CC_UNDEFINED_52
    cdef const int _CC_UNDEFINED_53
    cdef const int _CC_UNDEFINED_54
    cdef const int _CC_UNDEFINED_55
