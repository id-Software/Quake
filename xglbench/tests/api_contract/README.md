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

The texture-raster tests check creation-time RGB ordering, resource cleanup
and retry, readback errors, and requested native-array allocation accounting
(including geometry and stream-slot teardown). The native UI tests inspect
the separate 2D context, stroke-font lookup, polygon/text submission, buffer
selection, synchronization order, and error handling. UI memory-raster
creation and buffer-copy calls are rejected by these mocks. They do not
implement stroke-font rendering or establish text legibility on the device.
Measured text bounds include proportional widths, negative bearings and
descenders. A simple per-buffer paint-region model exercises the UI readback
gate: missing ink, missing panels, drawing to the wrong buffer and API readback
errors must fail without caching success, and saved context/raster state must
be restored. Recoverable primitive and completion errors also check that
pending UI work is completed before later main-context work, clear or swap.
The clear/transparency tests use a scalar
pixel and depth model for the backend's centered flat-triangle probes. They
check that missing color/depth clears, reversed opacity, incorrect blending,
broken depth freeze/write or testing, and native readback errors reject the
probe. They also check state restoration. This small model does not implement
XGL rasterization, lighting, quantization, or hardware execution. Passing it
does not establish native correctness or acceleration; real XGL readback gates
still run before timings are accepted. New-frame action constants use
independent bits so the mock can distinguish color and depth clearing.

Compilation against the installed Solaris XGL SDK remains mandatory.
