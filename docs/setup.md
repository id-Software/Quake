# Game data and display setup

For basic launch commands, see [game data and launch](../README.md#game-data-and-launch).

Run game commands from the directory containing `id1/`. The shareware
episode is the initial compatibility baseline. PAK recognition identifies
known file directories; it does not establish complete hardware validation
of every version.

Official Quake 1.00, 1.01 and 1.06 shareware PAK directories are recognized.
For a data directory elsewhere, use `-basedir /path/to/quake`, where that
directory contains `id1/pak0.pak`. Do not use `-game id1` or `-path` to select
the standard shareware data: the engine treats those options as modified
game paths. Startup reports the file count and directory CRC of each PAK
to help identify incompatible, repacked or extra files. Recognition of these
older official PAK versions is specific to the XGL target; registration and
modified-game checks remain enabled.

Use a local display with the Sun ZX's 24-bit TrueColor visual and DGA access.
The renderer requires hardware RGB, Z buffering, shading and two buffers.
`-display`, `-visualid`, `-width`, `-height`, `-winsize` and `-nomouse` select
the display/window/input settings. The video menu offers window sizes.
Original Quake console, save/load, demo, mouse sensitivity and key bindings
remain in the shared engine. The software-only `lcd_x` interlaced framebuffer
mode is not applied to XGL rendering.

## Troubleshooting

If a native XGL call fails, capture both output streams:

```sh
./xglquake -winsize 320 240 +map e1m1 > xglquake.log 2>&1
```

The diagnostic includes the rendering stage, XGL object and operands, error
type/category, and the source line where the error was checked. Some Solaris
installations return an empty error ID and only `Error number` as the message;
the remaining fields are retained to help identify the failing attribute.
Startup console drawing begins before sound initialization, so a graphics
failure at that point does not imply an audio failure.
