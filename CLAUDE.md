# Foreshock

Foreshock is a Quake (1996) source port targeting **Bazzite**, the Fedora
Atomic-based gaming distro, including handhelds such as the Steam Deck. It is
forked from id Software's original GPL source release.

## Current state

The code is still the unmodified 1999 id release. It targets DOS, Windows
(Visual C++ 6 + MASM), Solaris and old Linux (svgalib, X11, GLX, OSS sound),
and does not build on a modern system yet. Expect 32-bit assumptions
throughout, such as pointers cast to `int`.

Update this section as milestones land.

## Repository layout

- `WinQuake/` — the single-player/NetQuake engine (WinQuake software renderer
  and GLQuake). **This is the code Foreshock is built from.**
  - `gl_*.c` — OpenGL renderer; `r_*.c`, `d_*.c` — software renderer
  - `sys_*`, `vid_*`, `in_*`, `snd_*`, `cd_*`, `net_*` — platform layers, one
    file per OS or API
  - `*.s` — x86 assembly, used only when `id386` is set
  - `kit/`, `cwsdpmi.exe`, `dxsdk/`, `scitech/` — leftover Windows/DOS bits
- `QW/` — QuakeWorld client and server. Not a target for now; leave it alone
  unless asked.
- `qw-qc/` — QuakeWorld QuakeC game code.
- `scripts/` — Foreshock helper scripts.

## Goals and roadmap

1. Build the GL renderer on 64-bit Linux with a modern build system, C only
   (`id386 0`, no assembly).
2. Replace the platform layers with SDL2: video, input, audio and controllers.
3. Fix 64-bit and undefined-behaviour bugs found along the way.
4. Bazzite polish: gamescope-friendly fullscreen, SDL GameController support
   that works with Steam Input, and sensible defaults for handheld screens.
5. Flatpak packaging (app ID `io.github.VibeCoder671.Foreshock`) and GitHub
   Actions CI that builds every push.

## Development environment

Bazzite's host is read-only, so build inside the Distrobox container:

```sh
./scripts/setup-distrobox.sh   # one-time setup
distrobox enter foreshock-dev
```

Build and run commands: *none yet*. Add them here once the new build system
exists.

## Rules

- **Never commit Quake game data** (`id1/`, `*.pak`, and so on). The data is
  still copyrighted; `.gitignore` covers it. Users supply their own copy.
- **License:** the code is GPL-2.0 (see `gnu.txt`). All contributions must be
  compatible with it.
- **Stay compatible with the original game:** keep the network protocol,
  demo playback, save games and QuakeC behaviour working unless a change is
  explicitly agreed. Physics depend on frame timing, so don't change it
  casually.
- Don't use "Quake" in the product name or branding; id Software owns the
  trademark. Describing it as "a Quake source port" is fine.
- Prefer removing dead platform code (DOS, Win32, Solaris, svgalib) in its own
  commits, separate from functional changes, so history stays reviewable.
- Keep changes small and focused, one subsystem at a time.
- Testing needs a real game copy and a display, which automated sessions don't
  have. Say what was and wasn't verified, and leave playtesting to the
  maintainer.
