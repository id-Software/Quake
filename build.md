## Build details

The native target requires 32-bit SPARC Solaris. Development and hardware
testing use Solaris 2.6 and the [tgcware](https://jupiterrise.com/tgcware/)
GNU utilities. The current target is the single-SM61 system described above;
the project's envisaged larger system has two SM61 processors and 128 MB RAM.

Run all `Makefile.XGL` and `Makefile.Solaris` commands in this document from
`WinQuake/`.

The optional dispatcher `make -f Makefile.Solaris xglquake` runs the same
build. Existing software, XIL and OpenGL recipes are independent. Relative to
the repository root, XGL objects are kept in `WinQuake/build/xgl` and the
executable is `WinQuake/xglquake`. All engine objects in this target are
compiled with `XGLQUAKE`, never `GLQUAKE`.

Run the same make command after updating sources; no `clean` is needed for
normal source or header changes. GCC writes a `.d` file beside each object
to track the headers that source actually includes. Objects from older
builds without `.d` files rebuild once to establish these dependencies.
Editing `Makefile.XGL` alone does not force a rebuild. After changing
compiler options, run `make -f Makefile.XGL clean` before rebuilding;
dependency files do not track option changes. `clean` also removes the
generated dependency files.

The SDK paths default to `/opt/SUNWits/Graphics-sw/xgl` and `/usr/openwin`.
Override them if necessary:

```sh
make -f Makefile.XGL XGLHOME=/path/to/xgl OPENWINHOME=/path/to/openwin
```

The target uses the native compiler's 32-bit SPARC default. Use a compiler
configured for that ABI; a 64-bit build is not supported by the original
engine or the XGL renderer. The build enables `-ffast-math`, matching the
original Solaris release recipe, and omits `-ansi`. `CC`, `CPPFLAGS`, `CFLAGS`,
`LANGFLAGS`, `WARNFLAGS`, `LDFLAGS` and `XGL_LIBS` can be overridden on the
command line.

When updating from a build without `-ffast-math`, run
`make -f Makefile.XGL clean` once before rebuilding so every object uses the
new flags. Host regression tests also enable `-ffast-math` by default.

