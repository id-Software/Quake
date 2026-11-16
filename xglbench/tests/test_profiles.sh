#!/bin/sh
# Virtual-time profile scheduling tests. Never sleeps for real profile minutes
# and never measures XGL performance. GPL-2.0-or-later; see COPYING.
set -eu

bench=${1:-./build/xglbench-profile-test}
test_dir=build/profile-test-$$
mkdir -p "$test_dir"
trap 'rm -rf "$test_dir"' 0
trap 'exit 1' 1 2 3 15

fail()
{
    echo "Profile test failed: $*" >&2
    if test -f "$test_dir/output"; then cat "$test_dir/output" >&2; fi
    exit 1
}

expect_fail()
{
    if "$bench" "$@" >"$test_dir/output" 2>&1; then
        fail "unexpected success: $*"
    fi
}

expect_fail -quick -medium -list
expect_fail -long -extralong -list
expect_fail -pause
expect_fail -pause -1
expect_fail -pause nan
expect_fail -pause inf
expect_fail -pause 1garbage

# Listing is read-only and selection is deterministic. Medium adds work to
# quick; long runs medium coverage at two sizes; extralong retains all cases.
XB_TEST_LOG="$test_dir/list.log" "$bench" -list >"$test_dir/default.list"
test ! -e "$test_dir/list.log" || fail "default listing opened the display"
for profile in quick medium long extralong; do
    "$bench" "-$profile" -list >"$test_dir/$profile.list"
    case "$profile" in quick) count=16 ;; medium|long) count=34 ;; *) count=237 ;; esac
    test "$(wc -l <"$test_dir/$profile.list" | tr -d ' ')" = "$count" ||
        fail "$profile selected the wrong number of cases"
    sort "$test_dir/$profile.list" >"$test_dir/$profile.sorted"
    uniq -d "$test_dir/$profile.sorted" >"$test_dir/$profile.duplicates"
    test ! -s "$test_dir/$profile.duplicates" || fail "$profile contains duplicate cases"
done
cmp "$test_dir/default.list" "$test_dir/quick.list" >/dev/null || fail "default is not quick"
cmp "$test_dir/medium.list" "$test_dir/long.list" >/dev/null || fail "long changed medium coverage"
comm -23 "$test_dir/quick.sorted" "$test_dir/medium.sorted" >"$test_dir/missing"
test ! -s "$test_dir/missing" || fail "medium dropped a quick comparison"
comm -23 "$test_dir/long.sorted" "$test_dir/extralong.sorted" >"$test_dir/missing"
test ! -s "$test_dir/missing" || fail "extralong dropped a long comparison"
"$bench" -quick -quick -list >"$test_dir/repeated.list"
cmp "$test_dir/repeated.list" "$test_dir/quick.list" >/dev/null || fail "repeated same profile changed selection"

# Default profiles emphasize sampled materials, scene timing without a HUD,
# Quake-like CPU lighting and one matched smooth CPU/XGL lighting comparison.
for profile in quick medium long; do
    if grep -E '^texture-map/(point|bilinear|trilinear)/|^transparency-stress/' "$test_dir/$profile.list" >/dev/null; then
        fail "$profile selected optional native texture mapping or transparency stress"
    fi
    for compute in cpu-smooth xgl-smooth; do
        grep -x "lighting/submit/multi_polygon/$compute/point-1/depth-1" "$test_dir/$profile.list" >/dev/null ||
            fail "$profile lost matched $compute lighting"
    done
    for shade in flat smooth; do
        grep -x "lighting/quake-like/cpu-$shade/point-1/depth-1" "$test_dir/$profile.list" >/dev/null ||
            fail "$profile lost Quake-like CPU $shade lighting"
        grep -x "lighting/baked/cpu-$shade/cached/depth-1" "$test_dir/$profile.list" >/dev/null ||
            fail "$profile lost cached CPU $shade lighting"
    done
    for material in facet-flat vertex-smooth; do
        grep -x "scene/texture/$material/depth-1" "$test_dir/$profile.list" >/dev/null ||
            fail "$profile lost scene-only $material frame"
    done
    grep -x 'overhead/color-depth-clear' "$test_dir/$profile.list" >/dev/null ||
        fail "$profile lost combined clear comparison"
done
for profile in medium long; do
    for ui in menu console; do
        grep -x "ui/$ui" "$test_dir/$profile.list" >/dev/null || fail "$profile omitted native $ui"
    done
    if grep -E '^overhead/(color-clear|depth-clear)$' "$test_dir/$profile.list" >/dev/null; then
        fail "$profile kept separate clears instead of UI coverage"
    fi
done
awk '
    /^scene\/lighting\// { scene=NR }
    /^frame\/lighting\// { if (!scene) bad++; frame=NR }
    /^lighting\/baked\/.*\/styles-4\// { if (!frame) bad++; style=NR }
    /^transparency\// { if (!style) bad++; blends++ }
    /^texture\/.*\/depth-2$/ { if (blends != 6) bad++; depth=NR }
    /^pipeline\// { if (!depth) bad++; pipeline++ }
    END { if (!scene || !frame || !style || !depth || pipeline != 2 || bad) exit 1 }
' "$test_dir/long.list" || fail "long profile priority order changed"

"$bench" -quick -test texture-map/point/draw/depth-0 -list >"$test_dir/native.list"
grep -x 'texture-map/point/draw/depth-0' "$test_dir/native.list" >/dev/null ||
    fail "optional native texture case became inaccessible"

# A filter addresses the full registry, not just the selected profile subset.
"$bench" -quick -test lighting/matched/xgl-flat/point-4/depth-1 -list \
    >"$test_dir/filtered.list"
grep -x 'lighting/matched/xgl-flat/point-4/depth-1' "$test_dir/filtered.list" >/dev/null ||
    fail "explicit filter could not reach a case outside quick"
test "$(wc -l <"$test_dir/filtered.list" | tr -d ' ')" = 1 || fail "filter selected extra cases"
expect_fail -test -medium -quick -list
grep 'No test names match' "$test_dir/output" >/dev/null ||
    fail "a profile-looking filter value was parsed as a profile flag"

# Fine virtual-clock increments and short explicit samples finish the selected
# workloads quickly. Even -pause 0 retains mandatory protective idle periods.
for profile in quick medium long extralong; do
    XB_TEST_CLOCK_STEP=.001 XB_TEST_LOG="$test_dir/$profile.log" \
        "$bench" "-$profile" -triangles 8 -seconds .001 -warmup 0 -repeats 1 -pause 0 \
        >"$test_dir/output" 2>&1 || fail "$profile failed to complete short samples"
    grep "Profile: $profile" "$test_dir/output" >/dev/null || fail "missing $profile label"
    grep 'Nominal active + minimum idle:' "$test_dir/output" >/dev/null || fail "missing nominal estimate"
    grep '^open 320 240$' "$test_dir/$profile.log" >/dev/null || fail "$profile first resolution"
    case "$profile" in
        quick|medium)
            if grep '^resize ' "$test_dir/$profile.log" >/dev/null; then fail "$profile ran a second resolution"; fi
            ;;
        *) grep '^resize 640 480$' "$test_dir/$profile.log" >/dev/null || fail "$profile omitted its second resolution" ;;
    esac
    awk '/^clock-sleep / { if ($2 <= 0 || $2 > .100001) bad++; sleeps++ }
         END { if (!sleeps || bad) exit 1 }' "$test_dir/$profile.log" ||
        fail "$profile disabled mandatory rest or used uninterruptible sleep chunks"
    case "$profile" in quick) completed=16 ;; medium) completed=34 ;; long) completed=68 ;; *) completed=474 ;; esac
    grep "Completed $completed cases; 0 skipped/failed" "$test_dir/output" >/dev/null ||
        fail "$profile omitted short samples from its selection"
done

# Check the actual preset sample/repeat/warm-up defaults separately from the
# shortened selection tests above; nominal profile lengths are not promises.
for profile in quick medium long extralong; do
    case "$profile" in
        quick) settings='0.200 seconds/trial; 1 repeats; 0.020 seconds warm-up' ;;
        medium) settings='0.250 seconds/trial; 2 repeats; 0.030 seconds warm-up' ;;
        long) settings='0.400 seconds/trial; 2 repeats; 0.050 seconds warm-up' ;;
        extralong) settings='0.500 seconds/trial; 3 repeats; 0.100 seconds warm-up' ;;
    esac
    "$bench" "-$profile" -test submit/multi_polygon/batch-128 -triangles 8 \
        >"$test_dir/output" 2>&1 || fail "$profile defaults run"
    grep "$settings" "$test_dir/output" >/dev/null || fail "$profile sample defaults changed"
done

# Explicit timing, resolution and pause overrides work on either side of the
# profile flag. Profiles change defaults, never override an explicit value.
for order in before after; do
    case "$order" in
        before) set -- -sim-fps 60 -seconds .001 -warmup 0 -repeats 2 -pause 0 -winsize 192 128 -long ;;
        after) set -- -long -sim-fps 60 -seconds .001 -warmup 0 -repeats 2 -pause 0 -winsize 192 128 ;;
    esac
    XB_TEST_LOG="$test_dir/override-$order.log" \
        "$bench" "$@" -triangles 8 -test submit/multi_polygon/batch-128 \
        -csv "$test_dir/override-$order.csv" >"$test_dir/output" 2>&1 || fail "$order overrides failed"
    grep '0.001 seconds/trial; 2 repeats; 0.000 seconds warm-up' "$test_dir/output" >/dev/null ||
        fail "$order timing options were overridden by the profile"
    grep '^open 192 128$' "$test_dir/override-$order.log" >/dev/null || fail "$order size override"
    grep '^simulation-fps 60 0$' "$test_dir/override-$order.log" >/dev/null || fail "$order simulation override"
    if grep '^resize ' "$test_dir/override-$order.log" >/dev/null; then
        fail "$order size override was ignored"
    fi
done

# Cooling occurs between completed cases, never in a trial or after the last
# case. The same virtual draw workload must produce the same measured times.
for pause in 0 .5; do
    XB_TEST_LOG="$test_dir/pause-$pause.log" "$bench" -quick -test texture/ \
        -triangles 8 -seconds .001 -warmup 0 -repeats 1 -pause "$pause" \
        -csv "$test_dir/pause-$pause.csv" >"$test_dir/output" 2>&1 || fail "pause $pause run"
done
grep '^clock-sleep ' "$test_dir/pause-0.log" >/dev/null || fail "zero minimum pause disabled mandatory rest"
awk '
    /^lighting-configure / { cases++ }
    /^clock-sleep / {
        if (cases == 10 || $2 <= 0 || $2 > .100001) bad++
        rested[cases]=1; sleeps++
    }
    END {
        for (i=1; i<10; ++i) if (!rested[i]) bad++
        if (cases != 10 || !sleeps || bad) exit 1
    }
' "$test_dir/pause-.5.log" || fail "cooling placement or event polling interval is incorrect"
awk -F, '
    FNR == 1 {
        file++
        for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); col[$i]=i }
        record=col["record"]; name=col["test"]; status=col["status"]; total=col["total_ms"]
        next
    }
    $record == "summary" {
        if ($status != "ok" || $total <= 0) bad++
        if (file == 1) baseline[$name]=$total
        else {
            if (!($name in baseline) || $total < baseline[$name]*.999 ||
                $total > baseline[$name]*1.001) bad++
            compared++
        }
    }
    END { if (!record || !name || !status || !total || compared != 10 || bad) exit 1 }
' "$test_dir/pause-0.csv" "$test_dir/pause-.5.csv" || fail "idle time leaked into reported throughput"

# Native UI formatting/layout/draw work belongs inside measured render and
# total time. Advancing only its mock draw clock must not affect scene-only
# controls, preparation, or presentation time.
for workload in ui frame scene; do
    case "$workload" in
        ui) selected=ui/ ;;
        frame) selected=frame/lighting/cpu-flat/quake-point-1/depth-1 ;;
        scene) selected=scene/lighting/cpu-flat/quake-point-1/depth-1 ;;
    esac
    for delay in 0 .05; do
        XB_TEST_CLOCK_STEP=.001 XB_TEST_UI_ADVANCE="$delay" \
            "$bench" -quick -winsize 320 240 -test "$selected" -triangles 8 \
            -seconds .001 -warmup 0 -repeats 1 -pause 0 \
            -csv "$test_dir/ui-time-$workload-$delay.csv" >"$test_dir/output" 2>&1 ||
            fail "$workload native UI timing sample failed"
    done
    awk -F, -v workload="$workload" '
        FNR == 1 {
            file++
            for (i=1; i<=NF; ++i) col[$i]=i
            record=col["record"]; name=col["test"]; status=col["status"]
            total=col["total_ms"]; render=col["render_ms"]; prep=col["prepare_ms"]; present=col["present_ms"]
            next
        }
        $record == "summary" {
            if ($status != "ok") bad++
            if (file == 1) {
                totals[$name]=$total; renders[$name]=$render; preps[$name]=$prep; presents[$name]=$present
            } else {
                expected=workload == "scene" ? 0 : 50
                delta=$total-totals[$name]
                if (delta < expected-.000001 || delta > expected+.000001) bad++
                delta=$render-renders[$name]
                if (delta < expected-.000001 || delta > expected+.000001 ||
                    $prep != preps[$name] || $present != presents[$name]) bad++
                compared++
            }
        }
        END {
            expected=workload == "ui" ? 3 : 1
            if (!record || !name || !status || !total || !render || !prep || !present || compared != expected || bad) exit 1
        }
    ' "$test_dir/ui-time-$workload-0.csv" "$test_dir/ui-time-$workload-.05.csv" ||
        fail "$workload native UI cost escaped rendering time or affected another phase"
done

# Opening consumes the fixed profile budget even with explicit sample/pause
# overrides. Once opening exhausts it, no workload should be configured.
if XB_TEST_OPEN_ADVANCE=70 XB_TEST_LOG="$test_dir/open-expiry.log" \
    "$bench" -quick -seconds .001 -warmup 0 -repeats 1 -pause 0 -triangles 8 \
    -csv "$test_dir/open-expiry.csv" >"$test_dir/output" 2>&1; then
    fail "explicit timing overrides removed the protective deadline"
fi
grep 'Profile time budget reached' "$test_dir/output" >/dev/null || fail "opening delay escaped the budget"
grep '16 selected cases not run' "$test_dir/output" >/dev/null || fail "opening expiry lost remaining cases"
if grep '^lighting-configure ' "$test_dir/open-expiry.log" >/dev/null; then
    fail "expired profile continued configuring workloads"
fi

# CSV coverage includes every selected case even if setup exhausts the budget.
# Blank timings distinguish never-attempted work from a measured zero.
awk -F, '
    NR == 1 {
        columns=NF
        for (i=1; i<=NF; ++i) col[$i]=i
        record=col["record"]; status=col["status"]; kind=col["failure_kind"]
        total=col["total_ms"]; render=col["render_ms"]; rate=col["triangles_per_second"]
        enqueue=col["enqueue_ms"]; drain=col["drain_ms"]
        complete=col["completed_facets_sec"]; updated=col["updated_color_light_items"]; cached=col["cached_color_light_items"]
        uprep=col["updated_prepare_ms"]; cprep=col["cached_prepare_ms"]
        current=col["native_current_bytes"]; peak=col["native_peak_bytes"]
        next
    }
    {
        if (NF != columns || $record != "summary" || $status != "not-run" ||
            $kind != "not-run" || $total != "" || $render != "" || $rate != "" ||
            $enqueue != "" || $drain != "" || $complete != "" || $updated != "" || $cached != "" ||
            $uprep != "" || $cprep != "" || $current != "" || $peak != "") bad++
        rows++
    }
    END { if (!record || !status || !kind || !total || !render || !rate || !enqueue || !drain || !complete || !updated || !cached || !uprep || !cprep || !current || !peak || rows != 16 || bad) exit 1 }
' "$test_dir/open-expiry.csv" || fail "unattempted cases missing or reported timing"

# The fourth/fifth draw is inside the first timed trial after preview and one
# warm-up item. A blocking call can overrun the deadline, but its incomplete
# trial must never be accepted, including an overrun in the final required draw.
for draw in 4 5; do
    if XB_TEST_CLOCK_STEP=.001 XB_TEST_DRAW_ADVANCE=70 XB_TEST_DRAW_ADVANCE_AT="$draw" \
        XB_TEST_LOG="$test_dir/expiry-$draw.log" "$bench" -quick \
        -test submit/multi_polygon/batch-128 -seconds .001 -warmup 0 -repeats 1 -pause 0 \
        -triangles 8 -csv "$test_dir/expiry-$draw.csv" >"$test_dir/output" 2>&1; then
        fail "expired partial/final trial reported success"
    fi
    grep "^draw-advance $draw 70$" "$test_dir/expiry-$draw.log" >/dev/null || fail "timed overrun was not injected"
    grep 'profile time budget reached' "$test_dir/output" >/dev/null || fail "timed deadline reason lost"
    awk -F, '
        NR == 1 {
            for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); col[$i]=i }
            record=col["record"]; status=col["status"]; reason=col["reason"]; kind=col["failure_kind"]
            next
        }
        {
            if ($record != "summary" || $status != "skipped" || $kind != "time-limit" || $reason !~ /profile time budget reached/) bad++
            rows++
        }
        END { if (!record || !status || !reason || !kind || rows != 1 || bad) exit 1 }
    ' "$test_dir/expiry-$draw.csv" || fail "timed-out partial work produced a measurement"
done

# An in-flight scene draw may return after the deadline. The caller must then
# stop before issuing the frame HUD, swap or another draw, including cleanup.
if XB_TEST_CLOCK_STEP=.001 XB_TEST_DRAW_ADVANCE=70 XB_TEST_DRAW_ADVANCE_AT=4 \
    XB_TEST_LOG="$test_dir/frame-expiry.log" "$bench" -quick \
    -test frame/lighting/cpu-flat/point-1/depth-0 -seconds .001 -warmup 0 \
    -repeats 1 -pause 0 -triangles 8 >"$test_dir/output" 2>&1; then
    fail "expired scene draw reported a completed frame"
fi
awk '
    /^draw-advance / { expired=1; injected++; if ($2 != 4 || $3 != 70) bad++; next }
    expired && /^frame-(draw|hud|swap) / { bad++ }
    /^close / { closed++ }
    END { if (injected != 1 || closed != 1 || bad) exit 1 }
' "$test_dir/frame-expiry.log" || fail "deadline allowed more rendering before close"
grep 'profile time budget reached' "$test_dir/output" >/dev/null || fail "expired frame lost its deadline reason"

# Extralong has a larger finite deadline and keeps protective rest. Setup plus
# its rest interval can exceed shorter profiles while still fitting extralong.
XB_TEST_OPEN_ADVANCE=200 "$bench" -extralong -winsize 320 240 -triangles 8 \
    -test submit/multi_polygon/batch-128 -seconds .001 -warmup 0 -repeats 1 -pause 0 \
    >"$test_dir/output" 2>&1 || fail "extralong inherited a shorter profile deadline"
grep 'Completed 1 cases; 0 skipped/failed' "$test_dir/output" >/dev/null || fail "extralong was truncated"
grep 'Time limit: 2700 seconds' "$test_dir/output" >/dev/null || fail "extralong omitted its finite limit"

# Explicit tiny samples and zero optional pause cannot disable the 45-minute
# ceiling. Even the full profile stops before configuring work after expiry.
if XB_TEST_OPEN_ADVANCE=2701 XB_TEST_LOG="$test_dir/extra-expiry.log" \
    "$bench" -extralong -seconds .001 -warmup 0 -repeats 1 -pause 0 -triangles 8 \
    >"$test_dir/output" 2>&1; then
    fail "extralong expired opening was allowed to continue"
fi
grep 'Time limit: 2700 seconds' "$test_dir/output" >/dev/null || fail "extralong overrides changed the ceiling"
grep 'Profile time budget reached' "$test_dir/output" >/dev/null || fail "extralong remained unbounded"
grep '474 selected cases not run' "$test_dir/output" >/dev/null || fail "extralong expiry lost unrun cases"
if grep '^lighting-configure ' "$test_dir/extra-expiry.log" >/dev/null; then
    fail "expired extralong profile configured a workload"
fi

# The first setup-cooling sleep simulates Escape. Cancellation is polled before
# any workload starts, rather than waiting through the full rest interval.
if XB_TEST_CANCEL_ON_PAUSE=1 XB_TEST_LOG="$test_dir/cancel.log" \
    "$bench" -quick -triangles 8 >"$test_dir/output" 2>&1; then
    fail "cancellation during cooling reported normal completion"
fi
grep '^pause-cancel ' "$test_dir/cancel.log" >/dev/null || fail "pause did not poll cancellation"
if grep '^lighting-configure ' "$test_dir/cancel.log" >/dev/null; then fail "cancellation started a workload"; fi
test "$(grep -c '^clock-sleep ' "$test_dir/cancel.log")" = 1 || fail "cancellation waited through the entire pause"
grep 'interrupted' "$test_dir/output" >/dev/null || fail "cooldown cancellation was not reported"

echo 'Profile tests: selection, defaults, overrides, nominal durations, real idle scheduling, mandatory rest, deadline rejection and cancellation passed'
