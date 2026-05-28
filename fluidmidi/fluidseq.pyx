# cython: language_level=3
from fluidmidi.pyside.midi cimport FluidMidi, MidiRow, ndarray_ptr
from fluidmidi.pyside.midi import Mtype
from fluidmidi.pyside.cside cimport print_row, _DEFAULT_TICKS_PER_BEAT
from fsynth.lib cimport *

cdef extern from "cfsynth.h":
    cdef int fs_send_channel_message(fluid_synth_t *synth, const unsigned char *data, int length)

cdef extern from "synth_api.h":
    cdef int fs_send_fluidmidi_now(fluid_synth_t *fs, MidiRow *row, long num_rows)

def send_fluidmidi_messages(synth: Synthesizer, table: FluidMidi):
        cdef int err = fs_send_fluidmidi_now(synth.ptr, ndarray_ptr(table.arr), table.size)
        if err != FLUID_OK:
            raise Exception("Failed to send midi messages.")

cdef class FluidMidiSequence(SequencerClient):
    cdef MidiRow *_table
    cdef int tick
    cdef int row_idx
    cdef FluidMidi table
    cdef int adv, ret

    def __init__(self, sequencer: Sequencer, table: FluidMidi, advance: int, retreat: int):
        super().__init__(sequencer)
        self.tick = 0
        self.row_idx = 0
        self.table = table
        self._table = ndarray_ptr(self.table.arr)
        if table.size < 1:
            raise ValueError('Table has no rows to sequence.')

    cdef schedule_row(self, unsigned int at_tick, MidiRow *row):
        print_row(row)
        cdef int mtype = row.mtype
        if mtype == Mtype.NON:
            self._sequencer.send_note(
                at_tick,
                row.channel,
                row.noteval,
                row.notevel,
                row.midi_d
            )
        elif mtype == Mtype.CC:
            self._sequencer.send_control_change(
                at_tick,
                row.channel,
                row.control,
                row.cc_val
            )
        elif mtype == Mtype.PC:
            self._sequencer.send_program_change(
                at_tick,
                row.channel,
                row.program
            )

        elif mtype == Mtype.TEMPO:
            scale = _DEFAULT_TICKS_PER_BEAT * 1e6 / row.pitch
            self._sequencer.send_tempo_change(
                at_tick,
                scale
            )

        elif mtype in (Mtype.BEND0, Mtype.BEND1):
            self._sequencer.send_pitch_bend(
                at_tick,
                row.channel,
                row.pitch
            )

    def start(self):
        super().start()
        self.tick = self._sequencer.tick + 20
        print('start', self.tick)

    def callback(self, time: int, sequencer):
        cdef MidiRow *_row
        cdef int delta = 0
        cdef int advance = 0
        while True:
            _row = &self._table[self.row_idx]
            self.row_idx += 1
            delta = _row.delta
            advance += delta
            self.tick += advance
            if not self.muted:
                self.schedule_row(at_tick=self.tick, row=_row)
            if self.row_idx >= self.table.size:
                self.row_idx %= self.table.size
            if advance > self.adv:
                self._reschedule(self.tick - self.ret)
                break
