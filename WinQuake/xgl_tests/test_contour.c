/* Synthetic material/geometry checks, not a Sun XGL rasterizer or benchmark.
 * The comparison baseline follows the existing depth-two quad subdivision
 * and opaque mip-average/bilinear vertex sampling, with lighting and gamma
 * held at identity so only the representation of texture shapes is compared.
 */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int tc_fail_after = -1, tc_live_allocations;
static size_t tc_peak_allocation;
static void *tc_malloc(size_t bytes);
static void tc_free(void *memory);
#define malloc tc_malloc
#define free tc_free
#include "../xgl_contour.c"
#undef malloc
#undef free

#define TC_SIZE 64
#define TC_IMAGE 160
#define TC_BUDGET 32
#define TC_LEVELS 7

static unsigned char tc_pixels[TC_SIZE * TC_SIZE], tc_palette[768];
static unsigned char tc_mips[TC_LEVELS][TC_SIZE * TC_SIZE * 3];
static int tc_mip_width[TC_LEVELS], tc_mip_count, tc_services, tc_region_colors;
static float tc_map[2][3];
static xgl_contour_point_t tc_face[4];
static xgl_contour_vertex_t tc_contour[XGL_CONTOUR_MAX_TRIANGLES * 3];
static xgl_contour_vertex_t tc_uniform[TC_BUDGET * 3];
static unsigned char tc_images[3][TC_IMAGE * TC_IMAGE * 3];
static const char *tc_dump_directory;

static void *tc_malloc(size_t bytes)
{
    void *memory;
    if (tc_fail_after == 0) return NULL;
    if (tc_fail_after > 0) --tc_fail_after;
    memory = malloc(bytes);
    if (memory) {
        ++tc_live_allocations;
        if (bytes > tc_peak_allocation) tc_peak_allocation = bytes;
    }
    return memory;
}

static void tc_free(void *memory)
{
    if (memory) --tc_live_allocations;
    free(memory);
}

static void tc_service(void) { ++tc_services; }

static void tc_setup(void)
{
    static const float corners[4][2] = {{0,0}, {1,0}, {1,1}, {0,1}};
    int i, axis;
    memset(tc_palette, 0, sizeof(tc_palette));
    tc_palette[0] = 20; tc_palette[1] = 28; tc_palette[2] = 36;
    tc_palette[3] = 224; tc_palette[4] = 204; tc_palette[5] = 168;
    tc_palette[6] = 144; tc_palette[7] = 108; tc_palette[8] = 72;
    memset(tc_map, 0, sizeof(tc_map));
    tc_map[0][1] = tc_map[1][2] = 1;
    for (i = 0; i < 4; ++i) {
        tc_face[i].position[0] = corners[i][0];
        tc_face[i].position[1] = corners[i][1];
        tc_face[i].position[2] = 3 + corners[i][0] * .25f - corners[i][1] * .5f;
        for (axis = 0; axis < 2; ++axis) tc_face[i].st[axis] = corners[i][axis];
    }
    tc_fail_after = -1;
    assert(tc_live_allocations == 0);
    assert(tc_peak_allocation <= 128UL * 1024UL);
}

static void tc_apply_map(void)
{
    int i, axis;
    for (i = 0; i < 4; ++i)
        for (axis = 0; axis < 2; ++axis)
            tc_face[i].st[axis] = tc_map[axis][0] +
                tc_map[axis][1] * tc_face[i].position[0] +
                tc_map[axis][2] * tc_face[i].position[1];
}

static void tc_pattern(int pattern)
{
    int x, y, row, column, index;
    float dx, dy;
    tc_region_colors = pattern == 2 ? 3 : 2;
    for (y = 0; y < TC_SIZE; ++y)
        for (x = 0; x < TC_SIZE; ++x) {
            index = 0;
            if (pattern == 0) index = x >= 23;
            else if (pattern == 1) index = x * 5 + y * 3 >= 211;
            else if (pattern == 2) {
                row = y / 16; column = (x + ((row & 1) ? 16 : 0)) % 32;
                if (y % 16 >= 4 && column >= 4) index = 1 + (row & 1);
            } else if (pattern == 3) {
                dx = (x + .5f) / TC_SIZE - .59f;
                dy = (y + .5f) / TC_SIZE - .43f;
                index = dx * dx + dy * dy < .267f * .267f;
            } else if (pattern == 4) {
                dx = (x + .5f) / TC_SIZE - .5f;
                dy = (y + .5f) / TC_SIZE - .51f;
                index = fabs(dx) < .28f && (dy >= 0 || dx * dx + dy * dy < .28f * .28f);
            } else if (pattern == 5) index = 1;
            else if (pattern == 6) index = ((x * 1103515245UL + y * 12345UL) >> 13) & 1;
            tc_pixels[y * TC_SIZE + x] = (unsigned char)index;
        }
}

static void tc_source(float s, float t, float rgb[3])
{
    int x, y, index, channel;
    s -= (float)floor(s); t -= (float)floor(t);
    x = (int)(s * TC_SIZE); y = (int)(t * TC_SIZE);
    if (x >= TC_SIZE) x = TC_SIZE - 1;
    if (y >= TC_SIZE) y = TC_SIZE - 1;
    index = tc_pixels[y * TC_SIZE + x];
    for (channel = 0; channel < 3; ++channel) rgb[channel] = tc_palette[index * 3 + channel];
}

static void tc_build_mips(void)
{
    int i, level, width, next, x, y, c, sum;
    for (i = 0; i < TC_SIZE * TC_SIZE; ++i)
        memcpy(tc_mips[0] + i * 3, tc_palette + tc_pixels[i] * 3, 3);
    tc_mip_width[0] = TC_SIZE;
    width = TC_SIZE;
    for (level = 1; width > 1; ++level) {
        next = width / 2;
        tc_mip_width[level] = next;
        for (y = 0; y < next; ++y)
            for (x = 0; x < next; ++x)
                for (c = 0; c < 3; ++c) {
                    sum = tc_mips[level - 1][((y * 2) * width + x * 2) * 3 + c] +
                          tc_mips[level - 1][((y * 2) * width + x * 2 + 1) * 3 + c] +
                          tc_mips[level - 1][((y * 2 + 1) * width + x * 2) * 3 + c] +
                          tc_mips[level - 1][((y * 2 + 1) * width + x * 2 + 1) * 3 + c];
                    tc_mips[level][(y * next + x) * 3 + c] = (unsigned char)((sum + 2) / 4);
                }
        width = next;
    }
    tc_mip_count = level;
}

static void tc_filtered(float s, float t, float fs, float ft, unsigned char rgb[3])
{
    float extent, fx, fy, a, b, raw[3];
    int level, width, x0, x1, y0, y1, c;
    const unsigned char *pixels;
    extent = (fs > ft ? fs : ft) * TC_SIZE;
    if (extent < 2) {
        tc_source(s, t, raw);
        for (c = 0; c < 3; ++c) rgb[c] = (unsigned char)raw[c];
        return;
    }
    level = 1;
    while (extent >= 4 && level + 1 < tc_mip_count) { extent *= .5f; ++level; }
    width = tc_mip_width[level]; pixels = tc_mips[level];
    s -= (float)floor(s); t -= (float)floor(t);
    fx = s * width - .5f; fy = t * width - .5f;
    x0 = (int)floor(fx); y0 = (int)floor(fy);
    fx -= x0; fy -= y0;
    if (x0 < 0) x0 += width;
    if (y0 < 0) y0 += width;
    x1 = (x0 + 1) % width; y1 = (y0 + 1) % width;
    for (c = 0; c < 3; ++c) {
        a = pixels[(y0 * width + x0) * 3 + c] * (1 - fx) + pixels[(y0 * width + x1) * 3 + c] * fx;
        b = pixels[(y1 * width + x0) * 3 + c] * (1 - fx) + pixels[(y1 * width + x1) * 3 + c] * fx;
        rgb[c] = (unsigned char)(a * (1 - fy) + b * fy + .5f);
    }
}

static int tc_uniform_split(const xgl_contour_point_t *a, const xgl_contour_point_t *b,
                            const xgl_contour_point_t *c, int depth, int count)
{
    xgl_contour_point_t ab, bc, ca;
    float low, high, footprint[2];
    int axis, i;
    const xgl_contour_point_t *points[3];
    if (depth) {
        for (i = 0; i < 3; ++i) {
            ab.position[i] = (a->position[i] + b->position[i]) * .5f;
            bc.position[i] = (b->position[i] + c->position[i]) * .5f;
            ca.position[i] = (c->position[i] + a->position[i]) * .5f;
        }
        for (i = 0; i < 2; ++i) {
            ab.st[i] = (a->st[i] + b->st[i]) * .5f;
            bc.st[i] = (b->st[i] + c->st[i]) * .5f;
            ca.st[i] = (c->st[i] + a->st[i]) * .5f;
        }
        count = tc_uniform_split(a, &ab, &ca, depth - 1, count);
        count = tc_uniform_split(&ab, b, &bc, depth - 1, count);
        count = tc_uniform_split(&ca, &bc, c, depth - 1, count);
        return tc_uniform_split(&ab, &bc, &ca, depth - 1, count);
    }
    assert(count < TC_BUDGET);
    points[0] = a; points[1] = b; points[2] = c;
    for (axis = 0; axis < 2; ++axis) {
        low = high = points[0]->st[axis];
        for (i = 1; i < 3; ++i) {
            if (points[i]->st[axis] < low) low = points[i]->st[axis];
            if (points[i]->st[axis] > high) high = points[i]->st[axis];
        }
        footprint[axis] = (high - low) / 3;
    }
    for (i = 0; i < 3; ++i) {
        tc_uniform[count * 3 + i].point = *points[i];
        tc_filtered(points[i]->st[0], points[i]->st[1], footprint[0], footprint[1],
                    tc_uniform[count * 3 + i].color);
    }
    return count + 1;
}

static void tc_baseline(void)
{
    int count;
    tc_build_mips();
    count = tc_uniform_split(&tc_face[0], &tc_face[1], &tc_face[2], 2, 0);
    count = tc_uniform_split(&tc_face[0], &tc_face[2], &tc_face[3], 2, count);
    assert(count == TC_BUDGET);
}

static double tc_area(const float *a, const float *b, const float *c)
{
    return ((double)b[0] - a[0]) * ((double)c[1] - a[1]) -
           ((double)b[1] - a[1]) * ((double)c[0] - a[0]);
}

static int tc_barycentric(const xgl_contour_vertex_t *triangle, float x, float y,
                          double weights[3])
{
    float point[3];
    double area;
    point[0] = x; point[1] = y; point[2] = 0;
    area = tc_area(triangle[0].point.position, triangle[1].point.position, triangle[2].point.position);
    if (fabs(area) < 1e-12) return 0;
    weights[0] = tc_area(point, triangle[1].point.position, triangle[2].point.position) / area;
    weights[1] = tc_area(triangle[0].point.position, point, triangle[2].point.position) / area;
    weights[2] = 1 - weights[0] - weights[1];
    return weights[0] >= -1e-7 && weights[1] >= -1e-7 && weights[2] >= -1e-7;
}

static int tc_render(const xgl_contour_vertex_t *mesh, int count, float x, float y, float rgb[3])
{
    double weights[3];
    int i, c, j;
    for (i = 0; i < count; ++i) {
        if (!tc_barycentric(mesh + i * 3, x, y, weights)) continue;
        for (c = 0; c < 3; ++c) {
            rgb[c] = 0;
            for (j = 0; j < 3; ++j) rgb[c] += (float)weights[j] * mesh[i * 3 + j].color[c];
        }
        return 1;
    }
    return 0;
}

static void tc_coverage(int count)
{
    int i, j, axis, x, y, hits, on_edge;
    double area, sign, weights[3], total;
    float px, py, expected;
    sign = tc_area(tc_face[0].position, tc_face[1].position, tc_face[2].position);
    total = 0;
    assert(count > 0 && count <= TC_BUDGET);
    for (i = 0; i < count; ++i) {
        area = tc_area(tc_contour[i*3].point.position, tc_contour[i*3+1].point.position,
                       tc_contour[i*3+2].point.position);
        assert(area * sign > 1e-12);
        total += area;
        for (j = 0; j < 3; ++j) {
            const xgl_contour_point_t *point;
            point = &tc_contour[i * 3 + j].point;
            assert(point->position[0] >= -1e-6 && point->position[0] <= 1.000001);
            assert(point->position[1] >= -1e-6 && point->position[1] <= 1.000001);
            expected = 3 + point->position[0] * .25f - point->position[1] * .5f;
            assert(fabs(point->position[2] - expected) < 2e-5);
            for (axis = 0; axis < 2; ++axis) {
                expected = tc_map[axis][0] + tc_map[axis][1] * point->position[0] +
                           tc_map[axis][2] * point->position[1];
                assert(fabs(point->st[axis] - expected) < 2e-5);
            }
        }
    }
    assert(fabs(total - (sign > 0 ? 2 : -2)) < 2e-5);
    /* Area alone could conceal holes balanced by overlap. Jittered probes
     * verify a single covering interior triangle, excluding exact edges. */
    for (y = 0; y < 61; ++y)
        for (x = 0; x < 67; ++x) {
            px = (x + .317f) / 67; py = (y + .619f) / 61;
            hits = on_edge = 0;
            for (i = 0; i < count; ++i)
                if (tc_barycentric(tc_contour + i * 3, px, py, weights)) {
                    ++hits;
                    if (weights[0] < 1e-6 || weights[1] < 1e-6 || weights[2] < 1e-6) on_edge = 1;
                }
            assert(hits >= 1);
            if (!on_edge) assert(hits == 1);
        }
}

static int tc_nearest(const float rgb[3])
{
    int index, channel, best;
    double distance, low, difference;
    low = 1e30; best = 0;
    for (index = 0; index < tc_region_colors; ++index) {
        distance = 0;
        for (channel = 0; channel < 3; ++channel) {
            difference = rgb[channel] - tc_palette[index * 3 + channel];
            distance += difference * difference;
        }
        if (distance < low) { low = distance; best = index; }
    }
    return best;
}

static void tc_dump(const char *name, int count)
{
    static const char *kinds[3] = {"source", "uniform", "contour"};
    char path[1024];
    FILE *file;
    int image, i, j;
    if (!tc_dump_directory) return;
    assert(strlen(tc_dump_directory) + strlen(name) + 40 < sizeof(path));
    for (image = 0; image < 3; ++image) {
        sprintf(path, "%s/%s-%s.ppm", tc_dump_directory, name, kinds[image]);
        file = fopen(path, "wb"); assert(file);
        fprintf(file, "P6\n%d %d\n255\n", TC_IMAGE, TC_IMAGE);
        assert(fwrite(tc_images[image], 1, sizeof(tc_images[image]), file) == sizeof(tc_images[image]));
        assert(!fclose(file));
    }
    sprintf(path, "%s/%s-mesh.txt", tc_dump_directory, name);
    file = fopen(path, "w"); assert(file);
    fprintf(file, "%d\n", count);
    for (i = 0; i < count; ++i) {
        for (j = 0; j < 3; ++j)
            fprintf(file, "%.9g %.9g ", tc_contour[i*3+j].point.position[0], tc_contour[i*3+j].point.position[1]);
        fprintf(file, "\n");
    }
    assert(!fclose(file));
}

static void tc_quality(const char *name, int count, int require_improvement)
{
    double errors[2], mismatches[2];
    int x, y, c, which, offset;
    float px, py, s, t, source[3], rendered[2][3];
    errors[0] = errors[1] = mismatches[0] = mismatches[1] = 0;
    tc_baseline();
    for (y = 0; y < TC_IMAGE; ++y)
        for (x = 0; x < TC_IMAGE; ++x) {
            px = (x + .417f) / TC_IMAGE; py = (y + .637f) / TC_IMAGE;
            s = tc_map[0][0] + tc_map[0][1] * px + tc_map[0][2] * py;
            t = tc_map[1][0] + tc_map[1][1] * px + tc_map[1][2] * py;
            tc_source(s, t, source);
            assert(tc_render(tc_uniform, TC_BUDGET, px, py, rendered[0]));
            assert(tc_render(tc_contour, count, px, py, rendered[1]));
            for (which = 0; which < 2; ++which) {
                for (c = 0; c < 3; ++c) errors[which] += fabs(rendered[which][c] - source[c]);
                mismatches[which] += tc_nearest(rendered[which]) != tc_nearest(source);
            }
            offset = (y * TC_IMAGE + x) * 3;
            for (c = 0; c < 3; ++c) {
                tc_images[0][offset+c] = (unsigned char)source[c];
                tc_images[1][offset+c] = (unsigned char)(rendered[0][c] + .5f);
                tc_images[2][offset+c] = (unsigned char)(rendered[1][c] + .5f);
            }
        }
    for (which = 0; which < 2; ++which) {
        errors[which] /= TC_IMAGE * TC_IMAGE * 3;
        mismatches[which] *= 100.0 / (TC_IMAGE * TC_IMAGE);
    }
    printf("contour %-10s: %2d/%d triangles; RGB MAE %.3f -> %.3f; region mismatch %.2f%% -> %.2f%%\n",
           name, count, TC_BUDGET, errors[0], errors[1], mismatches[0], mismatches[1]);
    if (require_improvement) {
        assert(errors[1] < errors[0] * .85);
        assert(mismatches[1] < mismatches[0]);
    }
    tc_dump(name, count);
}

static int tc_build(int budget, xgl_contour_stats_t *stats)
{
    int count;
    memset(tc_contour, 0x5a, sizeof(tc_contour));
    memset(stats, 0, sizeof(*stats)); tc_services = 0;
    count = XGL_ContourBuild(tc_face, 4, tc_pixels, TC_SIZE, TC_SIZE, tc_palette,
                            budget, tc_contour, stats, tc_service);
    assert(tc_live_allocations == 0);
    assert(count >= 0 && count <= budget && count <= XGL_CONTOUR_MAX_TRIANGLES);
    assert(stats->samples <= 32768 && stats->evaluations <= 150000);
    return count;
}

static float tc_first_foreground(int count, float x)
{
    float y, rgb[3];
    int row;
    for (row = 0; row < TC_IMAGE; ++row) {
        y = (row + .5f) / TC_IMAGE;
        assert(tc_render(tc_contour, count, x, y, rgb));
        if (tc_nearest(rgb) == 1) return y;
    }
    return 1;
}

static void tc_shape_landmarks(int pattern, int count)
{
    float center, left, right, angle, rgb[3], x, y;
    int i, j, distinct;
    if (pattern == 0) {
        /* Shared geometry may carry different material colors on opposite
         * sides, so native interpolation cannot smear the sharp border. */
        distinct = 0;
        for (i = 0; i < count * 3; ++i)
            for (j = i + 1; j < count * 3; ++j)
                if (fabs(tc_contour[i].point.position[0] - tc_contour[j].point.position[0]) < 1e-6 &&
                    fabs(tc_contour[i].point.position[1] - tc_contour[j].point.position[1]) < 1e-6 &&
                    abs((int)tc_contour[i].color[0] - tc_contour[j].color[0]) > 100) distinct = 1;
        assert(distinct);
    }
    if (pattern == 3) {
        for (i = 0; i < 16; ++i) {
            angle = (float)(i * 6.283185307179586 / 16);
            x = .59f + .19f * (float)cos(angle); y = .43f + .19f * (float)sin(angle);
            assert(tc_render(tc_contour, count, x, y, rgb) && tc_nearest(rgb) == 1);
            x = .59f + .34f * (float)cos(angle); y = .43f + .34f * (float)sin(angle);
            assert(tc_render(tc_contour, count, x, y, rgb) && tc_nearest(rgb) == 0);
        }
    }
    if (pattern == 4) {
        left = tc_first_foreground(count, .25f);
        center = tc_first_foreground(count, .5f);
        right = tc_first_foreground(count, .75f);
        /* Global image error could improve while a flat rectangle replaces
         * the arch. Require its peak to remain above both shoulders. */
        printf("arch top landmarks: left %.3f center %.3f right %.3f\n", left, center, right);
        assert(center > .18f && center < .30f);
        assert(center + .03f < left && center + .03f < right);
    }
}

static void tc_test_shapes(void)
{
    static const char *names[] = {"panel", "diagonal", "bricks", "circle", "arch"};
    xgl_contour_stats_t stats;
    clock_t start;
    int pattern, count;
    for (pattern = 0; pattern < 5; ++pattern) {
        tc_setup(); tc_pattern(pattern);
        start = clock(); count = tc_build(TC_BUDGET, &stats);
        printf("build %-10s: %.3f host ms, %lu samples, %lu candidate evaluations, %d candidates, %d cuts\n",
               names[pattern], (clock() - start) * 1000.0 / CLOCKS_PER_SEC,
               stats.samples, stats.evaluations, stats.candidates, stats.cuts);
        assert(count > 0);
        assert(tc_services > 0);
        tc_coverage(count);
        tc_quality(names[pattern], count, 1);
        tc_shape_landmarks(pattern, count);
    }
}

static void tc_test_coordinates_and_budgets(void)
{
    xgl_contour_stats_t stats;
    xgl_contour_point_t swap;
    int count, i, mode, budget;
    for (mode = 0; mode < 4; ++mode) {
        tc_setup(); tc_pattern(mode == 3 ? 3 : 0);
        if (mode == 0) { tc_map[0][0] = -1.2f; tc_map[0][1] = 2; tc_map[1][0] = -.4f; }
        if (mode == 1) { tc_map[0][0] = .83f; tc_map[0][1] = -.8f; tc_map[0][2] = -.23f; tc_map[1][1] = .19f; }
        if (mode == 2) { tc_map[0][1] = 0; tc_map[0][2] = 1; tc_map[1][1] = -1; tc_map[1][2] = 0; }
        if (mode == 3) {
            swap = tc_face[1]; tc_face[1] = tc_face[3]; tc_face[3] = swap;
        }
        tc_apply_map(); count = tc_build(TC_BUDGET, &stats);
        assert(count > 0); tc_coverage(count);
    }
    tc_setup(); tc_pattern(0);
    for (budget = 1; budget <= TC_BUDGET; ++budget) {
        count = tc_build(budget, &stats);
        if (count) tc_coverage(count);
    }
    /* Repeated builds are deterministic, including clipped vertex order. */
    count = tc_build(TC_BUDGET, &stats);
    assert(count > 0);
    memcpy(tc_uniform, tc_contour, (size_t)count * 3 * sizeof(*tc_contour));
    for (i = 0; i < 3; ++i) {
        assert(tc_build(TC_BUDGET, &stats) == count);
        assert(!memcmp(tc_uniform, tc_contour, (size_t)count * 3 * sizeof(*tc_contour)));
    }
}

static void tc_test_existing_diagonal_budget(void)
{
    xgl_contour_stats_t stats;
    int x, y, i, count;
    tc_setup(); tc_pattern(0);
    for (y = 0; y < TC_SIZE; ++y)
        for (x = 0; x < TC_SIZE; ++x)
            tc_pixels[y * TC_SIZE + x] = (unsigned char)(x + y >= TC_SIZE);
    count = tc_build(2, &stats);
    assert(count == 2 && stats.cuts == 1);
    tc_coverage(count);
    tc_shape_landmarks(0, count);
    for (i = 0; i < count * 3; ++i) {
        float px, py;
        px = tc_contour[i].point.position[0]; py = tc_contour[i].point.position[1];
        assert(fabs(px) < 1e-6 || fabs(px - 1) < 1e-6);
        assert(fabs(py) < 1e-6 || fabs(py - 1) < 1e-6);
    }
    puts("existing diagonal budget: texture boundary represented with the original 2-triangle allowance");
}

static void tc_test_fallbacks(void)
{
    xgl_contour_stats_t stats;
    int count, failures;
    tc_setup(); tc_pattern(5);
    assert(tc_build(TC_BUDGET, &stats) == 0);
    tc_pattern(0);
    tc_map[0][1] = 6; tc_apply_map();
    assert(tc_build(TC_BUDGET, &stats) == 0);
    tc_map[0][1] = 0; tc_apply_map();
    assert(tc_build(TC_BUDGET, &stats) == 0);
    tc_setup(); tc_pattern(0);
    tc_face[2] = tc_face[1]; tc_face[3] = tc_face[0];
    assert(tc_build(TC_BUDGET, &stats) == 0);
    tc_setup(); tc_pattern(0);
    for (failures = 0; failures < 4; ++failures) {
        tc_fail_after = failures;
        count = tc_build(TC_BUDGET, &stats);
        if (failures == 0) assert(count == 0);
        if (count) tc_coverage(count);
    }
    tc_fail_after = -1;
    tc_setup(); tc_pattern(6);
    count = tc_build(TC_BUDGET, &stats);
    if (count) tc_coverage(count);
    assert(!XGL_ContourBuild(NULL, 4, tc_pixels, TC_SIZE, TC_SIZE, tc_palette,
                            TC_BUDGET, tc_contour, &stats, NULL));
    assert(!XGL_ContourBuild(tc_face, 4, NULL, TC_SIZE, TC_SIZE, tc_palette,
                            TC_BUDGET, tc_contour, &stats, NULL));
    assert(!XGL_ContourBuild(tc_face, 4, tc_pixels, 0, TC_SIZE, tc_palette,
                            TC_BUDGET, tc_contour, &stats, NULL));
    assert(tc_live_allocations == 0);
}

static void tc_test_convex_faces(void)
{
    static const float xy[5][2] = {{.1f,.1f}, {.8f,.05f}, {1,.6f}, {.6f,1}, {0,.75f}};
    xgl_contour_point_t face[5];
    xgl_contour_stats_t stats;
    double area, emitted, weights[3], side;
    float point[3];
    int i, j, x, y, count, inside, hits, near_edge;
    tc_setup(); tc_pattern(0);
    for (i = 0; i < 5; ++i) {
        face[i].position[0] = face[i].st[0] = xy[i][0];
        face[i].position[1] = face[i].st[1] = xy[i][1];
        face[i].position[2] = 3 + xy[i][0] * .25f - xy[i][1] * .5f;
    }
    count = XGL_ContourBuild(face, 5, tc_pixels, TC_SIZE, TC_SIZE, tc_palette,
                            TC_BUDGET, tc_contour, &stats, tc_service);
    assert(count > 0 && count <= TC_BUDGET && tc_live_allocations == 0);
    area = emitted = 0;
    for (i = 1; i < 4; ++i) area += tc_area(face[0].position, face[i].position, face[i+1].position);
    for (i = 0; i < count; ++i) {
        side = tc_area(tc_contour[i*3].point.position, tc_contour[i*3+1].point.position,
                       tc_contour[i*3+2].point.position);
        assert(side > 1e-12); emitted += side;
        for (j = 0; j < 3; ++j) {
            const xgl_contour_point_t *vertex;
            vertex = &tc_contour[i*3+j].point;
            assert(fabs(vertex->position[2] - (3 + vertex->position[0]*.25 - vertex->position[1]*.5)) < 2e-5);
            assert(fabs(vertex->st[0] - vertex->position[0]) < 2e-5);
            assert(fabs(vertex->st[1] - vertex->position[1]) < 2e-5);
        }
    }
    assert(fabs(area - emitted) < 2e-5);
    for (y = 0; y < 47; ++y)
        for (x = 0; x < 53; ++x) {
            point[0] = (x + .417f) / 53; point[1] = (y + .673f) / 47; point[2] = 0;
            inside = 1; near_edge = 0;
            for (i = 0; i < 5; ++i) {
                side = tc_area(face[i].position, face[(i+1)%5].position, point);
                if (side < 0) inside = 0;
                if (fabs(side) < 1e-6) near_edge = 1;
            }
            hits = 0;
            for (i = 0; i < count; ++i)
                if (tc_barycentric(tc_contour + i * 3, point[0], point[1], weights)) {
                    ++hits;
                    if (weights[0] < 1e-6 || weights[1] < 1e-6 || weights[2] < 1e-6) near_edge = 1;
                }
            if (!near_edge) assert(hits == inside);
        }
    /* An inward corner would invalidate the convex partition assumptions. */
    face[2].position[0] = face[2].st[0] = .35f;
    face[2].position[1] = face[2].st[1] = .4f;
    face[2].position[2] = 3 + .35f*.25f - .4f*.5f;
    assert(!XGL_ContourBuild(face, 5, tc_pixels, TC_SIZE, TC_SIZE, tc_palette,
                            TC_BUDGET, tc_contour, &stats, tc_service));
    assert(tc_live_allocations == 0);
}

int main(int argc, char **argv)
{
    if (argc == 3 && !strcmp(argv[1], "--dump")) tc_dump_directory = argv[2];
    else if (argc != 1) { fprintf(stderr, "usage: %s [--dump existing-directory]\n", argv[0]); return 2; }
    tc_test_shapes();
    tc_test_coordinates_and_budgets();
    tc_test_existing_diagonal_budget();
    tc_test_fallbacks();
    tc_test_convex_faces();
    printf("maximum temporary allocation: %lu bytes\n", (unsigned long)tc_peak_allocation);
    puts("XGL contour contracts passed: synthetic shape quality, coverage, winding, planarity, UVs, budgets and fallbacks (no native rendering)");
    return 0;
}
