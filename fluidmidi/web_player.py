# midi_player.py
import base64
import uuid
from IPython.display import HTML, display

_SCRIPT_URL = (
    "https://cdn.jsdelivr.net/combine/"
    "npm/tone@14.7.58,"
    "npm/@magenta/music@1.23.1/es6/core.js,"
    "npm/html-midi-player@1.5.0"
)

SOUNDFONTS = {
    "sgm_plus":   "https://storage.googleapis.com/magentadata/js/soundfonts/sgm_plus",
    "salamander": "https://storage.googleapis.com/magentadata/js/soundfonts/salamander",
    "music_box":  "https://storage.googleapis.com/magentadata/js/soundfonts/music_box",
    "none":       None,
}

# Injected into document.head so ::part() actually reaches the shadow DOM.
# Cell-output <style> tags are scoped and cannot pierce ::part() in JupyterLab.
_HEAD_STYLE = """
midi-player::part(control-panel) {
    background: #2b2d3e;
    border-radius: 8px;
    padding: 4px 8px;
}
midi-player::part(play-button) {
    color: #cdd6f4;
    background: #45475a;
    border-radius: 50%;
    border: none;
    cursor: pointer;
    width: 32px;
    height: 32px;
    font-size: 16px;
    display: flex;
    align-items: center;
    justify-content: center;
}
midi-player::part(play-button):hover {
    background: #585b70;
}
midi-player::part(seek-bar) {
    accent-color: #89b4fa;
    flex: 1;
}
midi-player::part(current-time),
midi-player::part(total-time) {
    color: #cdd6f4;
    font-family: monospace;
    font-size: 12px;
}
midi-player::part(loading-overlay) {
    background: #45475a;
    border-radius: 8px;
}
"""


def load_web_midi_player():
    """
    Inject the script bundle and global styles into document.head.
    Call once at the top of your notebook.
    """
    display(HTML(f"""
    <script>
    (function() {{
        // Script — guard against double-load
        if (!window._htmlMidiPlayerLoaded) {{
            window._htmlMidiPlayerLoaded = true;
            var s = document.createElement('script');
            s.src = '{_SCRIPT_URL}';
            document.head.appendChild(s);
        }}

        // Styles — must be in document.head for ::part() to work in JupyterLab
        if (!window._htmlMidiPlayerStyled) {{
            window._htmlMidiPlayerStyled = true;
            var style = document.createElement('style');
            style.textContent = `{_HEAD_STYLE}`;
            document.head.appendChild(style);
        }}
    }})();
    </script>
    <p style="color:gray;font-size:11px;margin:2px 0;">
        ✓ html-midi-player ready
    </p>
    """))


def show_midi(
    midi_bytes: bytes,
    soundfont: str = "sgm_plus",
    visualizer: bool = True,
    loop: bool = False,
    player_id: str | None = None,
    viz_height: int = 200,
) -> str:
    """
    Render an html-midi-player in the current output cell.

    Returns player_id so you can call change_soundfont() later.
    """
    if player_id is None:
        player_id = "mp-" + uuid.uuid4().hex[:8]
    viz_id = player_id + "-viz"

    b64 = base64.b64encode(midi_bytes).decode("ascii")
    data_uri = f"data:audio/midi;base64,{b64}"

    sf_url = SOUNDFONTS.get(soundfont, soundfont)
    sf_attr = "" if sf_url is None else (
        "sound-font" if sf_url == "" else f'sound-font="{sf_url}"'
    )
    loop_attr = "loop" if loop else ""
    viz_ref   = f'visualizer="#{viz_id}"' if visualizer else ""

    # The wrapper div is appended to the *light DOM* of <midi-visualizer>
    # by the element's connectedCallback. We size the element itself, and
    # also set overflow:auto so the wide SVG (fixed px width from Magenta)
    # scrolls rather than overflows the notebook.
    viz_block = f"""
        <div class="piano-roll-visualizer"
             style="width:100%;height:{viz_height}px;
                    background:#1e1e2e;border-radius:0 0 8px 8px;
                    border:1px solid #45475a;border-top:none;
                    overflow:auto;">
          <midi-visualizer
              type="piano-roll"
              id="{viz_id}"
              style="display:block;">
          </midi-visualizer>
        </div>
    """ if visualizer else ""

    html = f"""
    <div style="max-width:960px;margin:8px 0;font-family:sans-serif;">
      <midi-player
          id="{player_id}"
          {sf_attr}
          {loop_attr}
          {viz_ref}
          style="display:block;width:100%;">
      </midi-player>
      {viz_block}
    </div>

    <script>
    (function() {{
        var dataUri  = "{data_uri}";
        var pid      = "{player_id}";
        var vizId    = "{viz_id}";
        var hasViz   = {"true" if visualizer else "false"};

        function init() {{
            var player = document.getElementById(pid);
            if (!player || !customElements.get('midi-player')) {{
                requestAnimationFrame(init);
                return;
            }}

            player.src = dataUri;

            if (hasViz) {{
                // After the player parses the MIDI, copy the noteSequence to
                // the visualizer so the full piano roll is visible before playback.
                // The player only calls viz.redraw() per-note during play; it does
                // not paint the full roll at load time.
                player.addEventListener('load', function() {{
                    var viz = document.getElementById(vizId);
                    if (viz) {{
                        viz.noteSequence = player.noteSequence;
                    }}
                }}, {{ once: true }});
            }}
        }}

        requestAnimationFrame(init);
    }})();
    </script>
    """

    display(HTML(html))
    return player_id


def change_soundfont(soundfont: str, player_id: str):
    """Hot-swap soundfont on an already-rendered player."""
    sf_url = SOUNDFONTS.get(soundfont, soundfont)
    value  = "null" if sf_url is None else f'"{sf_url}"'
    display(HTML(f"""
    <script>
    (function() {{
        var el = document.getElementById("{player_id}");
        if (el) el.soundFont = {value};
        else console.warn("midi-player not found:", "{player_id}");
    }})();
    </script>
    """))

from fluidmidi import *

def web_fluidmidi_player(
    table: FluidMidi,
    soundfont: str = "sgm_plus",
    visualizer: bool = False,
    loop: bool = False,
    player_id: str = "midi-player",
    height: int = 400,
):
    mb = midibytes_save_0(table).get_bytes()
    show_midi(mb, soundfont, visualizer, loop, player_id, height)
