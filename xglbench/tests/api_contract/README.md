# Native syntax-check contracts

These deliberately incomplete, handwritten declarations let a host compiler
parse `xglbench_xgl.c` when Sun XGL and X11 development headers are unavailable.
They are **not SDK headers, not an ABI specification, and not an emulator**.
Enumeration values and opaque object representations are placeholders. Code
using these declarations must never link to a real XGL library. The build's
normal include paths never use this directory.

The XGL declarations used by the benchmark were checked against the supplied
Sun XGL Reference Manual 802-5864. The `xgl_open` declaration also follows the
provided SDK header: its first argument is `Xgl_attribute`, followed by a
variadic attribute list. This check catches C syntax errors, missing
declarations, misspelled fields and wrong fixed-argument types or counts. It
cannot verify the real SDK, library linkage, varargs attribute types, runtime behavior,
rendering correctness or hardware acceleration.

From `xglbench/`, a GCC host can run:

```sh
cc -pedantic -Wall -Wextra -Werror -D__EXTENSIONS__ \
   -Itests/api_contract -I. -fsyntax-only xglbench_xgl.c
```

`make syntax-check` runs the same check and accepts an optional `LANGFLAGS`
override. No strict ANSI compiler mode is enabled by default.

On a modern host with a GNU-compatible linker, `make native-contract-check`
also runs targeted backend regression checks against mock XGL functions. The
linker discards unused backend functions; no XGL library or graphics device is
used. This target is separate from the Solaris build and portable `make check`.
It verifies selected resource and error-handling contracts, not rendering or
device support.

Compilation against the installed Solaris XGL SDK remains mandatory.
