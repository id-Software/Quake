#!/usr/bin/env python3
"""Compile/link contract check; never executes mocked native graphics code."""

import argparse
import pathlib
import re
import shlex
import subprocess
import sys
import tempfile


def run(command, *, cwd, log):
    result = subprocess.run(command, cwd=cwd, text=True, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT)
    log.write("$ " + shlex.join(str(part) for part in command) + "\n")
    log.write(result.stdout)
    log.flush()
    if result.returncode:
        print(result.stdout[-12000:], file=sys.stderr)
        raise RuntimeError("command failed: " + shlex.join(str(part) for part in command))
    return result.stdout


def check_existing_paths(directory, compiler, includes, output, log):
    """Compare modified pre-existing engine files against their HEAD versions."""
    changed = run(["git", "diff", "HEAD", "--name-only", "--", "WinQuake"],
                  cwd=directory.parent, log=log).splitlines()
    changed = [path for path in changed
               if pathlib.PurePosixPath(path).parent == pathlib.PurePosixPath("WinQuake")
               and not pathlib.PurePosixPath(path).name.startswith("xgl_")
               and pathlib.PurePosixPath(path).suffix in (".c", ".h")]
    if not changed:
        print("Existing-source preprocessing: no modified C/header files to compare.")
        return
    baseline = output / "baseline"
    baseline.mkdir()
    baseline_includes = list(directory.glob("*.h")) + list(directory.glob("progdefs.q?"))
    for header in baseline_includes:
        original = subprocess.run(["git", "show", "HEAD:WinQuake/" + header.name],
                                  cwd=directory, stdout=subprocess.PIPE,
                                  stderr=subprocess.DEVNULL)
        if original.returncode == 0:
            (baseline / header.name).write_bytes(original.stdout)
    checked = 0
    for path in changed:
        filename = pathlib.PurePosixPath(path).name
        original = subprocess.run(["git", "show", "HEAD:" + path], cwd=directory,
                                  stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        if original.returncode:
            continue
        (baseline / filename).write_bytes(original.stdout)
        for mode in ([], ["-DGLQUAKE"]):
            base_command = compiler + ["-std=gnu89", "-E", "-P"] + mode + includes
            old = run(base_command + ["-I" + str(baseline), str(baseline / filename)],
                      cwd=directory, log=log)
            current = run(base_command + ["-I" + str(directory), str(directory / filename)],
                          cwd=directory, log=log)
            if old != current:
                mode_name = "GLQUAKE" if mode else "software"
                (output / (filename + "." + mode_name + ".before")).write_text(old)
                (output / (filename + "." + mode_name + ".after")).write_text(current)
                raise RuntimeError(filename + " changed preprocessing for " + mode_name)
            checked += 1
    print("Existing software/GL preprocessing unchanged in %d file/mode comparisons." % checked)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cc", default="cc")
    parser.add_argument("--sdk", required=True)
    parser.add_argument("--x11", default="",
                        help="optional real X11 include root, before host contracts")
    parser.add_argument("sources", nargs="+")
    args = parser.parse_args()
    directory = pathlib.Path(__file__).resolve().parent.parent
    contracts = directory / "xgl_tests" / "include"
    sdk_include = pathlib.Path(args.sdk).resolve() / "include"
    sdk_header = sdk_include / "xgl" / "xgl.h"
    if not sdk_header.is_file():
        parser.error("actual XGL SDK header not found: " + str(sdk_header))
    compiler = shlex.split(args.cc)
    x11_include = pathlib.Path(args.x11).resolve() if args.x11 else None
    if x11_include and not (x11_include / "X11" / "Xlib.h").is_file():
        parser.error("X11 include root has no X11/Xlib.h: " + str(x11_include))
    output = pathlib.Path(tempfile.mkdtemp(prefix="xglquake-host-"))
    print("Host compile/link artifacts: " + str(output))
    try:
        with (output / "check.log").open("w") as log:
            includes = ["-I" + str(sdk_include)]
            if x11_include:
                includes += ["-I" + str(x11_include)]
            includes += ["-I" + str(contracts)]
            print("X11 headers: " + (str(x11_include) if x11_include else "handwritten host contracts"))
            objects = []
            for source in args.sources:
                target = output / (pathlib.Path(source).stem + ".o")
                flags = ["-std=gnu89", "-O0", "-fcommon", "-fno-strict-aliasing",
                         "-DXGLQUAKE", "-D__EXTENSIONS__", "-D__sun__", "-pthread",
                         "-Wno-unknown-pragmas"]
                if pathlib.Path(source).name.startswith("xgl_"):
                    flags += ["-Wall", "-Wextra", "-Werror=implicit-function-declaration"]
                run(compiler + flags + includes + ["-I.", "-c", source, "-o", str(target)],
                    cwd=directory, log=log)
                objects.append(str(target))

            # Resolve all engine symbols together before supplying external
            # native API placeholders. Unknown engine/GL calls remain errors.
            combined = output / "xglquake-engine.o"
            run(compiler + ["-r", "-o", str(combined)] + objects, cwd=directory, log=log)
            undefined = run(["nm", "-u", str(combined)], cwd=directory, log=log)
            undefined = {line.split()[-1] for line in undefined.splitlines() if line.split()}
            sdk_text = "\n".join(path.read_text(errors="replace")
                                 for path in (sdk_include / "xgl").glob("*.h"))
            native_names = set(re.findall(r"\b(xgl_[a-zA-Z0-9_]+)\s*\(", sdk_text))
            contract_text = "\n".join(path.read_text()
                                      for path in (contracts / "X11").glob("*.h"))
            if x11_include:
                contract_text += "\n" + "\n".join(path.read_text(errors="replace")
                                                  for path in (x11_include / "X11").glob("*.h"))
            x_names = set(re.findall(r"\b(X[A-Z]\w*|DefaultScreen|RootWindow|DefaultDepth|"
                                     r"BlackPixel|WhitePixel|ConnectionNumber|DisplayString|"
                                     r"DisplayWidth|DisplayHeight|DefaultVisual)\s*\(",
                                     contract_text))
            external = sorted(undefined & (native_names | x_names))
            stub_source = output / "native_link_placeholders.c"
            stub_source.write_text("/* Link-only placeholders. Never execute. */\n"
                                   "#include <stdlib.h>\n" +
                                   "".join("void %s(void) { abort(); }\n" % name
                                           for name in external))
            executable = output / "xglquake-host-link-only"
            run(compiler + ["-o", str(executable), str(combined), str(stub_source), "-pthread", "-lm"],
                cwd=directory, log=log)
            print("Compiled and linked %d source files; %d native/X11 placeholders." %
                  (len(objects), len(external)))
            check_existing_paths(directory, compiler, includes, output, log)
    except (OSError, RuntimeError) as error:
        print("FAIL: " + str(error), file=sys.stderr)
        print("Details: " + str(output / "check.log"), file=sys.stderr)
        return 1
    print("PASS: host contract/link check only; no Solaris/SPARC/XGL runtime validation.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
