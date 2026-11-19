#!/bin/sh
# Portable CLI/scheduling regression tests. Never measures XGL performance.
# GPL-2.0-or-later; see COPYING.
set -eu

bench=${1:-./build/xglbench-test}

# Remove the optional minimum pause; mandatory protective rest still applies.
run_bench()
{
    "$bench" -pause 0 "$@"
}
test_dir=build/cli-test-$$
mkdir -p "$test_dir"
trap 'rm -rf "$test_dir"' 0
trap 'exit 1' 1 2 3 15

fail()
{
    echo "CLI test failed: $*" >&2
    if test -f "$test_dir/output"; then cat "$test_dir/output" >&2; fi
    exit 1
}

expect_fail()
{
    if run_bench "$@" >"$test_dir/output" 2>&1; then
        fail "unexpected success: $*"
    fi
}

expect_fail -unknown
expect_fail -winsize
expect_fail -winsize 320
expect_fail -winsize 0 240
expect_fail -winsize -320 240
expect_fail -winsize 320x 240
expect_fail -seconds 0
expect_fail -seconds nan
expect_fail -seconds inf
expect_fail -seconds 1garbage
expect_fail -warmup -1
expect_fail -repeats 0
expect_fail -repeats 32
expect_fail -triangles 3
expect_fail -triangles 0
expect_fail -triangles 16386
expect_fail -queue
expect_fail -queue 0
expect_fail -queue -1
expect_fail -queue 9
expect_fail -queue 2garbage
expect_fail -queue nan
expect_fail -sim-fps
expect_fail -sim-fps 0
expect_fail -sim-fps -1
expect_fail -sim-fps 241
expect_fail -sim-fps 15.5
expect_fail -sim-fps 60garbage
expect_fail -sim-fps nan
expect_fail -test __no_such_case__

XB_TEST_LOG="$test_dir/help.log" run_bench -help >"$test_dir/output" 2>&1
test ! -e "$test_dir/help.log" || fail "help opened a display"
XB_TEST_LOG="$test_dir/list.log" run_bench -extralong -list >"$test_dir/output" 2>&1
test ! -e "$test_dir/list.log" || fail "list opened a display"
grep 'submit/multi_polygon/batch-128' "$test_dir/output" >/dev/null || fail "missing stable case"
for area in 4 16 64 100 256 1024; do
    grep "area/${area}px" "$test_dir/output" >/dev/null || fail "missing calibrated $area pixel case"
done
for shape in horizontal-16x1 vertical-1x16; do
    grep "shape/100px/$shape" "$test_dir/output" >/dev/null || fail "missing $shape case"
done
for mode in serialized streaming; do
    for colors in cached relight; do
        for depth in 0 1 2; do
            grep "pipeline/$mode/$colors/depth-$depth" "$test_dir/output" >/dev/null ||
                fail "missing $mode/$colors/depth-$depth case"
        done
    done
done

for path in cpu-flat cpu-smooth xgl-flat xgl-smooth; do
    for light in directional point-1; do
        for depth in 0 1 2; do
            grep "lighting/matched/$path/$light/depth-$depth" "$test_dir/output" >/dev/null ||
                fail "missing matched $path/$light/depth-$depth case"
        done
    done
    for light in 2 4; do
        grep "lighting/matched/$path/point-$light/depth-1" "$test_dir/output" >/dev/null ||
            fail "missing matched $path/point-$light case"
    done
    grep "lighting/static/$path/directional/depth-1" "$test_dir/output" >/dev/null ||
        fail "missing static $path case"
    grep "lighting/area-100/$path/point-1" "$test_dir/output" >/dev/null ||
        fail "missing controlled-area $path case"
done
for path in cpu-flat cpu-smooth; do
    for styles in cached styles-1 styles-4; do
        for depth in 0 1 2; do
            grep "lighting/baked/$path/$styles/depth-$depth" "$test_dir/output" >/dev/null ||
                fail "missing baked $path/$styles/depth-$depth case"
        done
    done
    for lights in 1 2 4; do
        grep "lighting/quake-like/$path/point-$lights/depth-1" "$test_dir/output" >/dev/null ||
            fail "missing Quake-like $path/point-$lights case"
    done
done

for depth in 0 1 2; do
    for material in facet-flat vertex-smooth; do
        grep -x "texture/$material/depth-$depth" "$test_dir/output" >/dev/null ||
            fail "missing sampled-material $material/depth-$depth case"
    done
    for path in cpu-flat cpu-smooth xgl-flat xgl-smooth; do
        grep -x "frame/lighting/$path/point-1/depth-$depth" "$test_dir/output" >/dev/null ||
            fail "missing complete lit frame $path/depth-$depth case"
    done
    for path in cpu-flat cpu-smooth; do
        grep -x "frame/lighting/$path/quake-point-1/depth-$depth" "$test_dir/output" >/dev/null ||
            fail "missing complete Quake-like frame $path/depth-$depth case"
        for mode in serialized streaming; do
            grep -x "pipeline/$mode/quake-$path/depth-$depth" "$test_dir/output" >/dev/null ||
                fail "missing Quake-like pipeline $mode/$path/depth-$depth case"
        done
    done
done
for method in polygon multi_polygon; do
    for path in cpu-flat cpu-smooth xgl-flat xgl-smooth; do
        grep -x "lighting/submit/$method/$path/point-1/depth-1" "$test_dir/output" >/dev/null ||
            fail "missing lit submission $method/$path case"
    done
done
for winding in all-front mixed; do
    for culling in off back; do
        grep -x "cull/$winding/$culling" "$test_dir/output" >/dev/null ||
            fail "missing culling $winding/$culling case"
    done
done

for path in facet-flat vertex-smooth point bilinear trilinear; do
    for work in draw frame; do
        for depth in 0 1 2; do
            grep -x "texture-map/$path/$work/depth-$depth" "$test_dir/output" >/dev/null ||
                fail "missing texture-map $path/$work/depth-$depth case"
        done
    done
    grep -x "texture-map/$path/perspective-motion/depth-1" "$test_dir/output" >/dev/null ||
        fail "missing moving perspective texture-map $path case"
done

for path in facet-flat vertex-smooth; do
    for depth in 0 1; do
        grep -x "scene/texture/$path/depth-$depth" "$test_dir/output" >/dev/null ||
            fail "missing scene-only sampled material $path/depth-$depth case"
    done
done
for path in cpu-flat cpu-smooth; do
    grep -x "scene/lighting/$path/quake-point-1/depth-1" "$test_dir/output" >/dev/null ||
        fail "missing scene-only Quake lighting $path case"
done
for family in transparency transparency-stress; do
    for path in opaque blended-zfreeze blended-zwrite; do
        for order in near-to-far far-to-near; do
            grep -x "$family/$path/$order" "$test_dir/output" >/dev/null ||
                fail "missing $family/$path/$order case"
        done
    done
done
for ui in hud menu console; do
    grep -x "ui/$ui" "$test_dir/output" >/dev/null || fail "missing native $ui UI case"
done
if grep -E '^hud/|raster' "$test_dir/output" >/dev/null; then
    fail "removed bitmap raster workload remains selectable"
fi
grep -x 'overhead/color-depth-clear' "$test_dir/output" >/dev/null || fail "missing combined clear case"

XB_TEST_LOG="$test_dir/default.log" run_bench -long -allow-fallback \
    -test submit/multi_polygon/batch-128 -triangles 8 -seconds .002 -warmup 0 \
    -repeats 1 -csv "$test_dir/default.csv" >"$test_dir/output" 2>&1
grep '^open 320 240$' "$test_dir/default.log" >/dev/null || fail "default initial size"
grep '^simulation-fps 15 0$' "$test_dir/default.log" >/dev/null || fail "default simulation rate is not 15 FPS"
grep '^resize 640 480$' "$test_dir/default.log" >/dev/null || fail "default second size"
test "$(grep -c '^resize ' "$test_dir/default.log")" = 1 || fail "unexpected resize sequence"
grep 'NO VALID HARDWARE PERFORMANCE DATA' "$test_dir/output" >/dev/null || fail "missing mock warning"

# Discover columns by name so harmless CSV column reordering is permitted.
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) {
            gsub(/"/, "", $i)
            if ($i == "width") w=i
            if ($i == "height") h=i
        }
        next
    }
    w && h {
        if ($w == 320 && $h == 240) small++
        if ($w == 640 && $h == 480) large++
    }
    END { if (!w || !h || !small || !large) exit 1 }
' "$test_dir/default.csv" || fail "CSV did not distinguish both resolutions"

XB_TEST_LOG="$test_dir/forced.log" run_bench -winsize 800 600 -quick \
    -allow-fallback -test submit/multi_polygon/batch-128 -triangles 8 \
    -seconds .002 -warmup 0 -repeats 1 -csv "$test_dir/forced.csv" \
    >"$test_dir/output" 2>&1
grep '^open 800 600$' "$test_dir/forced.log" >/dev/null || fail "forced size not used"
if grep '^resize ' "$test_dir/forced.log" >/dev/null; then fail "forced size resized"; fi
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) {
            gsub(/"/, "", $i)
            if ($i == "width") w=i
            if ($i == "height") h=i
        }
        next
    }
    w && h { if ($w != 800 || $h != 600) bad++; rows++ }
    END { if (!w || !h || !rows || bad) exit 1 }
' "$test_dir/forced.csv" || fail "forced size CSV contained other dimensions"

if XB_TEST_FAIL_METHOD=multi_polygon run_bench -winsize 320 240 \
    -quick -allow-fallback -test submit/multi_polygon/batch-128 \
    -csv "$test_dir/skipped.csv" >"$test_dir/output" 2>&1; then
    fail "all-invalid run reported success"
fi
grep 'injected test-only validation failure' "$test_dir/output" >/dev/null || fail "missing validation failure"
if grep 'Lowest measured' "$test_dir/output" >/dev/null; then fail "failed method was recommended"; fi
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) {
            gsub(/"/, "", $i)
            if ($i == "record") r=i
            if ($i == "status") s=i
        }
        next
    }
    r && s { if ($r != "summary" || $s != "skipped") bad++; rows++ }
    END { if (!r || !s || !rows || bad) exit 1 }
' "$test_dir/skipped.csv" || fail "invalid method had a successful/timed CSV row"

# Explicit timing settings override -quick even when -quick comes last.
run_bench -winsize 320 240 -test submit/multi_polygon/batch-128 \
    -triangles 8 -allow-fallback -seconds .002 -warmup 0 -repeats 2 -quick \
    >"$test_dir/output" 2>&1
grep '0.002 seconds/trial; 2 repeats; 0.000 seconds warm-up' \
    "$test_dir/output" >/dev/null || fail "quick overrode explicit settings"

expect_fail -quick -allow-fallback -test submit/multi_polygon/batch-128 \
    -csv "$test_dir/nonexistent/results.csv"

fault_case()
{
    mode=$1
    fault_test=${2:-submit/multi_polygon/batch-128}
    fault_draw=${3:-5}
    case "$mode" in
        resize) set -- ;;
        *) set -- -winsize 320 240 ;;
    esac
    # A completion error in the final draw must not become a winning result.
    # The ordinary case uses preview + warm-up + three minimum timed frames.
    # Streaming uses preview + eight warm-up + eight timed work items.
    if XB_TEST_FAULT="$mode" XB_TEST_FAULT_DRAW="$fault_draw" \
        XB_TEST_LOG="$test_dir/$mode.log" run_bench "$@" \
        -allow-fallback -test "$fault_test" -triangles 8 -queue 8 \
        -seconds .001 -warmup 0 -repeats 1 -csv "$test_dir/$mode.csv" \
        >"$test_dir/output" 2>&1; then
        fail "faulted $mode run reported success"
    fi
    grep "^$mode 320 240" "$test_dir/$mode.log" >/dev/null || fail "$mode was not injected"
    if grep 'Lowest measured' "$test_dir/output" >/dev/null; then
        fail "$mode fault produced a winner"
    fi
    awk -F, '
        NR == 1 {
            for (i=1; i<=NF; ++i) {
                gsub(/"/, "", $i)
                if ($i == "record") r=i
                if ($i == "status") s=i
            }
            next
        }
        r && s { if ($r != "summary" || $s != "skipped") bad++; rows++ }
        END { if (!r || !s || rows != 1 || bad) exit 1 }
    ' "$test_dir/$mode.csv" || fail "$mode produced a successful or mislabeled measurement"
    case "$mode" in
        native-error)
            grep 'injected native completion failure' "$test_dir/output" >/dev/null || fail "native error lost"
            ;;
        resize)
            egrep 'size.*stopped|stopped.*size' "$test_dir/output" >/dev/null || fail "resize stop not explained"
            if grep '^resize 640 480$' "$test_dir/$mode.log" >/dev/null; then
                fail "unexpected resize did not stop the sequence"
            fi
            ;;
        expose)
            egrep 'exposed|obscured' "$test_dir/output" >/dev/null || fail "exposure not explained"
            ;;
        interrupt)
            grep 'interrupted by user' "$test_dir/output" >/dev/null || fail "interrupt not explained"
            ;;
    esac
}

fault_case native-error
fault_case resize
fault_case expose
fault_case interrupt

for mode in native-error resize expose interrupt; do
    fault_case "$mode" pipeline/streaming/relight/depth-0 17
done

# Fixed screen-space areas stay fixed across the default two resolutions,
# including the horizontal and vertical skinny-triangle variants.
run_bench -long -allow-fallback -test 100px -triangles 8 -seconds .001 -warmup 0 \
    -repeats 1 -csv "$test_dir/areas.csv" >"$test_dir/output" 2>&1
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); column[$i]=i }
        mean=column["nominal_area_mean_px"]; lo=column["nominal_area_min_px"]
        hi=column["nominal_area_max_px"]; payload=column["input_payload_bytes"]
        width=column["width"]; height=column["height"]; status=column["status"]
        record=column["record"]
        next
    }
    {
        if ($status != "ok" || $mean < 99.99 || $mean > 100.01 ||
            $lo < 99.99 || $hi > 100.01 || $payload <= 0) bad++
        if ($record == "summary" && $width == 320 && $height == 240) small++
        if ($record == "summary" && $width == 640 && $height == 480) large++
    }
    END {
        if (!mean || !lo || !hi || !payload || !width || !height || !status ||
            !record || small != 3 || large != 3 || bad) exit 1
    }
' "$test_dir/areas.csv" || fail "calibrated triangle area or payload CSV is invalid"

# The mock retains independent per-slot color copies until completion. Its
# journal verifies bounded submission and lifetime, not a GPU speed model.
check_queue_log()
{
    awk -v expected="$2" '
        /^queue-begin / {
            if (active || $2 != expected) bad++
            active=1; limit=$2; outstanding=0; submitted=0; completed=0
            for (slot in pending) delete pending[slot]
            begins++
        }
        /^queue-prepare / {
            if (!active || $2 < 0 || $2 >= limit || pending[$2] || $3 != outstanding) bad++
        }
        /^queue-draw / {
            if (!active || $2 < 0 || $2 >= limit || pending[$2]) bad++
            pending[$2]=1; outstanding++; submitted++
            draws++; unposted++
            if ($3 != outstanding || outstanding > limit) bad++
            if (outstanding > peak) peak=outstanding
        }
        /^queue-clear / {
            if (!active || $2 != 1 || $3 != 1) bad++
            clears++
        }
        /^queue-post / {
            if (!active || $2 != outstanding || $3 != completed || unposted != 1) bad++
            unposted=0
            posts++
        }
        /^queue-drain / {
            if (!active || $2 != outstanding || $3 != completed+outstanding || unposted) bad++
            completed+=outstanding; outstanding=0
            for (slot in pending) pending[slot]=0
            drains++
        }
        /^queue-end / {
            if (!active || outstanding || $2 != submitted || $3 != completed || submitted != completed) bad++
            active=0; ends++
        }
        /^queue-error / { bad++ }
        END {
            if (active || bad || !begins || begins != ends || !posts || !drains ||
                peak != expected || draws != posts || draws != clears) exit 1
        }
    ' "$1" || fail "queue $2 exceeded its bound, reused retained data, or lost completions"
}

check_pipeline_csv()
{
    awk -F, -v expected="$2" '
        NR == 1 {
            for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); column[$i]=i }
            q=column["queue_limit"]; m=column["max_queued"]; d=column["drain_calls"]
            e=column["enqueue_ms"]; wait=column["drain_ms"]; render=column["render_ms"]
            total=column["total_ms"]; mode=column["timing_mode"]; status=column["status"]
            next
        }
        {
            if ($status != "ok" || $q != expected || $m < 1 || $m > expected ||
                $d < 1 || $e < 0 || $wait < 0 || $render != "" || $mode == "") bad++
            rows++
        }
        END {
            if (!q || !m || !d || !e || !wait || !render || !total || !mode || !status || !rows || bad) exit 1
        }
    ' "$1" || fail "pipeline CSV omitted completion diagnostics or mislabeled rendering time"
}

for queue in 1 2 8; do
    for colors in cached relight; do
        log="$test_dir/queue-$queue-$colors.log"
        csv="$test_dir/queue-$queue-$colors.csv"
        XB_TEST_LOG="$log" run_bench -winsize 320 240 -allow-fallback \
            -test "pipeline/streaming/$colors/depth-1" -triangles 8 \
            -queue "$queue" -seconds .001 -warmup 0 -repeats 1 -csv "$csv" \
            >"$test_dir/output" 2>&1
        check_queue_log "$log" "$queue"
        check_pipeline_csv "$csv" "$queue"
        if test "$colors" = relight; then
            grep '^queue-prepare ' "$log" >/dev/null || fail "relight did not update slot colors"
        fi
    done
done

# Serialized controls use one retained buffer even when the streaming limit is
# higher. Both modes exercise the same primitive, clears and native storage.
XB_TEST_LOG="$test_dir/serialized.log" run_bench -winsize 320 240 -allow-fallback \
    -test pipeline/serialized/relight/depth-1 -triangles 8 -queue 8 \
    -seconds .001 -warmup 0 -repeats 1 -csv "$test_dir/serialized.csv" \
    >"$test_dir/output" 2>&1
check_queue_log "$test_dir/serialized.log" 1
check_pipeline_csv "$test_dir/serialized.csv" 1

# The default queue is two. The queue option does not change serialized tests.
XB_TEST_LOG="$test_dir/default-queue.log" run_bench -winsize 320 240 -allow-fallback \
    -test pipeline/streaming/cached/depth-0 -triangles 8 \
    -seconds .001 -warmup 0 -repeats 1 >"$test_dir/output" 2>&1
check_queue_log "$test_dir/default-queue.log" 2

# A deliberately slow final mock completion is included in completed wall time
# and reported as drain time. It is not evidence about hardware performance.
XB_TEST_LOG="$test_dir/delayed.log" XB_TEST_DRAIN_DELAY_MS=20 \
    run_bench -winsize 320 240 -allow-fallback \
    -test pipeline/streaming/relight/depth-0 -triangles 8 -queue 8 \
    -seconds .001 -warmup 0 -repeats 1 -csv "$test_dir/delayed.csv" \
    >"$test_dir/output" 2>&1
check_queue_log "$test_dir/delayed.log" 8
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); column[$i]=i }
        total=column["total_ms"]; wait=column["drain_ms"]; record=column["record"]
        next
    }
    $record == "trial" {
        if ($total < 1 || $wait < 1 || $total < $wait) bad++
        trials++
    }
    END { if (!total || !wait || !record || trials != 1 || bad) exit 1 }
' "$test_dir/delayed.csv" || fail "completed timing excluded the final drain"

if XB_TEST_LOG="$test_dir/drain-failure.log" XB_TEST_FAIL_DRAIN=2 \
    XB_TEST_DRAIN_DELAY_MS=20 run_bench -winsize 320 240 -allow-fallback \
    -test pipeline/streaming/relight/depth-0 -triangles 8 -queue 8 \
    -seconds .001 -warmup 0 -repeats 1 -csv "$test_dir/drain-failure.csv" \
    >"$test_dir/output" 2>&1; then
    fail "failed final drain reported success"
fi
grep '^queue-drain-failed ' "$test_dir/drain-failure.log" >/dev/null || fail "final drain failure not injected"
grep 'injected stream completion failure' "$test_dir/output" >/dev/null || fail "completion failure reason lost"
if grep 'Lowest measured' "$test_dir/output" >/dev/null; then fail "failed completion produced a winner"; fi
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); column[$i]=i }
        record=column["record"]; status=column["status"]
        next
    }
    { if ($record != "summary" || $status != "skipped") bad++; rows++ }
    END { if (!record || !status || rows != 1 || bad) exit 1 }
' "$test_dir/drain-failure.csv" || fail "failed final drain produced a successful measurement"

# Lighting logs verify host scheduling and data ownership, not illumination
# pixels. Native readback gates and the portable math tests cover those parts.
check_lighting_log()
{
    awk -v hardware="$2" -v smooth="$3" -v updates="$4" '
        /^lighting-configure / { if (!$2 || $3 != smooth) bad++; cases++ }
        /^lighting-compute / { if ($2 != hardware) bad++ }
        /^lighting-colors / {
            if (hardware || $2 != smooth) bad++
            colors++; changed+=$3
        }
        /^lighting-update / { if (!hardware) bad++; lights++ }
        /^lighting-error / { bad++ }
        END {
            if (bad || cases != 1) exit 1
            if (!updates && (colors || lights)) exit 1
            if (updates && hardware && !lights) exit 1
            if (updates && !hardware && (!colors || !changed)) exit 1
        }
    ' "$1" || fail "lighting path used incorrect updates, payloads or cached data"
}

check_lighting_csv()
{
    awk -F, -v path="$2" -v count="$3" -v model="$4" -v motion="$5" -v styles="$6" '
        NR == 1 {
            for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); col[$i]=i }
            record=col["record"]; status=col["status"]; total=col["total_ms"]
            facets=col["triangles"]; rate=col["triangles_per_second"]
            shade=col["shading"]; equation=col["lighting_model"]; cpu=col["lighting_compute"]
            lights=col["light_count"]; moving=col["light_motion"]; layers=col["style_layers"]
            sample=col["lighting_sample"]; mode=col["timing_mode"]; payload=col["input_payload_bytes"]
            next
        }
        {
            smooth=path ~ /smooth/; hardware=path ~ /^xgl/
            sampling=smooth ? "vertices" : (hardware ? "device-vertex" : "centroid")
            expected_shade=smooth ? "smooth" : "flat"
            expected_compute=hardware ? "xgl" : "cpu"
            if ($status != "ok" || $shade != expected_shade || $equation != model ||
                $cpu != expected_compute || $lights != count || $moving != motion ||
                $layers != styles || $sample != sampling || $mode != "lighting-serialized" ||
                $payload <= 0 || $total <= 0) bad++
            expected=$facets*1000/$total
            if ($rate < expected*.999 || $rate > expected*1.001) bad++
            rows++
        }
        END {
            if (!record || !status || !total || !facets || !rate || !shade || !equation ||
                !cpu || !lights || !moving || !layers || !sample || !mode || !payload ||
                rows != 2 || bad) exit 1
        }
    ' "$1" || fail "lighting CSV mislabeled equation, shading, sample location or completed rate"
}

for path in cpu-flat cpu-smooth xgl-flat xgl-smooth; do
    case "$path" in xgl-*) hardware=1 ;; *) hardware=0 ;; esac
    case "$path" in *smooth) smooth=1 ;; *) smooth=0 ;; esac
    for light in directional point-1 point-2 point-4; do
        case "$light" in point-2) lights=2 ;; point-4) lights=4 ;; *) lights=1 ;; esac
        log="$test_dir/lighting-$path-$light.log"
        csv="$test_dir/lighting-$path-$light.csv"
        XB_TEST_LOG="$log" run_bench -winsize 320 240 -allow-fallback \
            -test "lighting/matched/$path/$light/depth-1" -triangles 8 \
            -seconds .005 -warmup 0 -repeats 1 -csv "$csv" >"$test_dir/output" 2>&1
        check_lighting_log "$log" "$hardware" "$smooth" 1
        check_lighting_csv "$csv" "$path" "$lights" lambert 1 0
    done
    log="$test_dir/static-$path.log"
    csv="$test_dir/static-$path.csv"
    XB_TEST_LOG="$log" run_bench -winsize 320 240 -allow-fallback \
        -test "lighting/static/$path/directional/depth-1" -triangles 8 \
        -seconds .001 -warmup 0 -repeats 1 -csv "$csv" >"$test_dir/output" 2>&1
    check_lighting_log "$log" "$hardware" "$smooth" 0
    check_lighting_csv "$csv" "$path" 1 lambert 0 0
done

# New lit geometry preserves the requested pixel area while its Z surface and
# normals vary. Both resolutions must still report the same geometric area.
run_bench -long -allow-fallback -test lighting/area-100/ -triangles 8 \
    -seconds .001 -warmup 0 -repeats 1 -csv "$test_dir/lit-area.csv" \
    >"$test_dir/output" 2>&1
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); col[$i]=i }
        status=col["status"]; record=col["record"]; width=col["width"]
        average=col["nominal_area_mean_px"]; low=col["nominal_area_min_px"]; high=col["nominal_area_max_px"]
        next
    }
    $record == "summary" {
        if ($status != "ok" || $average < 99.99 || $average > 100.01 ||
            $low < 99.99 || $high > 100.01) bad++
        if ($width == 320) small++
        else if ($width == 640) large++
        else bad++
    }
    END { if (!status || !record || !width || !average || !low || !high || small != 4 || large != 4 || bad) exit 1 }
' "$test_dir/lit-area.csv" || fail "lighting geometry changed calibrated area or resolution labels"


# Payloads include smooth per-vertex colors and hardware normals. Equal
# triangle counts must not imply identical referenced native input sizes.
awk -F, '
    FNR == 1 {
        for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); col[$i]=i }
        record=col["record"]; payload=col["input_payload_bytes"]
        shade=col["shading"]; cpu=col["lighting_compute"]
        next
    }
    $record == "summary" { bytes[$cpu "/" $shade]=$payload }
    END {
        if (!(bytes["cpu/smooth"] > bytes["cpu/flat"] &&
              bytes["xgl/flat"] > bytes["cpu/flat"] &&
              bytes["xgl/smooth"] > bytes["cpu/smooth"])) exit 1
    }
' "$test_dir"/static-*.csv || fail "lighting input estimates omitted vertex colors or normals"

for path in cpu-flat cpu-smooth; do
    case "$path" in *smooth) smooth=1 ;; *) smooth=0 ;; esac
    for styles in cached styles-1 styles-4; do
        case "$styles" in cached) updates=0; layers=1 ;; styles-1) updates=1; layers=1 ;; *) updates=1; layers=4 ;; esac
        log="$test_dir/baked-$path-$styles.log"
        csv="$test_dir/baked-$path-$styles.csv"
        XB_TEST_LOG="$log" run_bench -winsize 320 240 -allow-fallback \
            -test "lighting/baked/$path/$styles/depth-1" -triangles 8 \
            -seconds .05 -warmup 0 -repeats 1 -csv "$csv" >"$test_dir/output" 2>&1
        check_lighting_log "$log" 0 "$smooth" "$updates"
        check_lighting_csv "$csv" "$path" 0 baked 0 "$layers"
    done
    log="$test_dir/quake-$path.log"
    csv="$test_dir/quake-$path.csv"
    XB_TEST_LOG="$log" run_bench -winsize 320 240 -allow-fallback \
        -test "lighting/quake-like/$path/point-2/depth-1" -triangles 8 \
        -seconds .005 -warmup 0 -repeats 1 -csv "$csv" >"$test_dir/output" 2>&1
    check_lighting_log "$log" 0 "$smooth" 1
    check_lighting_csv "$csv" "$path" 2 quake-like 1 1
done

# Short style trials cover changing 10 Hz styles and complete their cadence
# at both default 15 simulated FPS and the optional historical 60 FPS.
for fps in 15 60; do
    case "$fps" in 15) minimum=12; cycle=3 ;; *) minimum=48; cycle=6 ;; esac
    XB_TEST_LOG="$test_dir/style-minimum-$fps.log" XB_TEST_FAULT=native-error \
        XB_TEST_FAULT_DRAW=1000000 run_bench -winsize 320 240 -allow-fallback \
        -test lighting/baked/cpu-flat/styles-1/depth-0 -triangles 8 \
        -sim-fps "$fps" -seconds .001 -warmup 0 -repeats 2 \
        -csv "$test_dir/style-minimum-$fps.csv" >"$test_dir/output" 2>&1
    check_lighting_log "$test_dir/style-minimum-$fps.log" 0 0 1
    grep "^simulation-fps $fps 0$" "$test_dir/style-minimum-$fps.log" >/dev/null ||
        fail "explicit simulation rate $fps did not reach the workload"
    awk -F, -v minimum="$minimum" -v cycle="$cycle" '
        NR == 1 {
            for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); col[$i]=i }
            record=col["record"]; status=col["status"]; frames=col["frames_all_trials"]
            next
        }
        {
            if ($status != "ok" || $frames < minimum || $frames % cycle != 0) bad++
            if ($record == "trial") trials++
            else if ($record == "summary") summaries++
            else bad++
        }
        END { if (!record || !status || !frames || trials != 2 || summaries != 1 || bad) exit 1 }
    ' "$test_dir/style-minimum-$fps.csv" || fail "short style trials omitted a complete changing cadence at $fps FPS"
    awk '
        /^lighting-colors / { updates++; changed+=$3 }
        /^native-error / { bad++ }
        END { if (updates < 4 || changed < 4 || bad) exit 1 }
    ' "$test_dir/style-minimum-$fps.log" || fail "short repeated style trials exercised only cached colors"
done

XB_TEST_LOG="$test_dir/style-cached-control.log" XB_TEST_FAULT=native-error \
    XB_TEST_FAULT_DRAW=1000000 run_bench -winsize 320 240 -allow-fallback \
    -test lighting/baked/cpu-flat/cached/depth-0 -triangles 8 \
    -seconds .001 -warmup 0 -repeats 2 >"$test_dir/output" 2>&1
check_lighting_log "$test_dir/style-cached-control.log" 0 0 0


for mode in native-error resize expose interrupt; do
    fault_case "$mode" lighting/matched/xgl-smooth/point-1/depth-1 5
done

check_lighting_rejected()
{
    awk -F, '
        NR == 1 {
            for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); col[$i]=i }
            record=col["record"]; status=col["status"]
            next
        }
        { if ($record != "summary" || $status != "skipped") bad++; rows++ }
        END { if (!record || !status || rows != 1 || bad) exit 1 }
    ' "$1" || fail "failed lighting operation produced a successful measurement"
}

if XB_TEST_FAIL_LIGHT_UPDATE=2 run_bench -winsize 320 240 -allow-fallback \
    -test lighting/matched/xgl-flat/point-1/depth-1 -triangles 8 \
    -seconds .001 -warmup 0 -repeats 1 -csv "$test_dir/light-update-failure.csv" \
    >"$test_dir/output" 2>&1; then
    fail "failed light-state update reported success"
fi
grep 'injected light-state update failure' "$test_dir/output" >/dev/null || fail "light-state failure reason lost"
check_lighting_rejected "$test_dir/light-update-failure.csv"

if XB_TEST_FAIL_LIGHTING=1 run_bench -winsize 320 240 -allow-fallback \
    -test lighting/matched/xgl-smooth/point-1/depth-1 -triangles 8 \
    -seconds .001 -warmup 0 -repeats 1 -csv "$test_dir/light-validation-failure.csv" \
    >"$test_dir/output" 2>&1; then
    fail "failed lighting readback reported success"
fi
grep 'injected lighting readback validation failure' "$test_dir/output" >/dev/null || fail "lighting validation failure reason lost"
check_lighting_rejected "$test_dir/light-validation-failure.csv"

# Per-vertex materials must reach the backend as three sampled colors per
# triangle, and must be labeled distinctly from facet-sampled controls.
XB_TEST_LOG="$test_dir/texture.log" run_bench -winsize 320 240 -allow-fallback \
    -test texture/ -triangles 8 -seconds .001 -warmup 0 -repeats 1 \
    -csv "$test_dir/texture.csv" >"$test_dir/output" 2>&1
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); col[$i]=i }
        record=col["record"]; status=col["status"]; shade=col["shading"]
        material=col["material_sample"]; payload=col["input_payload_bytes"]
        depth=col["depth"]; model=col["lighting_model"]; budget=col["frame_budget_ms"]; name=col["test"]
        next
    }
    $record == "summary" {
        if ($status != "ok" || $model != "none" || $payload <= 0) bad++
        if ($name ~ /scene\/texture\//) {
            if ($budget < 66.666 || $budget > 66.667) bad++
            scenes++
        } else if ($budget != "") bad++
        if ($shade == "smooth") {
            if ($material != "vertices") bad++
            smooth[$depth]=$payload
        } else {
            if ($shade != "flat" || $material != "facet") bad++
            flat[$depth]=$payload
        }
        rows++
    }
    END {
        for (d=0; d<=2; ++d) if (!(smooth[d] > flat[d] && flat[d] > 0)) bad++
        if (!record || !status || !shade || !material || !payload || !depth ||
            !model || !budget || !name || rows != 10 || scenes != 4 || bad) exit 1
    }
' "$test_dir/texture.csv" || fail "texture modes lost sampled-material or payload semantics"
for count in 24 96 384; do
    grep "^native-color-layout 1 $count$" "$test_dir/texture.log" >/dev/null ||
        fail "smooth texture did not supply all $count vertex colors"
done

# Lit frame trials include real light/color updates, native text/geometry HUD and swap.
# Budget metadata is a fraction of the 15 Hz frame allowance, not engine FPS.
XB_TEST_LOG="$test_dir/lit-frame.log" run_bench -winsize 320 240 -allow-fallback \
    -test frame/lighting/ -triangles 8 -seconds .001 -warmup 0 -repeats 1 \
    -csv "$test_dir/lit-frame.csv" >"$test_dir/output" 2>&1
awk '
    /^lighting-configure / { cases++ }
    /^frame-clear / { if ($2 != 1 || $3 != 1) bad++; clears++ }
    /^frame-draw / { draws++ }
    /^frame-hud / { if ($2 != 0) bad++; hud++ }
    /^frame-swap / { swaps++ }
    /^lighting-colors / { colors++; changes+=$3 }
    /^lighting-update / { lights++ }
    /^lighting-error / { bad++ }
    END {
        if (cases != 18 || !colors || !changes || !lights || hud < cases*4 ||
            clears != draws || swaps != draws || draws != hud+cases || bad) exit 1
    }
' "$test_dir/lit-frame.log" || fail "complete lit frames omitted geometry, lighting, HUD or presentation"
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); col[$i]=i }
        record=col["record"]; status=col["status"]; shade=col["shading"]
        material=col["material_sample"]; budget=col["frame_budget_ms"]
        fraction=col["frame_budget_fraction"]; total=col["total_ms"]
        present=col["present_ms"]; model=col["lighting_model"]
        ui=col["ui_kind"]; uimethod=col["ui_method"]; path=col["frame_path"]
        facets=col["triangles"]; rate=col["triangles_per_second"]
        next
    }
    $record == "summary" {
        expected_material=$shade == "smooth" ? "vertices" : "facet"
        expected_rate=$facets*1000/$total
        expected_fraction=$total*.015
        if ($status != "ok" || $material != expected_material ||
            $ui != "hud" || $uimethod != "native-text-geometry" || $path != "native-ui-swap" ||
            ($model != "lambert" && $model != "quake-like") ||
            $budget < 66.666 || $budget > 66.667 || $total <= 0 || $present < 0 ||
            $fraction < expected_fraction*.999-1e-8 || $fraction > expected_fraction*1.001+1e-8 ||
            $rate < expected_rate*.999 || $rate > expected_rate*1.001) bad++
        rows++
    }
    END {
        if (!record || !status || !shade || !material || !budget || !fraction ||
            !total || !present || !model || !ui || !uimethod || !path || !facets || !rate || rows != 18 || bad) exit 1
    }
' "$test_dir/lit-frame.csv" || fail "lit frame budget or completed-work metadata is invalid"

# A failing HUD validation must not suppress scene clear/draw/swap timings.
XB_TEST_FAIL_UI=all XB_TEST_LOG="$test_dir/scene.log" run_bench \
    -winsize 320 240 -allow-fallback -test scene/ -triangles 8 \
    -seconds .001 -warmup 0 -repeats 1 -csv "$test_dir/scene.csv" \
    >"$test_dir/output" 2>&1
awk '
    /^frame-kind / { if ($2 != 1 || phase) bad++; cases++ }
    /^frame-clear / { if ($2 != 1 || $3 != 1 || phase) bad++; phase=1; clears++ }
    /^frame-draw / { if (phase != 1) bad++; phase=2; draws++ }
    /^frame-swap / { if (phase != 2) bad++; phase=0; swaps++ }
    /^frame-hud / { bad++ }
    /^ui-(configure|draw) / { bad++ }
    /^lighting-error / { bad++ }
    END { if (cases != 6 || clears != draws || swaps != draws || !draws || phase || bad) exit 1 }
' "$test_dir/scene.log" || fail "scene-only frames omitted clear/draw/swap or depended on the HUD"
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) col[$i]=i
        record=col["record"]; status=col["status"]; path=col["frame_path"]
        total=col["total_ms"]; budget=col["frame_budget_ms"]; rate=col["completed_facets_sec"]
        facets=col["triangles"]; basis=col["rate_basis"]
        next
    }
    $record == "summary" {
        expected=$facets*1000/$total
        if ($status != "ok" || $path != "scene-swap" || $total <= 0 ||
            $budget < 66.666 || $budget > 66.667 ||
            $rate < expected*.999 || $rate > expected*1.001) bad++
        rows++
    }
    END { if (!record || !status || !path || !total || !budget || !rate || !facets || !basis || rows != 6 || bad) exit 1 }
' "$test_dir/scene.csv" || fail "scene-only completed-work metadata is invalid"
if XB_TEST_FAIL_UI=all run_bench -winsize 320 240 -allow-fallback \
    -test frame/lighting/cpu-flat/quake-point-1/depth-1 -triangles 8 \
    -seconds .001 -warmup 0 -repeats 1 -csv "$test_dir/hud-rejected.csv" \
    >"$test_dir/output" 2>&1; then
    fail "a HUD-dependent frame ignored failed HUD validation"
fi
check_lighting_rejected "$test_dir/hud-rejected.csv"

XB_TEST_LOG="$test_dir/combined-clear.log" run_bench -winsize 320 240 \
    -test overhead/color-depth-clear -seconds .001 -warmup 0 -repeats 1 \
    -csv "$test_dir/combined-clear.csv" >"$test_dir/output" 2>&1
awk '/^combined-clear / { if ($2 != 1 || $3 != 1) bad++; calls++ }
     END { if (calls < 5 || bad) exit 1 }' "$test_dir/combined-clear.log" ||
    fail "combined clear omitted color or depth in an actual clear call"

# Every blend ordering has a matched opaque control. Failed blend readback
# rejects blend timings while independent opaque cases remain usable.
XB_TEST_LOG="$test_dir/transparency.log" run_bench -winsize 320 240 \
    -test transparency/ -seconds .001 -warmup 0 -repeats 1 \
    -csv "$test_dir/transparency.csv" >"$test_dir/output" 2>&1
awk '
    /^transparency-configure / { if ($2 != 50 || ($3 != 0 && $3 != 1)) bad++; writes[$3]++; configs++ }
    /^transparency-draw / { if ($2 != 50 || ($3 != 0 && $3 != 1)) bad++; orders[$3]++ }
    END { if (configs != 4 || writes[0] != 2 || writes[1] != 2 || !orders[0] || !orders[1] || bad) exit 1 }
' "$test_dir/transparency.log" || fail "transparency sweep omitted an ordering or depth-write mode"
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) col[$i]=i
        record=col["record"]; status=col["status"]; name=col["test"]
        blend=col["transparency"]; opacity=col["opacity"]; write=col["blend_zwrite"]
        facets=col["triangles"]; base=col["base_triangles"]; layers=col["overlap_layers"]
        next
    }
    $record == "summary" {
        if ($status != "ok" || $facets != 8 || $base != 8 || $layers != 4) bad++
        if ($name ~ /\/opaque\//) {
            if ($blend != "opaque" || $opacity != "" || $write != "") bad++
            opaque++
        } else {
            if ($blend != "blended" || $opacity != .5 ||
                $write != ($name ~ /blended-zwrite/ ? 1 : 0)) bad++
            blended++
        }
    }
    END { if (!record || !status || !name || !blend || !opacity || !write || !facets || !base || !layers || opaque != 2 || blended != 4 || bad) exit 1 }
' "$test_dir/transparency.csv" || fail "transparency metadata mislabeled opacity or depth writes"
XB_TEST_FAIL_TRANSPARENCY=1 run_bench -winsize 320 240 \
    -test transparency/ -seconds .001 -warmup 0 -repeats 1 \
    -csv "$test_dir/transparency-rejected.csv" >"$test_dir/output" 2>&1
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) col[$i]=i
        record=col["record"]; status=col["status"]; blend=col["transparency"]
        kind=col["failure_kind"]; rate=col["completed_facets_sec"]
        next
    }
    $record == "summary" {
        if ($blend == "opaque") { if ($status != "ok") bad++; opaque++ }
        else {
            if ($status != "skipped" || $kind != "validation-failed" || $rate != "") bad++
            rejected++
        }
    }
    END { if (!record || !status || !blend || !kind || !rate || opaque != 2 || rejected != 4 || bad) exit 1 }
' "$test_dir/transparency-rejected.csv" || fail "blend validation failure produced a measurement or rejected opaque controls"

# Default transparency controls stay at four overlapping quads even when
# other geometry uses a different global base count. Stress uses that count.
for family in transparency transparency-stress; do
    case "$family" in transparency) expected=8 ;; *) expected=16 ;; esac
    XB_TEST_LOG="$test_dir/$family-size.log" run_bench -winsize 320 240 \
        -test "$family/" -triangles 16 -seconds .001 -warmup 0 -repeats 1 \
        -csv "$test_dir/$family-size.csv" >"$test_dir/output" 2>&1
    awk -v expected="$expected" '
        /^geometry-count / { if ($2 != expected || $3 != 1) bad++; cases++ }
        END { if (cases != 6 || bad) exit 1 }
    ' "$test_dir/$family-size.log" || fail "$family geometry ignored its intended base count"
    awk -F, -v expected="$expected" '
        NR == 1 {
            for (i=1; i<=NF; ++i) col[$i]=i
            record=col["record"]; facets=col["triangles"]; base=col["base_triangles"]
            layers=col["overlap_layers"]; next
        }
        $record == "summary" {
            if ($facets != expected || $base != expected || $layers != expected/2) bad++
            rows++
        }
        END { if (!record || !facets || !base || !layers || rows != 6 || bad) exit 1 }
    ' "$test_dir/$family-size.csv" || fail "$family CSV mislabeled effective count or overlapping quads"
done
for selected in transparency-stress/opaque/near-to-far submit/multi_polygon/batch-128; do
    run_bench -winsize 320 240 -test "$selected" -seconds .001 -warmup 0 -repeats 1 \
        -csv "$test_dir/global-base.csv" >"$test_dir/output" 2>&1
    awk -F, '
        NR == 1 {
            for (i=1; i<=NF; ++i) col[$i]=i
            record=col["record"]; facets=col["triangles"]; base=col["base_triangles"]; next
        }
        $record == "summary" { if ($facets != 512 || $base != 512) bad++; rows++ }
        END { if (!record || !facets || !base || rows != 1 || bad) exit 1 }
    ' "$test_dir/global-base.csv" || fail "global base count changed from 512"
done

# Native HUD/menu/console workloads must configure only their own UI and draw
# through the native UI API. Bitmap-copy benchmark APIs are absent.
XB_TEST_LOG="$test_dir/ui.log" run_bench -winsize 320 240 -test ui/ \
    -seconds .001 -warmup 0 -repeats 1 -csv "$test_dir/ui.csv" >"$test_dir/output" 2>&1
awk '
    /^ui-configure / { if ($2 < 0 || $2 > 2) bad++; configured[$2]++; mode=$2; cases++ }
    /^ui-draw / { if ($2 != mode || $3 < 0) bad++; draws[$2]++ }
    /^ui-swap / { swaps++ }
    /^frame-(draw|hud|swap) / { bad++ }
    END {
        for (i=0; i<3; ++i) if (configured[i] != 1 || draws[i] < 5) bad++
        if (cases != 3 || swaps != 3 || bad) exit 1
    }
' "$test_dir/ui.log" || fail "native UI scheduling omitted a workload or presented inside a render-only trial"
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) col[$i]=i
        record=col["record"]; status=col["status"]; name=col["test"]; path=col["frame_path"]
        kind=col["ui_kind"]; method=col["ui_method"]; present=col["present_ms"]; budget=col["frame_budget_ms"]
        next
    }
    $record == "summary" {
        expected=$name; gsub(/"/, "", expected); sub(/^ui\//, "", expected)
        if ($status != "ok" || $path != "ui-only" || $kind != expected ||
            $method != "native-text-geometry" || $present != 0 || $budget != "") bad++
        rows++
    }
    END { if (!record || !status || !name || !path || !kind || !method || !present || !budget || rows != 3 || bad) exit 1 }
' "$test_dir/ui.csv" || fail "native UI CSV is confused with the retired raster path"
XB_TEST_FAIL_UI=menu run_bench -winsize 320 240 -test ui/ \
    -seconds .001 -warmup 0 -repeats 1 -csv "$test_dir/ui-rejected.csv" >"$test_dir/output" 2>&1
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) col[$i]=i
        record=col["record"]; status=col["status"]; kind=col["ui_kind"]; failure=col["failure_kind"]
        rate=col["completed_facets_sec"]; next
    }
    $record == "summary" {
        if ($kind == "menu") {
            if ($status != "skipped" || $failure != "validation-failed" || $rate != "") bad++
            rejected++
        } else { if ($status != "ok") bad++; accepted++ }
    }
    END { if (!record || !status || !kind || !failure || !rate || rejected != 1 || accepted != 2 || bad) exit 1 }
' "$test_dir/ui-rejected.csv" || fail "failed native UI validation affected independent UI workloads"
if XB_TEST_FAIL_UI_DRAW=3 run_bench -winsize 320 240 \
    -test frame/lighting/cpu-flat/quake-point-1/depth-1 -triangles 8 \
    -seconds .001 -warmup 0 -repeats 1 -csv "$test_dir/ui-draw-failed.csv" \
    >"$test_dir/output" 2>&1; then
    fail "failed native HUD drawing became a valid complete frame"
fi
check_lighting_rejected "$test_dir/ui-draw-failed.csv"
XB_TEST_FAIL_UI=all XB_TEST_LOG="$test_dir/ui-unneeded.log" run_bench \
    -winsize 320 240 -test pipeline/serialized/quake-cpu-flat/depth-0 -triangles 8 \
    -seconds .001 -warmup 0 -repeats 1 >"$test_dir/output" 2>&1
if grep '^ui-' "$test_dir/ui-unneeded.log" >/dev/null; then
    fail "a no-UI pipeline allocated or drew a native UI workload"
fi

# Independent polygons and polygon batches use matching lighting workloads;
# their API call counts must reflect the actual submitted primitive method.
run_bench -winsize 320 240 -allow-fallback -test lighting/submit/ \
    -triangles 8 -seconds .001 -warmup 0 -repeats 1 \
    -csv "$test_dir/lit-submit.csv" >"$test_dir/output" 2>&1
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); col[$i]=i }
        record=col["record"]; status=col["status"]; method=col["method"]
        facets=col["triangles"]; calls=col["calls"]; batch=col["batch"]
        next
    }
    $record == "summary" {
        if ($status != "ok" || $facets != 32) bad++
        if ($method == "polygon") { if ($calls != $facets) bad++; polygons++ }
        else if ($method == "multi_polygon") {
            if ($calls != int(($facets+$batch-1)/$batch)) bad++
            batches++
        } else bad++
    }
    END {
        if (!record || !status || !method || !facets || !calls || !batch ||
            polygons != 4 || batches != 4 || bad) exit 1
    }
' "$test_dir/lit-submit.csv" || fail "lit polygon/batch comparison lost submission semantics"

# CPU Quake-like updates retain independently owned facet or vertex arrays in
# each outstanding slot. Both serialized and streaming variants fully drain.
for path in cpu-flat cpu-smooth; do
    case "$path" in *smooth) smooth=1; count=96 ;; *) smooth=0; count=32 ;; esac
    for mode in serialized streaming; do
        case "$mode" in streaming) queue=8 ;; *) queue=1 ;; esac
        log="$test_dir/quake-stream-$path-$mode.log"
        csv="$test_dir/quake-stream-$path-$mode.csv"
        XB_TEST_LOG="$log" run_bench -winsize 320 240 -allow-fallback \
            -test "pipeline/$mode/quake-$path/depth-1" -triangles 8 -queue 8 \
            -seconds .003 -warmup 0 -repeats 1 -csv "$csv" >"$test_dir/output" 2>&1
        check_queue_log "$log" "$queue"
        check_pipeline_csv "$csv" "$queue"
        check_lighting_log "$log" 0 "$smooth" 1
        grep "^queue-color-layout $smooth $count$" "$log" >/dev/null ||
            fail "Quake stream retained the wrong number of color samples"
        awk -F, -v smooth="$smooth" '
            NR == 1 {
                for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); col[$i]=i }
                model=col["lighting_model"]; material=col["material_sample"]
                budget=col["frame_budget_ms"]; next
            }
            {
                if ($model != "quake-like" || $budget != "" ||
                    $material != (smooth ? "vertices" : "facet")) bad++
                rows++
            }
            END { if (!model || !material || !budget || rows != 2 || bad) exit 1 }
        ' "$csv" || fail "Quake stream mislabeled lighting, material or no-swap timing"
    done
done

# A final completion fault must also invalidate the new smooth stream path.
fault_case native-error pipeline/streaming/quake-cpu-smooth/depth-0 17

# Inspect the input winding as well as the requested culling state; merely
# changing a case label cannot turn a front-facing mesh into a mixed fixture.
XB_TEST_LOG="$test_dir/culling.log" run_bench -winsize 320 240 -allow-fallback \
    -test cull/ -triangles 8 -seconds .001 -warmup 0 -repeats 1 \
    -csv "$test_dir/culling.csv" >"$test_dir/output" 2>&1
awk '
    /^culling-configure / { if ($2 == 0) off++; else if ($2 == 1) back++; else bad++ }
    /^culling-winding / {
        if ($2 == 4 && $3 == 4) mixed++
        else if ($2 == 0 && $3 == 8) front++
        else bad++
    }
    END { if (off != 2 || back != 2 || mixed != 2 || front != 2 || bad) exit 1 }
' "$test_dir/culling.log" || fail "culling comparison did not change winding and rejection state"
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); col[$i]=i }
        status=col["status"]; record=col["record"]; cull=col["cull"]; winding=col["winding"]
        next
    }
    $record == "summary" {
        if ($status != "ok" || ($cull != "off" && $cull != "back") ||
            ($winding != "front" && $winding != "mixed")) bad++
        pairs[$cull "/" $winding]++
    }
    END {
        if (!status || !record || !cull || !winding || bad ||
            pairs["off/front"] != 1 || pairs["back/front"] != 1 ||
            pairs["off/mixed"] != 1 || pairs["back/mixed"] != 1) exit 1
    }
' "$test_dir/culling.csv" || fail "culling CSV lost rejection/winding metadata"

# Native texture cases use XYZUV corner payloads and build their texture once
# before the first draw. CPU-sampled controls use the same image without native
# mapping. The test double verifies scheduling, never actual texture pixels.
XB_TEST_LOG="$test_dir/mapping.log" run_bench -winsize 320 240 \
    -test texture-map/ -triangles 8 -seconds .001 -warmup 0 -repeats 1 \
    -csv "$test_dir/mapping.csv" >"$test_dir/output" 2>&1
awk '
    /^texture-configure / {
        if (configured && (!draws || (filter && (setups != 1 || payloads != 1)))) bad++
        filter=$2; configured++; draws=0; setups=0; payloads=0
        if ($3 != 1 || filter < 0 || filter > 3) bad++
        if (filter) mapped++; else controls++
    }
    /^texture-setup / {
        if (!filter || $2 != filter || $3 != 64 || draws || setups) bad++
        setups++
    }
    /^texture-payload / {
        if (!filter || !setups || draws || $3 != 5 || ($2 != 24 && $2 != 96 && $2 != 384)) bad++
        payloads++
    }
    /^texture-draw / {
        if ($2 != (filter != 0) || $3 != filter || (filter && (!setups || !payloads))) bad++
        draws++
    }
    /^texture-hud / { if ($2 != 0 || $3 != 0 || !draws) bad++; hud++ }
    /^texture-transform / { if ($2 != 1 || $3 != filter) bad++; motion[filter]++ }
    /^texture-error / { bad++ }
    END {
        for (f=0; f<=3; ++f) if (!motion[f]) bad++
        if (bad || configured != 35 || mapped != 21 || controls != 14 || !hud || !draws ||
            (filter && (setups != 1 || payloads != 1))) exit 1
    }
' "$test_dir/mapping.log" || fail "native mapping setup, payload, HUD isolation or moving transforms are invalid"
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); col[$i]=i }
        record=col["record"]; status=col["status"]; test=col["test"]
        mapping=col["texture_mapping"]; filter=col["texture_filter"]
        execution=col["texture_execution"]; width=col["texture_width"]; height=col["texture_height"]
        material=col["material_sample"]; shade=col["shading"]; model=col["lighting_model"]
        payload=col["input_payload_bytes"]; facets=col["triangles"]; budget=col["frame_budget_ms"]
        reason=col["reason"]; next
    }
    $record == "summary" {
        name=$test; gsub(/"/, "", name); split(name, part, "/")
        native=part[2] == "point" || part[2] == "bilinear" || part[2] == "trilinear"
        if ($status != "ok" || $model != "none" || $payload <= 0) bad++
        if (native) {
            if ($mapping != "native-xgl" || $filter != part[2] || $execution != "unclassified" ||
                $width != 64 || $height != 64 || $material != "texels" || $shade != "flat" ||
                $payload != $facets*60 || $reason !~ /unclassified/) bad++
            mapped++
        } else {
            if ($mapping != "none" || $filter != "off" || $execution != "none" ||
                $width != "" || $height != "" ||
                $material != (part[2] == "vertex-smooth" ? "vertices" : "facet")) bad++
            controls++
        }
        if (part[3] == "frame") { if ($budget < 66.666 || $budget > 66.667) bad++; frames++ }
        else if ($budget != "") bad++
    }
    END {
        if (!record || !status || !test || !mapping || !filter || !execution || !width || !height ||
            !material || !shade || !model || !payload || !facets || !budget || !reason ||
            mapped != 21 || controls != 14 || frames != 15 || bad) exit 1
    }
' "$test_dir/mapping.csv" || fail "native mapping report claimed acceleration or mislabeled controls/payloads"

# Unsupported filtering remains a skipped mapping result, while other filters
# and ordinary CPU controls still complete. Never relabel an untextured draw
# as successful native texture mapping.
XB_TEST_FAIL_TEXTURE=bilinear run_bench -winsize 320 240 \
    -test texture-map/ -triangles 8 -seconds .001 -warmup 0 -repeats 1 \
    -csv "$test_dir/mapping-unavailable.csv" >"$test_dir/output" 2>&1
awk -F, '
    NR == 1 {
        for (i=1; i<=NF; ++i) { gsub(/"/, "", $i); col[$i]=i }
        record=col["record"]; status=col["status"]; filter=col["texture_filter"]
        mapping=col["texture_mapping"]; total=col["total_ms"]; reason=col["reason"]
        next
    }
    $filter == "bilinear" {
        if ($record != "summary" || $status != "skipped" || $mapping != "native-xgl" ||
            $total != 0 || $reason !~ /injected texture mapping unavailable/) bad++
        rejected++; next
    }
    $record == "summary" { if ($status != "ok") bad++; accepted++ }
    END {
        if (!record || !status || !filter || !mapping || !total || !reason ||
            rejected != 7 || accepted != 28 || bad) exit 1
    }
' "$test_dir/mapping-unavailable.csv" || fail "unavailable mapping produced timings or suppressed independent controls"

if XB_TEST_FAIL_TEXTURE=all run_bench -winsize 320 240 \
    -test texture-map/point/draw/depth-0 -triangles 8 -seconds .001 -warmup 0 -repeats 1 \
    -csv "$test_dir/mapping-rejected.csv" >"$test_dir/output" 2>&1; then
    fail "all-unavailable texture mapping run reported success"
fi
check_lighting_rejected "$test_dir/mapping-rejected.csv"
grep 'injected texture mapping unavailable' "$test_dir/output" >/dev/null ||
    fail "native texture setup failure reason was lost"
fault_case native-error texture-map/point/draw/depth-0 5

# Categories come from structured backend outcomes, never wording guesses.
check_failure_kind()
{
    awk -F, -v expected="$2" '
        NR == 1 {
            columns=NF
            for (i=1; i<=NF; ++i) col[$i]=i
            record=col["record"]; status=col["status"]; kind=col["failure_kind"]
            next
        }
        {
            if (NF != columns || $record != "summary" || $status != "skipped" || $kind != expected) bad++
            rows++
        }
        END { if (!record || !status || !kind || rows != 1 || bad) exit 1 }
    ' "$1" || fail "incorrect failure category: $2"
}
check_failure_kind "$test_dir/skipped.csv" validation-failed
check_failure_kind "$test_dir/light-validation-failure.csv" validation-failed
check_failure_kind "$test_dir/native-error.csv" api-error
check_failure_kind "$test_dir/mapping-rejected.csv" unsupported
check_failure_kind "$test_dir/light-update-failure.csv" error

# Counters count only timed items and distinguish changed from cached work.
# Reported conditional preparation means must agree with each trial mean.
awk -F, '
    FNR == 1 {
        columns=NF
        for (i=1; i<=NF; ++i) col[$i]=i
        status=col["status"]; record=col["record"]; name=col["test"]; width=col["width"]
        frames=col["frames_all_trials"]; colors=col["color_update_items"]; lights=col["light_update_items"]
        updated=col["updated_color_light_items"]; cached=col["cached_color_light_items"]
        uprep=col["updated_prepare_ms"]; cprep=col["cached_prepare_ms"]; prep=col["prepare_ms"]
        motion=col["light_motion"]; styles=col["style_layers"]; model=col["lighting_model"]
        fps=col["sim_fps"]; complete=col["completed_facets_sec"]; legacy=col["triangles_per_second"]
        total=col["total_ms"]; render=col["render_ms"]; facets=col["triangles"]; basis=col["rate_basis"]
        current=col["native_current_bytes"]; peak=col["native_peak_bytes"]
        if (!status || !record || !name || !width || !frames || !colors || !lights || !updated ||
            !cached || !uprep || !cprep || !prep || !motion || !styles || !model || !fps ||
            !complete || !legacy || !total || !render || !facets || !basis || !current || !peak) bad++
        next
    }
    {
        if (NF != columns) bad++
        if ($status != "ok") {
            if ($record != "summary" || $complete != "" || $colors != "" || $lights != "" ||
                $updated != "" || $cached != "" || $uprep != "" || $cprep != "") bad++
            rejected++; next
        }
        if ($current < 0 || $peak < $current || $fps < 1 || $fps > 240) bad++
        expected=$facets && $total > 0 ? $facets*1000/$total : 0
        if ($complete < expected*.999-.001 || $complete > expected*1.001+.001) bad++
        divisor=$basis == "total_ms" ? $total : $render
        if ($basis != "total_ms" && $basis != "render_ms") bad++
        expected=$facets && divisor > 0 ? $facets*1000/divisor : 0
        if ($legacy < expected*.999-.001 || $legacy > expected*1.001+.001) bad++
        if ($colors == "") {
            if ($lights != "" || $updated != "" || $cached != "" || $uprep != "" || $cprep != "") bad++
            next
        }
        count=$updated+$cached
        if (count <= 0 || $colors < 0 || $lights < 0 || $cached < 0 ||
            $updated < $colors || $updated < $lights || $updated > $colors+$lights ||
            ($updated > 0 ? $uprep == "" || $uprep < 0 : $uprep != "") ||
            ($cached > 0 ? $cprep == "" || $cprep < 0 : $cprep != "")) bad++
        if ($motion && (count % 8 || $cached || $updated != count)) bad++
        if ($model == "baked" && $name ~ /\/styles-[14]\//) {
            cadence=$fps == 60 ? 6 : 3
            if (count % cadence || !$updated || !$cached) bad++
        }
        key=FILENAME ":" $name ":" $width
        if ($record == "trial") {
            sumcolors[key]+=$colors; sumlights[key]+=$lights
            sumupdated[key]+=$updated; sumcached[key]+=$cached
            expected=($uprep*$updated+$cprep*$cached)/count
            if ($prep < expected*.999-.000001 || $prep > expected*1.001+.000001) bad++
            trials++
        } else if ($record == "summary") {
            if ($colors != sumcolors[key] || $lights != sumlights[key] ||
                $updated != sumupdated[key] || $cached != sumcached[key] || count != $frames) bad++
            summaries++
        } else bad++
    }
    END { if (!trials || !summaries || !rejected || bad) exit 1 }
' "$test_dir"/*.csv || fail "CSV completion, update/cached counts, preparation means or memory bounds are inconsistent"

# Queue memory reports this mock actual allocated RGB slots. Increasing the
# queue must increase the peak, without claims about native driver storage.
for queue in 1 2 8; do
    awk -F, -v queue="$queue" '
        NR == 1 {
            for (i=1; i<=NF; ++i) col[$i]=i
            record=col["record"]; peak=col["native_peak_bytes"]; current=col["native_current_bytes"]
            facets=col["triangles"]; next
        }
        $record == "summary" {
            if ($peak != queue*$facets*12 || $current != $peak) bad++
            rows++
        }
        END { if (!record || !peak || !current || !facets || rows != 1 || bad) exit 1 }
    ' "$test_dir/queue-$queue-relight.csv" || fail "mock queue memory did not reflect allocated RGB slots"
done

echo 'CLI tests: arguments, resize, geometry, queues, sampled materials, lighting, full frames, culling, native texture scheduling, scene/HUD isolation, transparency, counters, memory and fault rejection passed'
