# cython: language_level=3
from enum import IntEnum
import numpy as np
cimport numpy as np
np.import_array()
from libc.stdlib cimport calloc, free
from .bytearray cimport pyByteArray
from .cside cimport (
    int32_t, int16_t, uint8_t, uintptr_t,
    _Err, NO_ERR, cBytes, bytes_finalize,
    MTYPE_EMPTY, MTYPE_META, MTYPE_TEMPO, MTYPE_TSIG, MTYPE_TRACK, MTYPE_SYSEX, MTYPE_ESC,
    MTYPE_PC, MTYPE_CC, MTYPE_POLYP, MTYPE_CHANP, MTYPE_CHAN,
    MTYPE_BEND1, MTYPE_NOF, MTYPE_BEND0, MTYPE_NON,
    MTYPE_REST,
    MTYPE_EOT,
    MTYPE_REC,
    MidiRow,
    cmiditable_free_all_byte_array,
    cmiditable_copy_all_cbytes,
    cmidifile_read_header_info,
    cmidifile_read_track_header_info,
    cmidifile_read_track_messages_decode,
    cmiditable_time_rel_to_abs,
    cmiditable_note_durations_calculate,
    cmiditable_remove_decoded_data,
    cmiditable_encode_table_data,
    cmidirow_encode,
    cmiditable_scale_time_resolution,

    _DEFAULT_TICKS_PER_BEAT,
    _DEFAULT_NOTE_OFF_VEL,
    _DEFAULT_TEMPO_MAX,
    _DEFAULT_TSIG_CLOCKS_PER_CLICK,
    _DEFAULT_TSIG_32nd_NOTES_PER_BEAT,
    _TEXT, _TRACK_NAME,
    _END_OF_TRACK,
    _TEMPO_SET, _TIME_SIGNATURE,

    ctable_print,
    ctable_compare_print,

    SC_SORT_DEFAULT,
    cmiditable_sort,
    cmiditable_merge_sorted,
    cmiditable_timecatenate,
    cmiditable_sorted_insert,
    cmidirow_equality,

    ctick2second,
    csecond2tick,
    cbpm2tempo,
    ctempo2bpm,

    cmiditable_time_abs_to_rel,
    cmidifile_write_header_info,
    cmidifile_write_track_chunk,

    program_changes,
    control_changes,
)

from .cside cimport (
    _MIN_PITCHWHEEL, _MAX_PITCHWHEEL,
    _HIRES_MSB_LSB_DIFF, _HIRES_COMBINED_MAX,
    _CC_BANK_SELECT, _CC_MODULATION_WHEEL, _CC_BREATH_CONTROLLER, _CC_FOOT_CONTROLLER, _CC_PORTAMENTO_TIME, _CC_DATA_ENTRY,
    _CC_CHANNEL_VOLUME, _CC_BALANCE, _CC_PAN, _CC_EXPRESSION_CONTROLLER, _CC_EFFECT_CONTROL_1, _CC_EFFECT_CONTROL_2,
    _CC_GEN_CONTROLLER_1, _CC_GEN_CONTROLLER_2, _CC_GEN_CONTROLLER_3, _CC_GEN_CONTROLLER_4, _CC_LSB_BANK_SELECT,
    _CC_LSB_MODULATION_WHEEL, _CC_LSB_BREATH_CONTROLLER, _CC_LSB_FOOT_CONTROLLER, _CC_LSB_PORTAMENTO_TIME, _CC_LSB_DATA_ENTRY,
    _CC_LSB_CHANNEL_VOLUME, _CC_LSB_BALANCE, _CC_LSB_PAN, _CC_LSB_EXPRESSION_CONTROLLER, _CC_LSB_EFFECT_CONTROL_1,
    _CC_LSB_EFFECT_CONTROL_2, _CC_LSB_GEN_CONTROLLER_1, _CC_LSB_GEN_CONTROLLER_2, _CC_LSB_GEN_CONTROLLER_3,
    _CC_LSB_GEN_CONTROLLER_4, _CC_SWITCH_SUSTAIN, _CC_SWITCH_PORTAMENTO, _CC_SWITCH_SOSTENUTO, _CC_SWITCH_SOFT_PEDAL,
    _CC_SWITCH_LEGATO, _CC_SWITCH_HOLD_2, _CC_SOUND_CONTROLLER_1, _CC_SOUND_CONTROLLER_2, _CC_SOUND_CONTROLLER_3,
    _CC_SOUND_CONTROLLER_4, _CC_SOUND_CONTROLLER_5, _CC_SOUND_CONTROLLER_6, _CC_SOUND_CONTROLLER_7, _CC_SOUND_CONTROLLER_8,
    _CC_SOUND_CONTROLLER_9, _CC_SOUND_CONTROLLER_10, _CC_GEN_CONTROLLER_5, _CC_GEN_CONTROLLER_6, _CC_GEN_CONTROLLER_7,
    _CC_GEN_CONTROLLER_8, _CC_PORTAMENTO_CONTROL, _CC_HIGH_RES_VELOCITY_PREFIX, _CC_EFFECTS_1_DEPTH, _CC_EFFECTS_2_DEPTH,
    _CC_EFFECTS_3_DEPTH, _CC_EFFECTS_4_DEPTH, _CC_EFFECTS_5_DEPTH, _CC_DATA_INCREMENT, _CC_DATA_DECREMENT, _CC_NRPN_LSB,
    _CC_NRPN_MSB, _CC_RPN_LSB, _CC_RPN_MSB, _CC_CH_ALL_SOUND_OFF, _CC_CH_RESET_ALL_CONTROLLERS, _CC_CH_LOCAL_CONTROL,
    _CC_CH_ALL_NOTES_OFF, _CC_CH_OMNI_OFF, _CC_CH_OMNI_ON, _CC_CH_MONO_ON, _CC_CH_POLY_ON, _CC_UNDEFINED_1, _CC_UNDEFINED_2,
    _CC_UNDEFINED_3, _CC_UNDEFINED_4, _CC_UNDEFINED_5, _CC_UNDEFINED_6, _CC_UNDEFINED_7, _CC_UNDEFINED_8, _CC_UNDEFINED_9,
    _CC_UNDEFINED_10, _CC_UNDEFINED_11, _CC_UNDEFINED_12, _CC_UNDEFINED_13, _CC_UNDEFINED_14, _CC_UNDEFINED_15, _CC_UNDEFINED_16,
    _CC_UNDEFINED_17, _CC_UNDEFINED_18, _CC_UNDEFINED_19, _CC_UNDEFINED_20, _CC_UNDEFINED_21, _CC_UNDEFINED_22, _CC_UNDEFINED_23,
    _CC_UNDEFINED_24, _CC_UNDEFINED_25, _CC_UNDEFINED_26, _CC_UNDEFINED_27, _CC_UNDEFINED_28, _CC_UNDEFINED_29, _CC_UNDEFINED_30,
    _CC_UNDEFINED_31, _CC_UNDEFINED_32, _CC_UNDEFINED_33, _CC_UNDEFINED_34, _CC_UNDEFINED_35, _CC_UNDEFINED_36, _CC_UNDEFINED_37,
    _CC_UNDEFINED_38, _CC_UNDEFINED_39, _CC_UNDEFINED_40, _CC_UNDEFINED_41, _CC_UNDEFINED_42, _CC_UNDEFINED_43, _CC_UNDEFINED_44,
    _CC_UNDEFINED_45, _CC_UNDEFINED_46, _CC_UNDEFINED_47, _CC_UNDEFINED_48, _CC_UNDEFINED_49, _CC_UNDEFINED_50, _CC_UNDEFINED_51,
    _CC_UNDEFINED_52, _CC_UNDEFINED_53, _CC_UNDEFINED_54, _CC_UNDEFINED_55,
    _RT_TIMING_CLOCK, _RT_UNDEFINED, _RT_START, _RT_CONTINUE, _RT_STOP, _RT_UNDEFINED2, _RT_ACTIVE_SENSING, _RT_SYSTEM_RESET,
)

class CC_UNDEFINED:
    def __init__(self) -> None:
        self.ccs = [_CC_UNDEFINED_1, _CC_UNDEFINED_2,
            _CC_UNDEFINED_3, _CC_UNDEFINED_4, _CC_UNDEFINED_5, _CC_UNDEFINED_6, _CC_UNDEFINED_7, _CC_UNDEFINED_8, _CC_UNDEFINED_9,
            _CC_UNDEFINED_10, _CC_UNDEFINED_11, _CC_UNDEFINED_12, _CC_UNDEFINED_13, _CC_UNDEFINED_14, _CC_UNDEFINED_15, _CC_UNDEFINED_16,
            _CC_UNDEFINED_17, _CC_UNDEFINED_18, _CC_UNDEFINED_19, _CC_UNDEFINED_20, _CC_UNDEFINED_21, _CC_UNDEFINED_22, _CC_UNDEFINED_23,
            _CC_UNDEFINED_24, _CC_UNDEFINED_25, _CC_UNDEFINED_26, _CC_UNDEFINED_27, _CC_UNDEFINED_28, _CC_UNDEFINED_29, _CC_UNDEFINED_30,
            _CC_UNDEFINED_31, _CC_UNDEFINED_32, _CC_UNDEFINED_33, _CC_UNDEFINED_34, _CC_UNDEFINED_35, _CC_UNDEFINED_36, _CC_UNDEFINED_37,
            _CC_UNDEFINED_38, _CC_UNDEFINED_39, _CC_UNDEFINED_40, _CC_UNDEFINED_41, _CC_UNDEFINED_42, _CC_UNDEFINED_43, _CC_UNDEFINED_44,
            _CC_UNDEFINED_45, _CC_UNDEFINED_46, _CC_UNDEFINED_47, _CC_UNDEFINED_48, _CC_UNDEFINED_49, _CC_UNDEFINED_50, _CC_UNDEFINED_51,
            _CC_UNDEFINED_52, _CC_UNDEFINED_53, _CC_UNDEFINED_54, _CC_UNDEFINED_55
        ]

    def __getitem__(self, key):
        return self.ccs[key]

    def __len__(self):
        return len(self.ccs)

CC_UNDEFINED = CC_UNDEFINED()

DEFAULT_TICKS_PER_BEAT = _DEFAULT_TICKS_PER_BEAT
DEFAULT_NOTE_OFF_VEL = _DEFAULT_NOTE_OFF_VEL
HIRES_MSB_LSB_DIFF = _HIRES_MSB_LSB_DIFF
MIN_PITCHWHEEL = _MIN_PITCHWHEEL
MAX_PITCHWHEEL = _MAX_PITCHWHEEL

class Mtype(IntEnum):
    EMPTY = MTYPE_EMPTY
    META = MTYPE_META
    TEMPO = MTYPE_TEMPO
    TSIG = MTYPE_TSIG
    TRACK = MTYPE_TRACK
    SYSEX = MTYPE_SYSEX
    ESCAPE = MTYPE_ESC
    PC = MTYPE_PC
    CC = MTYPE_CC
    POLYP = MTYPE_POLYP
    CHANP = MTYPE_CHANP
    CHAN = MTYPE_CHAN
    BEND1 = MTYPE_BEND1
    NOF = MTYPE_NOF
    BEND0 = MTYPE_BEND0
    NON = MTYPE_NON
    REST = MTYPE_REST
    EOT = MTYPE_EOT
    REC = MTYPE_REC

ROW_TYPE = np.dtype([
    ('time', np.int32),
    ('track', np.uint8),
    ('channel', np.uint8),
    ('mtype', np.uint8),
    ('status', np.uint8),
    ('program', np.uint8),
    ('control', np.uint8),
    ('cc_val', np.uint8),
    ('noteval', np.uint8),
    ('notevel', np.uint8),
    ('midi_d', np.uint32),
    ('score_d', np.uint32),
    ('pitch', np.int32),
    ('delta', np.uint32),
    ('data', np.uintp),
], align=True)

def select_mtype_channnel_msgs(np.ndarray ndarray):
    return (
        (ndarray['mtype'] == Mtype.BEND0) |
        (ndarray['mtype'] == Mtype.BEND1) |
        (ndarray['mtype'] == Mtype.CC) |
        (ndarray['mtype'] == Mtype.CHAN) |
        (ndarray['mtype'] == Mtype.CHANP) |
        (ndarray['mtype'] == Mtype.NOF) |
        (ndarray['mtype'] == Mtype.NON) |
        (ndarray['mtype'] == Mtype.PC) |
        (ndarray['mtype'] == Mtype.POLYP)
    )

def apply_kwargs_ndarray(row, **colvals):
    if any([kw not in ROW_TYPE.names for kw in colvals.keys()]):
        raise KeyError("Got bad column name")
    for col, val in colvals.items():
        row[col] = val
    channel = colvals.pop('channel', None)
    if channel is not None:
        row['channel'][select_mtype_channnel_msgs(row)] = channel
    return row

def get_column_array(np.ndarray ndarray, column_name: str) -> np.ndarray:
    dtype = ROW_TYPE.fields[column_name][0]
    return np.array(ndarray[column_name], dtype=dtype)

class FluidMsg:
    mtype = Mtype.EMPTY

    def __init__(self, **kwargs):
        self._row = apply_kwargs_ndarray(create_empty_array(1), mtype=self.mtype, **kwargs)

class ChannelMsg(FluidMsg):
    @property
    def data(self):
        cdef _Err err
        err = cmidirow_encode(ndarray_ptr(self._row))
        if err != NO_ERR:
            raise ValueError("Failed to get MIDI row data")
        ptr = self._row['data'][0]
        data = pyByteArray.from_pyobj(ptr)
        return data.get_bytes()

class NoteOff(ChannelMsg):
    mtype = Mtype.NOF

    def __init__(self, **colvals):
        colvals['notevel'] = colvals.get('notevel', _DEFAULT_NOTE_OFF_VEL)
        super().__init__(**colvals)

class NoteOn(ChannelMsg):
    mtype = Mtype.NON

class ControlChange(ChannelMsg):
    mtype = Mtype.CC

class ProgramChange(ChannelMsg):
    mtype = Mtype.PC

class PitchBend(ChannelMsg):
    def __init__(self, **colvals):
        pitch = colvals.get('pitch', 0)
        mtype = Mtype.BEND0
        if pitch:
            mtype = Mtype.BEND1
        colvals['mtype'] = mtype
        self._row = apply_kwargs_ndarray(create_empty_array(1), **colvals)

class RealtimeMessage:
    data = _RT_UNDEFINED.to_bytes()

class RTTimingClock(RealtimeMessage):
    data = _RT_TIMING_CLOCK.to_bytes()

class RTStart(RealtimeMessage):
    data = _RT_START.to_bytes()

class RTContinue(RealtimeMessage):
    data = _RT_CONTINUE.to_bytes()

class RTStop(RealtimeMessage):
    data = _RT_STOP.to_bytes()

class RTUndefined(RealtimeMessage):
    data = _RT_UNDEFINED2.to_bytes()

class MetaMessage(FluidMsg):
    mtype = Mtype.META
    status = 0

    def __init__(self, **colvals):
        colvals['status'] = self.status
        super().__init__(**colvals)

class TempoSet(MetaMessage):
    mtype = Mtype.TEMPO
    status = _TEMPO_SET

    def __init__(self, *, bpm=120, **colvals):
        tempo = cbpm2tempo(bpm)
        if (tempo > _DEFAULT_TEMPO_MAX):
            tempo = _DEFAULT_TEMPO_MAX
        super().__init__(pitch=tempo, **colvals)

class TimeSignature(MetaMessage):
    mtype = Mtype.TSIG
    status = _TIME_SIGNATURE

    def __init__(self, *, numerator=4, denominator=4, **colvals):
        super().__init__(program=numerator, control=denominator,
                         cc_val=_DEFAULT_TSIG_CLOCKS_PER_CLICK,
                         noteval=_DEFAULT_TSIG_32nd_NOTES_PER_BEAT,
                         **colvals)

class Text(MetaMessage):
    mtype = Mtype.META
    status = _TEXT

    def __init__(self, *, text: str, **colvals):
        cdef pyByteArray data = pyByteArray.from_str(text)
        super().__init__(data=<uintptr_t> data.array, **colvals)
        data.array = NULL

class TrackName(Text):
    mtype = Mtype.TRACK
    status = _TRACK_NAME

    def __init__(self, *, name: str, **colvals):
        super().__init__(text=name, **colvals)

# Implementation of FluidTree
class FluidNode:
    def __init__(self, children: list):
        self.children = children

    def __iter__(self):
        for child in self.children:
            if isinstance(child, FluidMsg):
                yield child
            elif isinstance(child, FluidNode):
                yield from child
            else:
                raise TypeError("Must be FluidNode or FluidMsg type")
    def append(self, child):
        self.children.append(child)

class ControlChange1(FluidNode):
    def __init__(self, cc_val=0):
        super().__init__([ControlChange(control=self._control, cc_val=cc_val)])

class ControlChange2(FluidNode):

    def __init__(self, ctl1=None, ctl2=None, val1=None, val2=None, **kwargs):
        m = []
        if val1 is not None:
            m.append(ControlChange(control=ctl1, cc_val=val1))
        if val2 is not None:
            m.append(ControlChange(control=ctl2, cc_val=val2))
        if not m:
            raise Exception("Both values cannot be None")
        super().__init__(m, **kwargs)

# High Resolution Controls
cdef convert_14bit_to_msb_lsb(num: int):
    if num == 0:
        return 0, 0
    num = num - 1
    lsb = num % 128
    msb = num // 128
    return msb, lsb

cdef convert_abs_to_msb_lsb(double num):
    cdef int x = round(num * _HIRES_COMBINED_MAX)
    return convert_14bit_to_msb_lsb(x)

cdef convert_pct_to_msb_lsb(double pct):
    return convert_abs_to_msb_lsb(pct / 100.0)

class HighResContinuousController(ControlChange2):
    _control = None

    def __init__(self, msb:int=None, lsb:int=None, hires:int=None, abs:float=None, pct:float=None, **kwargs):
        if hires is not None:
            assert 0 <= hires <= _HIRES_COMBINED_MAX, "High resolution control value out of allowed range"
            msb, lsb = convert_14bit_to_msb_lsb(hires)
        if abs is not None:
            assert 0 <= abs <= 1.0, "Control abs value must be in range [0, 1]"
            msb, lsb = convert_abs_to_msb_lsb(abs)
        if pct is not None:
            assert 0 <= pct <= 100.0, "Percent value out of range"
            msb, lsb = convert_pct_to_msb_lsb(pct)
        if msb is None and lsb is None:
            raise ValueError('All arguments cannot be None')
        super().__init__(ctl1=self._control, ctl2=self._control + _HIRES_MSB_LSB_DIFF, val1=msb, val2=lsb, **kwargs)

class ControlBankSelect(HighResContinuousController):
    _control = _CC_BANK_SELECT

class ControlModulationWheel(HighResContinuousController):
    _control = _CC_MODULATION_WHEEL

class ControlBreathController(HighResContinuousController):
    _control = _CC_BREATH_CONTROLLER

class ControlFootController(HighResContinuousController):
    _control = _CC_FOOT_CONTROLLER

class ControlPortamentoTime(HighResContinuousController):
    _control = _CC_PORTAMENTO_TIME

class ControlDataEntry(HighResContinuousController):
    _control = _CC_DATA_ENTRY

class ControlChannelVolume(HighResContinuousController):
    _control = _CC_CHANNEL_VOLUME

class ControlBalance(HighResContinuousController):
    _control = _CC_BALANCE

class ControlPan(HighResContinuousController):
    _control = _CC_PAN

class ControlExpressionController(HighResContinuousController):
    _control = _CC_EXPRESSION_CONTROLLER

class ControlEffect1(HighResContinuousController):
    _control = _CC_EFFECT_CONTROL_1

class ControlEffect2(HighResContinuousController):
    _control = _CC_EFFECT_CONTROL_2

# Control switches
class ControlSwitch(FluidNode):
    _control = None

    def __init__(self, state: bool):
        m = ControlChange(control=self._control, cc_val=127 if state else 0)
        super().__init__([m])

class SwitchSustain(ControlSwitch):
    _control = _CC_SWITCH_SUSTAIN

class SwitchPortamento(ControlSwitch):
    _control = _CC_SWITCH_PORTAMENTO

class SwitchSostenuto(ControlSwitch):
    _control = _CC_SWITCH_SOSTENUTO

class SwitchSoftPedal(ControlSwitch):
    _control = _CC_SWITCH_SOFT_PEDAL

class SwitchLegato(ControlSwitch):
    _control = _CC_SWITCH_LEGATO

class SwitchHold2(ControlSwitch):
    _control = _CC_SWITCH_HOLD_2

# RPNs & NRPNs
class ControlDataIncrement(ControlChange1):
    _control = _CC_DATA_INCREMENT

    def __init__(self, value):
        super().__init__(cc_val=value)

class ControlDataDecrement(ControlChange1):
    _control = _CC_DATA_DECREMENT

class ControlRPNSelect(ControlChange2):
    _msb, _lsb = None, None
    _ctl1, _ctl2 = _CC_RPN_LSB, _CC_RPN_MSB

    def __init__(self):
        super().__init__(ctl1=self._ctl1, ctl2=self._ctl2, val1=self._lsb, val2=self._msb)

# class ControlNRPNSelect(FluidNode):
#     def __init__(self, lsb=None, msb=None):
#         m = []
#         if lsb is not None:
#             m.append(ControlChange(control=_CC_NRPN_LSB, cc_val=lsb))
#         if msb is not None:
#             m.append(ControlChange(control=_CC_NRPN_MSB, cc_val=msb))
#         if not m:
#             raise Exception("NPRN message needs LSB, MSB values")
#         super().__init__(m)

class RPNSelectPitchBendSensitivity(ControlRPNSelect):
    _msb, _lsb = 0, 0

class RPNSelectChannelFineTuning(ControlRPNSelect):
    _msb, _lsb = 0, 1

class RPNSelectChannelCoarseTuning(ControlRPNSelect):
    _msb, _lsb = 0, 2

class RPNSelectTuningProgram(ControlRPNSelect):
    _msb, _lsb = 0, 3

class RPNSelectTuningBank(ControlRPNSelect):
    _msb, _lsb = 0, 4

class RPNSelectModulationDepthRange(ControlRPNSelect):
    _msb, _lsb = 0, 5

class RPNNullFunction(ControlRPNSelect):
    _msb, _lsb = 0x7f, 0x7f

# Channel Mode Messages
class AllSoundOff(ControlChange1):
    _control = _CC_CH_ALL_SOUND_OFF

class ResetAllControllers(ControlChange1):
    _control = _CC_CH_RESET_ALL_CONTROLLERS

class SwitchLocalControl(ControlSwitch):
    _control = _CC_CH_LOCAL_CONTROL

class AllNotesOff(ControlChange1):
    _control = _CC_CH_ALL_NOTES_OFF

class OmniOff(ControlChange1):
    _control = _CC_CH_OMNI_OFF

class OmniOn(ControlChange1):
    _control = _CC_CH_OMNI_OFF

class MonoOn(ControlChange1):
    _control = _CC_CH_MONO_ON

class PolyOn(ControlChange1):
    _control = _CC_CH_POLY_ON

# Other useful midi utilities
class BankProgram(FluidNode):
    def __init__(self, msb=0, lsb=None, program=0):
        super().__init__([
            ControlBankSelect(msb=msb, lsb=lsb),
            ProgramChange(program=program)
        ])

cdef bint validate_list_of_fluidmsg(list_of_msgs: list):
    cdef int i
    for i in range(len(list_of_msgs)):
        if not isinstance(list_of_msgs[i], FluidMsg):
            return 0
    return 1

cdef FluidMidi miditable_timecatenate(tables: list[FluidMidi]):
    cdef int num_tables = <int> len(tables)
    cdef int i
    cdef int *table_lens = <int *> calloc(num_tables, sizeof(int))
    cdef int32_t *max_times = <int32_t *> calloc(num_tables, sizeof(int32_t))
    cdef MidiRow **tbl_ptrs = <MidiRow **> calloc(num_tables, sizeof(MidiRow *))
    cdef FluidMidi temp
    cdef long total_cnt_msgs = 0
    cdef int32_t table_cum_times = 0
    for i in range(num_tables):
        temp = tables[i]
        table_lens[i] = temp.size
        total_cnt_msgs += temp.size
        tbl_ptrs[i] = ndarray_ptr(temp.arr)
        table_cum_times += max_time(temp)
        max_times[i] = table_cum_times
    cdef FluidMidi final = FluidMidi(total_cnt_msgs)
    cdef _Err err = cmiditable_timecatenate(ndarray_ptr(final.arr), num_tables, tbl_ptrs, table_lens, max_times)
    if err != NO_ERR:
        raise Exception("Failed to timecatenate the tables")
    free(table_lens)
    free(tbl_ptrs)
    free(max_times)
    return final

cdef FluidMidi miditable_merge_sorted(tables: list[FluidMidi]):
    cdef int num_tables = <int> len(tables)
    cdef int i
    cdef int *table_lens = <int *> calloc(num_tables, sizeof(int))
    cdef long total_cnt_msgs = 0
    cdef MidiRow **tbl_ptrs = <MidiRow **> calloc(num_tables, sizeof(MidiRow *))
    cdef FluidMidi temp
    for i in range(num_tables):
        temp = tables[i]
        table_lens[i] = temp.size
        total_cnt_msgs += temp.size
        tbl_ptrs[i] = ndarray_ptr(temp.arr)
    cdef FluidMidi final = FluidMidi(total_cnt_msgs)
    cdef _Err err = cmiditable_merge_sorted(ndarray_ptr(final.arr), num_tables, tbl_ptrs, table_lens, SC_SORT_DEFAULT)
    if err != NO_ERR:
        raise Exception("Failed to merge tables")
    free(table_lens)
    free(tbl_ptrs)
    return final

def midirow_equality(row1, row2) -> bool:
    cdef int eq = cmidirow_equality(ndarray_ptr(row1), ndarray_ptr(row2))
    return bool(eq)

cdef np.ndarray combine_list_of_fluidmsg(list msgs):
    return np.concatenate([msg._row for msg in msgs])

cdef MidiRow* ndarray_ptr(np.ndarray table):
    cdef MidiRow* ptr = <MidiRow *> table.data
    return ptr

cdef long int ndarray_size(np.ndarray table):
    return <long int> table.size

cdef np.ndarray create_empty_array(long num_rows):
    cdef np.ndarray array = np.zeros(num_rows, dtype=ROW_TYPE)
    return array

cdef bint check_dtype(np.ndarray arr):
    return getattr(arr, 'dtype', None) == ROW_TYPE

cdef class FluidMidi:

    def __init__(self, arg, **colvals):
        if isinstance(arg, int):
            self.arr = create_empty_array(arg)
        elif isinstance(arg, FluidNode):
            self.arr = combine_list_of_fluidmsg(list(arg))
            err = cmiditable_sort(ndarray_ptr(self.arr), self.size, SC_SORT_DEFAULT)
            if err != NO_ERR:
                raise Exception('Failed to sort FluidNode created table')
        elif isinstance(arg, list) and validate_list_of_fluidmsg(arg):
            self.arr = combine_list_of_fluidmsg(arg)
        else:
            raise TypeError("Bad argument")
        self.arr = apply_kwargs_ndarray(self.arr, **colvals)

    @property
    def ndarray(self):
        return self.arr

    def make(self) -> FluidMidi:
        cmiditable_sort(ndarray_ptr(self.arr), self.size, SC_SORT_DEFAULT)
        b = insert_eot(insert_nof(self))
        cmiditable_time_abs_to_rel(ndarray_ptr(b.arr), b.size)
        cdef _Err err = cmiditable_encode_table_data(ndarray_ptr(b.arr), b.size )
        if err != NO_ERR:
            raise Exception("Failed to encode midi data.")
        return b

    def __del__(self):
        cdef _Err err = cmiditable_free_all_byte_array(ndarray_ptr(self.arr), self.size)
        if err != NO_ERR:
            raise Exception("Failed to free data memory")

    @property
    def size(self) -> int:
        return ndarray_size(self.arr)

    def print(self, debug=False):
        ctable_print(ndarray_ptr(self.arr), self.size, debug)

    def __getitem__(self, idx):
        cdef np.ndarray obj = self.arr[idx]
        if not check_dtype(obj):
            return obj
        if not obj.flags.owndata:
            obj = np.copy(obj)
        cdef FluidMidi newobj = self.__class__(0)
        cdef _Err err = cmiditable_copy_all_cbytes(ndarray_ptr(obj), ndarray_ptr(obj), ndarray_size(obj))
        if err != NO_ERR:
            raise Exception("Failed to copy data in place")
        newobj.arr = obj
        return newobj

    def __setitem__(self, key, value):
        self.arr[key] = value

    def __add__(self, other: FluidMidi):
        return miditable_timecatenate([self, other])

    def __or__(self, other: FluidMidi):
        return miditable_merge_sorted([self, other])

    def __mul__(self, other: int):
        return miditable_timecatenate([self] * other)

# cdef FluidMidi cast_ndarray_to_FluidMidi(np.ndarray array):
#     cdef FluidMidi table = FluidMidi()
#     table.arr = array
#     return table

def max_time(table):
    return np.max(table['time'] + table['midi_d'])

def midifile_extract_data(pyByteArray midiin) -> FluidMidi:
    cdef int16_t format, num_tracks, tpb
    cdef int32_t num_bytes
    cdef _Err err

    err = cmidifile_read_header_info(midiin.array, &format, &num_tracks, &tpb)
    if err != NO_ERR:
        raise Exception('Bad midi file')
    cdef uint8_t trackno
    cdef int num_msgs
    cdef FluidMidi track
    cdef list tracks = []

    for trackno in range(num_tracks):
        err = cmidifile_read_track_header_info(midiin.array, &num_bytes)
        if err != NO_ERR:
            raise Exception('Bad midi file')
        track = FluidMidi(num_bytes)
        err = cmidifile_read_track_messages_decode(midiin.array, num_bytes, ndarray_ptr(track.arr), &num_msgs)
        if err != NO_ERR:
            raise Exception('Failed to read midi track')
        track = track[:num_msgs]
        cmiditable_time_rel_to_abs(ndarray_ptr(track.arr), track.size)
        track.arr['track'] = trackno
        cmiditable_note_durations_calculate(ndarray_ptr(track.arr), track.size)
        track = track[track['mtype'] != MTYPE_EMPTY]
        if (tpb != _DEFAULT_TICKS_PER_BEAT):
            cmiditable_scale_time_resolution(ndarray_ptr(track.arr), track.size, tpb, _DEFAULT_TICKS_PER_BEAT)
        tracks.append(track)

    cdef FluidMidi merged = miditable_merge_sorted(tracks)
    cmiditable_time_abs_to_rel(ndarray_ptr(merged.arr), merged.size)
    for trackno in range(num_tracks):
        track = tracks[trackno]
        err = cmiditable_free_all_byte_array(ndarray_ptr(track.arr), track.size)
        if err != NO_ERR:
            raise Exception("Failed to free memory.")
    # cdef FluidMidi midifile = cast_ndarray_to_FluidMidi(merged)
    return merged

def midibytes_load(pyByteArray midiin) -> FluidMidi:
    midifile = midifile_extract_data(midiin)
    return midifile

def midifile_load(str fname) -> FluidMidi:
    cdef pyByteArray midiin = pyByteArray.fload(fname)
    return midibytes_load(midiin)

def split_table_by_column(FluidMidi table, str colname) -> list[FluidMidi]:
    cdef _Err err
    cdef np.ndarray unique_vals = np.unique(table[colname])
    cdef np.ndarray idcs
    cdef FluidMidi temp
    cdef list tracks = []
    cdef int value
    for value in unique_vals:
        idcs = (table['track'] == value)
        temp = table[idcs]
        err = cmiditable_copy_all_cbytes(ndarray_ptr(temp.arr), ndarray_ptr(temp.arr), temp.size)
        if err != NO_ERR:
            raise Exception("Failed to copy midi data into table")
        tracks.append(temp)
    return tracks

def split_tracks(FluidMidi table) -> list[FluidMidi]:
    return split_table_by_column(table, 'track')

def midibytes_save_1(FluidMidi midi) -> pyByteArray:
    cdef list tracks = split_tracks(midi)
    cdef FluidMidi temp
    cdef pyByteArray midifile = pyByteArray.create(1000)
    cdef int trackno

    err = cmidifile_write_header_info(<cBytes> midifile.array, <int16_t> 1, <int16_t> len(tracks), _DEFAULT_TICKS_PER_BEAT)
    if err != NO_ERR:
        raise Exception("Failed to write midi header information")

    for trackno in range(len(tracks)):
        temp = tracks[trackno]
        cmiditable_time_abs_to_rel(ndarray_ptr(temp.arr), temp.size)
        err = cmidifile_write_track_chunk(midifile.array, ndarray_ptr(temp.arr), temp.size)
        if err != NO_ERR:
            raise Exception("Failed to write track data")
    err = bytes_finalize(midifile.array)
    if err != NO_ERR:
        raise Exception("Failed to finalize bytearray")
    return midifile

def midifile_save_1(FluidMidi midi, str fname) -> None:
    cdef pyByteArray midifbytes = midibytes_save_1(midi)
    midifbytes.fwrite(fname)

def midibytes_save_0(FluidMidi midi) -> pyByteArray:
    cdef pyByteArray midifile = pyByteArray.create(1000)
    cdef _Err err = cmidifile_write_header_info(midifile.array, 0, 1, _DEFAULT_TICKS_PER_BEAT)
    if err != NO_ERR:
        raise Exception("Failed to write midi header information")
    err = cmidifile_write_track_chunk(midifile.array, ndarray_ptr(midi.arr), midi.size)
    if err != NO_ERR:
        raise Exception("Failed to write track data")
    err = bytes_finalize(midifile.array)
    if err != NO_ERR:
        raise Exception("Failed to finalize bytearray")
    return midifile

def midifile_save_0(FluidMidi midi, str fname) -> None:
    cdef pyByteArray track = midibytes_save_0(midi)
    track.fwrite(fname)

def tick2second(tick, tempo, tpb: int = _DEFAULT_TICKS_PER_BEAT) -> float:
    return ctick2second(<long> tick, <long> tempo, <int> tpb)

def second2tick(sec, tempo, tpb: int = _DEFAULT_TICKS_PER_BEAT) -> int:
    return csecond2tick(<double> sec, <int> tempo, tpb)

def bpm2tempo(bpm) -> int:
    return cbpm2tempo(<double> bpm)

def tempo2bpm(tempo) -> float:
    return ctempo2bpm(<int> tempo)

def remove_decoded_data(FluidMidi table):
    cdef _Err err = cmiditable_remove_decoded_data(ndarray_ptr(table.arr), table.size)
    if err != NO_ERR:
        raise Exception("Failed to remove decoded data from table")

def encode_table(FluidMidi table):
    cdef _Err err = cmiditable_encode_table_data(ndarray_ptr(table.arr), table.size)
    if err != NO_ERR:
        raise Exception("Failed to encode midi data in table")

cdef FluidMidi insert_eot(FluidMidi table):
    cdef np.ndarray no_eot = table.arr[table.arr['mtype'] != Mtype.EOT]
    cdef np.ndarray num_tracks = np.unique(no_eot['track'])
    cdef np.ndarray eots = create_empty_array(ndarray_size(num_tracks))
    cdef uint8_t track
    eots['mtype'] = Mtype.EOT
    eots['status'] = _END_OF_TRACK
    for i in range(num_tracks.size):
        track = <uint8_t> num_tracks[i]
        eots['track'][i] = track
        eots['time'][i] = max_time(no_eot[no_eot['track'] == track])
    err = cmiditable_sort(ndarray_ptr(eots), ndarray_size(eots), SC_SORT_DEFAULT);
    if err != NO_ERR:
        raise Exception("Failed to sort eots")
    cdef FluidMidi final = FluidMidi(no_eot.size + eots.size)
    err = cmiditable_sorted_insert(ndarray_ptr(final.arr), ndarray_ptr(no_eot), ndarray_size(no_eot), ndarray_ptr(eots), ndarray_size(eots))
    if err != NO_ERR:
        raise Exception("Failed to insert eots into table")
    return final

cdef FluidMidi insert_nof(FluidMidi table):
    cdef np.ndarray arr = table.arr
    cdef np.ndarray allbutnofs = arr[arr['mtype'] != Mtype.NOF]
    cdef np.ndarray nofs = allbutnofs[allbutnofs['mtype'] == Mtype.NON]
    nofs['mtype'] = Mtype.NOF
    nofs['time'] = nofs['time'] + nofs['midi_d']
    nofs['notevel'] = _DEFAULT_NOTE_OFF_VEL
    nofs['midi_d'] = nofs['score_d'] = 0
    nofs['status'] = Mtype.EMPTY
    nofs['data'] = 0
    err = cmiditable_sort(ndarray_ptr(nofs), ndarray_size(nofs), SC_SORT_DEFAULT);
    if err != NO_ERR:
        raise Exception("Failed to sort eots")
    cdef FluidMidi final = FluidMidi(allbutnofs.size + nofs.size)
    err = cmiditable_sorted_insert(ndarray_ptr(final.arr), ndarray_ptr(allbutnofs), ndarray_size(allbutnofs), ndarray_ptr(nofs), ndarray_size(nofs))
    if err != NO_ERR:
        raise Exception("Failed to insert note offs into table")
    return final

def print_comparison(FluidMidi a, FluidMidi b):
    ctable_compare_print(ndarray_ptr(a.arr), ndarray_ptr(b.arr), a.size)

PROGRAM_CHANGES: dict = None
PROGRAM_CHANGESi: list = None

def __get_program_change():
    global PROGRAM_CHANGES, PROGRAM_CHANGESi
    cdef str name
    if PROGRAM_CHANGES is None or PROGRAM_CHANGESi is None:
        PROGRAM_CHANGES = {}
        PROGRAM_CHANGESi = []
        for i in range(128):
            name = <str> program_changes[i].decode("utf-8")
            PROGRAM_CHANGES[name] = i
            PROGRAM_CHANGESi.append(name)

__get_program_change()

CONTROL_CHANGES: dict = None
CONTROL_CHANGESi: list = None

def __get_control_change():
    global CONTROL_CHANGES, CONTROL_CHANGESi
    cdef str name
    if CONTROL_CHANGES is None or CONTROL_CHANGESi is None:
        CONTROL_CHANGES = {}
        CONTROL_CHANGESi = []
        for i in range(128):
            name = <str> control_changes[i].decode('utf-8')
            CONTROL_CHANGES[name] = i
            CONTROL_CHANGESi.append(name)

__get_control_change()
