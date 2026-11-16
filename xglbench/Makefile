# Standalone XGL benchmark; GNU make, as used by WinQuake/Makefile.Solaris.
# No engine objects or game assets are required.

CC = gcc
CFLAGS ?= -O2
CPPFLAGS ?=
LANGFLAGS ?=
WARNFLAGS ?= -Wall -W -Wstrict-prototypes
LDFLAGS ?=

XGLHOME ?= /opt/SUNWits/Graphics-sw/xgl
OPENWINHOME ?= /usr/openwin
API_CPPFLAGS = -D__EXTENSIONS__
XGL_CPPFLAGS = -I$(XGLHOME)/include -I$(OPENWINHOME)/include
XGL_LDFLAGS = -L$(XGLHOME)/lib -L$(OPENWINHOME)/lib
XGL_LIBS ?= -lxgl -lX11 -lXext -ldga -ldl -lm

ifeq ($(shell uname -s),SunOS)
XGL_RPATH ?= -R$(XGLHOME)/lib:$(OPENWINHOME)/lib
else
XGL_RPATH ?=
endif

COMMON = xglbench.c xglbench_scene.c xglbench_lighting.c xglbench_timer.c xglbench_watchdog.c
NATIVE_OBJS = build/xglbench.o build/xglbench_scene.o \
              build/xglbench_lighting.o build/xglbench_timer.o build/xglbench_watchdog.o build/xglbench_xgl.o

.PHONY: all native sdk-check check syntax-check native-contract-check clean
all: native
native: sdk-check
	$(MAKE) xglbench

sdk-check:
	@test -f "$(XGLHOME)/include/xgl/xgl.h" || { \
	    echo "XGL SDK header missing: $(XGLHOME)/include/xgl/xgl.h" >&2; \
	    echo "Set XGLHOME to the Solaris XGL SDK. Use 'make check' for portable tests." >&2; \
	    exit 1; }
	@test -f "$(OPENWINHOME)/include/X11/Xlib.h" || { \
	    echo "X11 header missing: $(OPENWINHOME)/include/X11/Xlib.h" >&2; \
	    echo "Set OPENWINHOME to the OpenWindows development installation." >&2; \
	    exit 1; }

build/%.o: %.c xglbench.h
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(API_CPPFLAGS) $(XGL_CPPFLAGS) $(CFLAGS) $(LANGFLAGS) $(WARNFLAGS) -c $< -o $@

xglbench: $(NATIVE_OBJS)
	$(CC) $(LDFLAGS) $(XGL_LDFLAGS) $(XGL_RPATH) -o $@ $(NATIVE_OBJS) $(XGL_LIBS)

build/test_scene: tests/test_scene.c xglbench_scene.c xglbench.h
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(API_CPPFLAGS) -I. $(CFLAGS) $(LANGFLAGS) $(WARNFLAGS) \
	    -o $@ tests/test_scene.c xglbench_scene.c -lm

build/test_timer: tests/test_timer.c xglbench_timer.c xglbench.h
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(API_CPPFLAGS) -I. $(CFLAGS) $(LANGFLAGS) $(WARNFLAGS) \
	    -o $@ tests/test_timer.c xglbench_timer.c -lm

build/test_lighting: tests/test_lighting.c xglbench_lighting.c xglbench_scene.c xglbench.h
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(API_CPPFLAGS) -I. $(CFLAGS) $(LANGFLAGS) $(WARNFLAGS) \
	    -o $@ tests/test_lighting.c xglbench_lighting.c xglbench_scene.c -lm

build/test_watchdog: tests/test_watchdog.c xglbench_watchdog.c xglbench.h
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(API_CPPFLAGS) -I. $(CFLAGS) $(LANGFLAGS) $(WARNFLAGS) \
	    -o $@ tests/test_watchdog.c xglbench_watchdog.c

# This executable tests scheduling and reports, not graphics performance.
build/xglbench-test: $(COMMON) tests/test_backend.c xglbench.h
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(API_CPPFLAGS) -I. $(CFLAGS) $(LANGFLAGS) $(WARNFLAGS) \
	    -o $@ $(COMMON) tests/test_backend.c -lm

# A simulated clock exercises minute-scale profiles without long test waits.
build/xglbench-profile-test: $(COMMON) tests/test_backend.c tests/test_profile_clock.c xglbench.h
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(API_CPPFLAGS) -DXB_PROFILE_TEST_CLOCK -I. $(CFLAGS) $(LANGFLAGS) $(WARNFLAGS) \
	    -o $@ $(filter-out xglbench_timer.c,$(COMMON)) tests/test_backend.c tests/test_profile_clock.c -lm

check: build/test_scene build/test_timer build/test_lighting build/test_watchdog build/xglbench-test build/xglbench-profile-test
	./build/test_scene
	./build/test_timer
	./build/test_lighting
	./build/test_watchdog
	sh tests/test_cli.sh ./build/xglbench-test
	sh tests/test_profiles.sh ./build/xglbench-profile-test

# Modern development host only. The contract headers are NOT the Sun SDK.
syntax-check:
	$(CC) $(CPPFLAGS) $(API_CPPFLAGS) $(LANGFLAGS) -pedantic -Wall -W -Werror \
	    -Itests/api_contract -I. -fsyntax-only xglbench_xgl.c

# Modern host with a GNU-compatible linker only; links mock XGL functions.
# Keep this separate from portable checks and the native Solaris build.
build/test_native_raster: tests/test_native_raster.c xglbench_xgl.c xglbench.h \
                         $(wildcard tests/api_contract/X11/*.h) tests/api_contract/xgl/xgl.h
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(API_CPPFLAGS) -Itests/api_contract -I. \
	    $(CFLAGS) $(LANGFLAGS) $(WARNFLAGS) -ffunction-sections -fdata-sections \
	    $(LDFLAGS) -o $@ tests/test_native_raster.c -Wl,--gc-sections -lm

native-contract-check: build/test_native_raster
	./build/test_native_raster

clean:
	rm -rf build xglbench
