"""
midi_visualizer.py
──────────────────
A piano-roll MIDI visualizer for Jupyter notebooks.

Usage
-----
    from midi_visualizer import visualize_midi

    visualize_midi(
        starts,      # np.ndarray, float  – note-on times in seconds
        values,      # np.ndarray, int    – MIDI note numbers 0-127
        durations,   # np.ndarray, float  – note durations in seconds
        velocities,  # np.ndarray, int    – MIDI velocities 0-127
        channels,    # np.ndarray, int    – MIDI channel numbers (0- or 1-based)
        meta=None,   # dict | None        – reserved for tempo/time-sig data (see below)
    )

meta format (future use)
------------------------
    meta = {
        "tempo_map":        [(time_sec, bpm), ...],          # sorted by time
        "time_signatures":  [(time_sec, numerator, denom), ...],
        "markers":          [(time_sec, label), ...],
    }
"""

from __future__ import annotations

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.collections import PatchCollection
import matplotlib.ticker as ticker
import ipywidgets as widgets
from IPython.display import display
from fluidmidi import Mtype

# ──────────────────────────────────────────────────────────────────────────────
# Constants
# ──────────────────────────────────────────────────────────────────────────────

NOTE_NAMES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]

# Distinct, colourblind-friendly channel palette (up to 16 channels)
CHANNEL_PALETTE = [
    "#4e9af1",  # blue
    "#f17c4e",  # orange
    "#4ef1a0",  # mint
    "#f14e7c",  # rose
    "#c44ef1",  # violet
    "#f1d34e",  # gold
    "#4ef1e8",  # cyan
    "#f14ec4",  # magenta
    "#8af14e",  # lime
    "#f1874e",  # coral
    "#4e7cf1",  # indigo
    "#f14e4e",  # red
    "#4ef168",  # green
    "#e8f14e",  # yellow
    "#4ec4f1",  # sky
    "#a04ef1",  # purple
]

NOTE_HEIGHT = 0.8   # fraction of semitone slot occupied by each note rect
FIGURE_SIZE = (14, 6)


# ──────────────────────────────────────────────────────────────────────────────
# Helpers
# ──────────────────────────────────────────────────────────────────────────────

def _note_name(midi_number: int) -> str:
    octave = midi_number // 12 - 1
    name = NOTE_NAMES[midi_number % 12]
    return f"{name}{octave}"


def _channel_color(channel: int, unique_channels: list[int]) -> str:
    idx = unique_channels.index(channel) % len(CHANNEL_PALETTE)
    return CHANNEL_PALETTE[idx]


# ──────────────────────────────────────────────────────────────────────────────
# Time annotation layer  (stub – extend this when meta is available)
# ──────────────────────────────────────────────────────────────────────────────

def _draw_time_annotations(ax: plt.Axes, meta: dict | None, x_min: float, x_max: float) -> None:
    """
    Draw time ruler / bar markers on *ax* for the visible range [x_min, x_max].

    This function is called every time the horizontal viewport changes, so
    all artists it creates must be ephemeral — either removed before the next
    call or drawn on a dedicated twin axis that is cleared beforehand.

    Current behaviour: draws plain second ticks (always available).
    Future: when meta carries tempo_map + time_signatures, replace the second-
    tick block with beat/bar lines and measure numbers.

    Parameters
    ----------
    ax   : the main piano-roll axes
    meta : dict with keys tempo_map, time_signatures, markers  — or None
    x_min, x_max : currently visible time range in seconds
    """

    # Remove previously drawn annotation artists tagged with our marker
    for artist in ax.get_lines() + ax.texts:
        if getattr(artist, "_midi_time_annotation", False):
            artist.remove()

    visible_span = x_max - x_min

    # ── Plain second grid (always present) ────────────────────────────────────
    # Choose a sensible tick interval based on zoom level
    if visible_span <= 2:
        interval = 0.25
    elif visible_span <= 10:
        interval = 1.0
    elif visible_span <= 60:
        interval = 5.0
    elif visible_span <= 300:
        interval = 30.0
    else:
        interval = 60.0

    first_tick = np.ceil(x_min / interval) * interval
    ticks = np.arange(first_tick, x_max + interval * 0.01, interval)

    for t in ticks:
        line = ax.axvline(t, color="#ffffff", alpha=0.08, linewidth=0.6, zorder=1)
        line._midi_time_annotation = True
        label_str = f"{t:.2f}s" if interval < 1 else f"{int(t)}s"
        txt = ax.text(
            t + visible_span * 0.003, ax.get_ylim()[1] - 0.8,
            label_str,
            color="#aaaaaa", fontsize=7, va="top", zorder=5,
            clip_on=True,
        )
        txt._midi_time_annotation = True

    # ── Future: bar / beat markers ────────────────────────────────────────────
    # if meta is not None:
    #     _draw_bar_markers(ax, meta, x_min, x_max)
    #     _draw_tempo_markers(ax, meta, x_min, x_max)
    #     _draw_cue_markers(ax, meta, x_min, x_max)


# ──────────────────────────────────────────────────────────────────────────────
# Core renderer
# ──────────────────────────────────────────────────────────────────────────────

def _build_figure(
    starts: np.ndarray,
    values: np.ndarray,
    durations: np.ndarray,
    velocities: np.ndarray,
    channels: np.ndarray,
    meta: dict | None,
) -> tuple[plt.Figure, plt.Axes]:
    """Create the figure and draw all note patches as a PatchCollection."""

    unique_channels = sorted(set(channels.tolist()))
    color_map = {ch: _channel_color(ch, unique_channels) for ch in unique_channels}

    fig, ax = plt.subplots(figsize=FIGURE_SIZE, facecolor="#1a1a2e")
    ax.set_facecolor("#16213e")

    # ── Draw notes ────────────────────────────────────────────────────────────
    # Group by channel so each channel can be a single PatchCollection
    for ch in unique_channels:
        mask = channels == ch
        ch_starts = starts[mask]
        ch_values = values[mask]
        ch_durations = durations[mask]
        ch_velocities = velocities[mask]

        patches = []
        alphas = []

        for s, v, d, vel in zip(ch_starts, ch_values, ch_durations, ch_velocities):
            rect = mpatches.FancyBboxPatch(
                (s, v - NOTE_HEIGHT / 2),
                width=max(d, 0.01),          # guard against zero-dur notes
                height=NOTE_HEIGHT,
                boxstyle="round,pad=0.01",
                linewidth=0,
            )
            patches.append(rect)
            alphas.append(np.clip(vel / 127.0, 0.15, 1.0))

        if not patches:
            continue

        col = PatchCollection(patches, facecolor=color_map[ch], linewidth=0, zorder=3)
        col.set_alpha(None)          # let individual face-alphas (via array) work
        # matplotlib PatchCollection doesn't support per-patch alpha natively,
        # so we encode velocity into the alpha channel of the RGBA facecolors.
        import matplotlib.colors as mcolors
        base_rgb = mcolors.to_rgb(color_map[ch])
        rgba = np.array([[*base_rgb, a] for a in alphas])
        col.set_facecolor(rgba)
        ax.add_collection(col)

    # ── Octave grid lines ─────────────────────────────────────────────────────
    for midi in range(0, 128, 12):   # C notes
        ax.axhline(midi, color="#ffffff", alpha=0.06, linewidth=0.5, zorder=1)

    # ── Y axis: note labels ───────────────────────────────────────────────────
    ax.yaxis.set_major_locator(ticker.MultipleLocator(12))
    ax.yaxis.set_minor_locator(ticker.MultipleLocator(1))
    ax.yaxis.set_major_formatter(
        ticker.FuncFormatter(lambda y, _: _note_name(int(y)) if 0 <= y <= 127 else "")
    )
    ax.tick_params(axis="y", which="major", colors="#cccccc", labelsize=8)
    ax.tick_params(axis="y", which="minor", colors="#555555", length=2)
    ax.tick_params(axis="x", colors="#888888", labelsize=8)

    for spine in ax.spines.values():
        spine.set_edgecolor("#333355")

    # ── Legend ────────────────────────────────────────────────────────────────
    legend_handles = [
        mpatches.Patch(facecolor=color_map[ch], label=f"Ch {ch}")
        for ch in unique_channels
    ]
    ax.legend(
        handles=legend_handles,
        loc="upper right",
        fontsize=8,
        framealpha=0.4,
        facecolor="#1a1a2e",
        edgecolor="#333355",
        labelcolor="#cccccc",
    )

    ax.set_xlabel("Time (s)", color="#888888", fontsize=9)
    ax.set_ylabel("Note", color="#888888", fontsize=9)

    return fig, ax


# ──────────────────────────────────────────────────────────────────────────────
# Public entry point
# ──────────────────────────────────────────────────────────────────────────────

def visualize_midi(
    starts: np.ndarray,
    values: np.ndarray,
    durations: np.ndarray,
    velocities: np.ndarray,
    channels: np.ndarray,
    meta: dict | None = None,
) -> None:
    """
    Display an interactive piano-roll visualizer in a Jupyter notebook.

    Parameters
    ----------
    starts     : note-on times in seconds
    values     : MIDI note numbers (0–127)
    durations  : note durations in seconds
    velocities : MIDI velocities (0–127)
    channels   : MIDI channel numbers
    meta       : optional dict for tempo map / time signatures / markers
    """

    # ── Validate & normalise ──────────────────────────────────────────────────
    starts     = np.asarray(starts,     dtype=float)
    values     = np.asarray(values,     dtype=int)
    durations  = np.asarray(durations,  dtype=float)
    velocities = np.asarray(velocities, dtype=int)
    channels   = np.asarray(channels,   dtype=int)

    if not (len(starts) == len(values) == len(durations) == len(velocities) == len(channels)):
        raise ValueError("All arrays must have the same length.")

    if len(starts) == 0:
        print("No notes to display.")
        return

    # ── Compute full extents ──────────────────────────────────────────────────
    t_start_full  = float(starts.min())
    t_end_full    = float((starts + durations).max())
    note_min_full = int(values.min())
    note_max_full = int(values.max())

    t_pad_full  = (t_end_full - t_start_full) * 0.02 + 0.1
    n_pad_full  = max((note_max_full - note_min_full) * 0.05, 2)

    X_MIN_FULL = t_start_full - t_pad_full
    X_MAX_FULL = t_end_full   + t_pad_full
    Y_MIN_FULL = max(note_min_full - n_pad_full, -0.5)
    Y_MAX_FULL = min(note_max_full + n_pad_full, 127.5)

    total_time  = X_MAX_FULL - X_MIN_FULL
    total_notes = Y_MAX_FULL - Y_MIN_FULL

    # ── Build figure ──────────────────────────────────────────────────────────
    fig, ax = _build_figure(starts, values, durations, velocities, channels, meta)
    ax.set_xlim(X_MIN_FULL, X_MAX_FULL)
    ax.set_ylim(Y_MIN_FULL, Y_MAX_FULL)
    _draw_time_annotations(ax, meta, X_MIN_FULL, X_MAX_FULL)
    plt.tight_layout()

    # ── Widget state ──────────────────────────────────────────────────────────
    state = {
        "h_zoom": 1.0,   # fraction of total_time visible  (1.0 = all)
        "v_zoom": 1.0,   # fraction of total_notes visible (1.0 = all)
        "h_pan":  0.0,   # 0.0 = leftmost, 1.0 = rightmost
        "v_pan":  0.0,   # 0.0 = bottom,   1.0 = top
    }

    def _current_xlim():
        visible = total_time * state["h_zoom"]
        x_lo = X_MIN_FULL + state["h_pan"] * (total_time - visible)
        return x_lo, x_lo + visible

    def _current_ylim():
        visible = total_notes * state["v_zoom"]
        y_lo = Y_MIN_FULL + state["v_pan"] * (total_notes - visible)
        return y_lo, y_lo + visible

    def _redraw(_=None):
        xlo, xhi = _current_xlim()
        ylo, yhi = _current_ylim()
        ax.set_xlim(xlo, xhi)
        ax.set_ylim(ylo, yhi)
        _draw_time_annotations(ax, meta, xlo, xhi)
        fig.canvas.draw_idle()

    # ── Widgets ───────────────────────────────────────────────────────────────
    style  = {"description_width": "90px"}
    layout = widgets.Layout(width="520px")

    w_hzoom = widgets.FloatSlider(
        value=1.0, min=0.02, max=1.0, step=0.01,
        description="H Zoom",
        style=style, layout=layout,
        continuous_update=True, readout=False,
    )
    w_vzoom = widgets.FloatSlider(
        value=1.0, min=0.05, max=1.0, step=0.01,
        description="V Zoom",
        style=style, layout=layout,
        continuous_update=True, readout=False,
    )
    w_hpan = widgets.FloatSlider(
        value=0.0, min=0.0, max=1.0, step=0.005,
        description="H Pan",
        style=style, layout=layout,
        continuous_update=True, readout=False,
    )
    w_vpan = widgets.FloatSlider(
        value=0.0, min=0.0, max=1.0, step=0.005,
        description="V Pan",
        style=style, layout=layout,
        continuous_update=True, readout=False,
    )
    w_reset = widgets.Button(
        description="Reset View",
        button_style="",
        layout=widgets.Layout(width="100px"),
        style={"button_color": "#2a2a4a"},
    )

    def _on_hzoom(change):
        state["h_zoom"] = change["new"]
        # clamp pan so view doesn't exceed bounds
        state["h_pan"] = np.clip(state["h_pan"], 0, 1)
        _redraw()

    def _on_vzoom(change):
        state["v_zoom"] = change["new"]
        state["v_pan"] = np.clip(state["v_pan"], 0, 1)
        _redraw()

    def _on_hpan(change):
        state["h_pan"] = change["new"]
        _redraw()

    def _on_vpan(change):
        state["v_pan"] = change["new"]
        _redraw()

    def _on_reset(_):
        state["h_zoom"] = 1.0
        state["v_zoom"] = 1.0
        state["h_pan"]  = 0.0
        state["v_pan"]  = 0.0
        w_hzoom.value = 1.0
        w_vzoom.value = 1.0
        w_hpan.value  = 0.0
        w_vpan.value  = 0.0
        _redraw()

    w_hzoom.observe(_on_hzoom, names="value")
    w_vzoom.observe(_on_vzoom, names="value")
    w_hpan.observe (_on_hpan,  names="value")
    w_vpan.observe (_on_vpan,  names="value")
    w_reset.on_click(_on_reset)

    controls = widgets.VBox([
        widgets.HBox([w_hzoom, w_hpan]),
        widgets.HBox([w_vzoom, w_vpan]),
        widgets.HBox([w_reset]),
    ])

    plt.show()        # embeds the live ipympl canvas into the cell output
    display(controls) # controls sit below the live figure

def visualize_fluidmidi(table):
    notes = table[table['mtype'] == Mtype.NON]
    visualize_midi(notes['time'], notes['noteval'], notes['midi_d'], notes['notevel'], notes['channel'])
