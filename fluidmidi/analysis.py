from fluidmidi import FluidMidi, Mtype, tempo2bpm
import numpy as np

def fluidmidi_get_tempo_changes(table: FluidMidi):
    tempo_table = table[table['mtype'] == Mtype.TEMPO]
    tempo = np.zeros(shape=(tempo_table.size, 2), dtype=float)
    tempo[:, 0] = tempo_table['time']
    calc_bpm = np.vectorize(tempo2bpm)
    tempo[:, 1] = calc_bpm(tempo_table['pitch'])
    return tempo

def fluidmidi_get_timesig_changes(table: FluidMidi):
    tsig_table = table[table['mtype'] == Mtype.TSIG]
    tsig = np.vstack((tsig_table['time'], tsig_table['program'], tsig_table['control']))
    return tsig.T
