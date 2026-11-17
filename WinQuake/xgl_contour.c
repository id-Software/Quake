/* Planar, budgeted texture contours for static XGL world surfaces.
 * GPL-2.0-or-later.
 *
 * A small face-local sample grid supplies color observations and boundary
 * segments. Greedy convex cuts follow those segments and reduce sampled
 * color error. The same point cloud scores every candidate: scoring never
 * resamples the texture. Work, memory, repetitions and output are bounded.
 * All cut vertices interpolate the original plane; no height is inferred.
 */
#include "xgl_contour.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define C_GRID 16
#define C_GRID_SAMPLES (C_GRID * C_GRID)
#define C_SAMPLES (C_GRID_SAMPLES + 128)
#define C_LINES 64
#define C_CELLS 64
#define C_CORNERS 36
#define C_EVALUATIONS 150000UL
#define C_EDGE_CONTRAST 900.0f
#define C_EPSILON .000001f

typedef struct {
    xgl_contour_point_t point;
    float x, y;
} c_vertex_t;

typedef struct {
    float x, y, rgb[3];
    int inside;
} c_sample_t;

typedef struct { float a, b, c, x, y; } c_line_t;

typedef struct {
    c_vertex_t vertices[C_CORNERS];
    unsigned short samples[C_SAMPLES];
    int count, sample_count, best_line, best_cost;
    float sum[3], squares, error, best_gain, best_score;
} c_cell_t;

/* One temporary allocation below is less than 128 KiB on 32/64-bit hosts.
 * No stack array grows with texture size, number of faces, or triangle count. */
typedef struct {
    c_cell_t cells[C_CELLS];
    c_sample_t samples[C_SAMPLES];
    c_line_t lines[C_LINES];
    c_vertex_t left[C_CORNERS + 1], right[C_CORNERS + 1];
    const unsigned char *pixels, *palette;
    int width, height, line_count, complex;
    float low[2], span[2];
    xgl_contour_stats_t stats;
    void (*service)(void);
} c_work_t;

static float c_abs(float x) { return x < 0 ? -x : x; }

static void c_sample(c_work_t *w, float x, float y, float rgb[3])
{
    float s, t;
    int ix, iy, index;
    s = w->low[0] + x * w->span[0];
    t = w->low[1] + y * w->span[1];
    s -= (float)(int)s; t -= (float)(int)t;
    if (s < 0) s += 1;
    if (t < 0) t += 1;
    ix = (int)(s * w->width); iy = (int)(t * w->height);
    if (ix >= w->width) ix = w->width - 1;
    if (iy >= w->height) iy = w->height - 1;
    index = w->pixels[iy * w->width + ix] * 3;
    rgb[0] = w->palette[index];
    rgb[1] = w->palette[index + 1];
    rgb[2] = w->palette[index + 2];
    ++w->stats.samples;
    if ((w->stats.samples & 127UL) == 0 && w->service) w->service();
}

static float c_distance(const float *a, const float *b)
{
    float r, g, blue;
    r = a[0] - b[0]; g = a[1] - b[1]; blue = a[2] - b[2];
    return r*r + g*g + blue*blue;
}

static float c_side(const c_line_t *line, float x, float y)
{
    return line->a*x + line->b*y + line->c;
}

static double c_area(const c_vertex_t *v, int n)
{
    double area;
    int i, j;
    area = 0;
    for (i = 0, j = n - 1; i < n; j = i++)
        area += (double)v[j].x*v[i].y - (double)v[i].x*v[j].y;
    return area * .5;
}

static int c_inside(const c_vertex_t *v, int n, float x, float y, float winding)
{
    float cross;
    int i, j;
    for (i = 0, j = n - 1; i < n; j = i++) {
        cross = (v[i].x-v[j].x)*(y-v[j].y) - (v[i].y-v[j].y)*(x-v[j].x);
        if (cross*winding < -C_EPSILON) return 0;
    }
    return 1;
}

static void c_crossing(c_work_t *w, const c_sample_t *a,
                       const c_sample_t *b, float out[2])
{
    float low, high, mid, rgb[3];
    int i;
    low = 0; high = 1;
    for (i = 0; i < 5; ++i) {
        mid = (low + high) * .5f;
        c_sample(w, a->x+(b->x-a->x)*mid, a->y+(b->y-a->y)*mid, rgb);
        if (c_distance(rgb, a->rgb) <= c_distance(rgb, b->rgb)) low = mid;
        else high = mid;
    }
    mid = (low + high) * .5f;
    out[0] = a->x+(b->x-a->x)*mid;
    out[1] = a->y+(b->y-a->y)*mid;
}

static void c_add_line_precision(c_work_t *w, const float *p, const float *q,
                                 float angle_tolerance, float offset_tolerance)
{
    c_line_t line, *old;
    float scale;
    int i;
    line.a = q[1]-p[1]; line.b = p[0]-q[0];
    scale = c_abs(line.a) > c_abs(line.b) ? c_abs(line.a) : c_abs(line.b);
    if (scale < .0001f) return;
    line.a /= scale; line.b /= scale;
    line.c = -(line.a*p[0] + line.b*p[1]);
    line.x = (p[0]+q[0])*.5f; line.y = (p[1]+q[1])*.5f;
    if (line.a < 0 || (line.a == 0 && line.b < 0)) {
        line.a = -line.a; line.b = -line.b; line.c = -line.c;
    }
    for (i = 0; i < w->line_count; ++i) {
        old = &w->lines[i];
        if (c_abs(old->a-line.a) < angle_tolerance &&
            c_abs(old->b-line.b) < angle_tolerance &&
            c_abs(old->c-line.c) < offset_tolerance) return;
    }
    if (w->line_count == C_LINES) { w->complex = 1; return; }
    w->lines[w->line_count++] = line;
}

static void c_add_line(c_work_t *w, const float *p, const float *q)
{
    c_add_line_precision(w, p, q, .015f, .004f);
}

static void c_find_lines(c_work_t *w)
{
    const c_sample_t *corners[4];
    float crossing[4][2];
    int x, y, edge, next, count, i, j;
    for (y = 0; y < C_GRID-1 && !w->complex; ++y) {
        for (x = 0; x < C_GRID-1 && !w->complex; ++x) {
            corners[0] = &w->samples[y*C_GRID+x];
            corners[1] = corners[0]+1;
            corners[2] = corners[1]+C_GRID;
            corners[3] = corners[0]+C_GRID;
            count = 0;
            for (edge = 0; edge < 4; ++edge) {
                next = (edge+1)&3;
                if (c_distance(corners[edge]->rgb, corners[next]->rgb) < C_EDGE_CONTRAST) continue;
                c_crossing(w, corners[edge], corners[next], crossing[count++]);
            }
            /* Ambiguous or multicolor cells offer each segment; global
             * error reduction chooses useful cuts. Excess complexity falls
             * back instead of treating fine noise as geometry. */
            for (i = 0; i < count; ++i)
                for (j = i+1; j < count; ++j)
                    c_add_line(w, crossing[i], crossing[j]);
        }
    }
    w->stats.candidates = w->line_count;
}

static void c_corner_lines(c_work_t *w)
{
    const c_cell_t *face;
    const c_line_t *line;
    float tolerance, texel_x, texel_y, p[2], q[2];
    int i, j, first, lines;
    face = &w->cells[0]; lines = w->line_count;
    for (i = 0; i < lines && w->line_count < C_LINES; ++i) {
        line = &w->lines[i];
        /* A sampled pixel boundary is not an exact analytic contour. If
         * two existing corners already follow it within roughly a texel,
         * also offer their diagonal. It can replace the original fan edge
         * without spending any additional triangles. Outer vertices never
         * move; the error score still decides whether this is useful. */
        texel_x = line->a/(w->width*w->span[0]);
        texel_y = line->b/(w->height*w->span[1]);
        tolerance = (float)sqrt(texel_x*texel_x + texel_y*texel_y);
        if (tolerance > .025f) tolerance = .025f;
        first = -1;
        for (j = 0; j < face->count; ++j) {
            if (c_abs(c_side(line, face->vertices[j].x, face->vertices[j].y)) > tolerance)
                continue;
            if (first < 0) { first = j; continue; }
            if (j == first+1 || (first == 0 && j == face->count-1)) continue;
            p[0] = face->vertices[first].x; p[1] = face->vertices[first].y;
            q[0] = face->vertices[j].x; q[1] = face->vertices[j].y;
            /* The nearby sampled line must not deduplicate this exact
             * corner line: their triangle costs can differ. */
            c_add_line_precision(w, p, q, C_EPSILON, C_EPSILON);
            break;
        }
    }
    w->stats.candidates = w->line_count;
}

static void c_edge_samples(c_work_t *w, float winding)
{
    c_cell_t *face;
    const c_line_t *line;
    c_sample_t pair[2];
    float direction;
    int i, j, count;
    face = &w->cells[0]; count = C_GRID_SAMPLES;
    for (i = 0; i < w->line_count; ++i) {
        line = &w->lines[i];
        /* Regular-grid observations alone can miss the narrow cap between
         * a curve and its chord. A bounded pair beside each detected edge
         * makes that error visible without enlarging the regular grid or
         * repeatedly sampling during candidate scoring. Retain only real
         * color boundaries; unrelated/noisy candidate pairs add no votes. */
        for (j = 0; j < 2; ++j) {
            direction = j ? -.25f/C_GRID : .25f/C_GRID;
            pair[j].x = line->x+line->a*direction;
            pair[j].y = line->y+line->b*direction;
            pair[j].inside = c_inside(face->vertices, face->count,
                                      pair[j].x, pair[j].y, winding);
            c_sample(w, pair[j].x, pair[j].y, pair[j].rgb);
        }
        if (c_distance(pair[0].rgb, pair[1].rgb) < C_EDGE_CONTRAST) continue;
        for (j = 0; j < 2 && count < C_SAMPLES; ++j) {
            if (!pair[j].inside) continue;
            w->samples[count] = pair[j];
            face->samples[face->sample_count++] = (unsigned short)count++;
        }
    }
}

static c_vertex_t c_interpolate(const c_vertex_t *a, const c_vertex_t *b, float t)
{
    c_vertex_t out;
    int i;
    for (i = 0; i < 3; ++i)
        out.point.position[i] = a->point.position[i] + (b->point.position[i]-a->point.position[i])*t;
    for (i = 0; i < 2; ++i)
        out.point.st[i] = a->point.st[i] + (b->point.st[i]-a->point.st[i])*t;
    out.x = a->x + (b->x-a->x)*t;
    out.y = a->y + (b->y-a->y)*t;
    return out;
}

static int c_simplify(c_vertex_t *v, int n)
{
    float ax, ay, bx, by;
    int i, previous, next, changed;
    changed = 1;
    while (changed && n >= 3) {
        changed = 0;
        for (i = 0; i < n; ++i) {
            previous = i ? i-1 : n-1; next = i+1 == n ? 0 : i+1;
            ax = v[i].x-v[previous].x; ay = v[i].y-v[previous].y;
            bx = v[next].x-v[i].x; by = v[next].y-v[i].y;
            if (c_abs(ax*by-ay*bx) > .0000001f) continue;
            if (ax*bx+ay*by < 0) continue;
            memmove(v+i, v+i+1, (size_t)(n-i-1)*sizeof(*v));
            --n; changed = 1; break;
        }
    }
    return n;
}

static int c_clip(const c_vertex_t *v, int n, const c_line_t *line,
                   int positive, c_vertex_t *out)
{
    float before, after, t;
    int i, previous, count, was_inside, is_inside;
    count = 0; previous = n-1;
    before = c_side(line, v[previous].x, v[previous].y);
    if (!positive) before = -before;
    for (i = 0; i < n; ++i) {
        after = c_side(line, v[i].x, v[i].y);
        if (!positive) after = -after;
        was_inside = before >= 0; is_inside = after >= 0;
        if (was_inside != is_inside) {
            t = before/(before-after);
            if (count == C_CORNERS+1) return 0;
            out[count++] = c_interpolate(v+previous, v+i, t);
        }
        if (is_inside) {
            if (count == C_CORNERS+1) return 0;
            out[count++] = v[i];
        }
        previous = i; before = after;
    }
    count = c_simplify(out, count);
    return count >= 3 && count <= C_CORNERS ? count : 0;
}

static float c_mean_term(const float *sum, int count)
{
    if (!count) return 0;
    return (sum[0]*sum[0]+sum[1]*sum[1]+sum[2]*sum[2])/count;
}

static void c_totals(c_work_t *w, c_cell_t *cell)
{
    const c_sample_t *sample;
    int i, j;
    cell->squares = 0;
    cell->sum[0] = cell->sum[1] = cell->sum[2] = 0;
    for (i = 0; i < cell->sample_count; ++i) {
        sample = &w->samples[cell->samples[i]];
        for (j = 0; j < 3; ++j) {
            cell->sum[j] += sample->rgb[j];
            cell->squares += sample->rgb[j]*sample->rgb[j];
        }
    }
    cell->error = cell->squares - c_mean_term(cell->sum, cell->sample_count);
    if (cell->error < 0) cell->error = 0;
}

static void c_best(c_work_t *w, c_cell_t *cell, int allowance)
{
    const c_line_t *line;
    const c_sample_t *sample;
    float left[3], right[3], gain, side, low, high;
    int i, j, k, nl, nr, vl, vr, cost;
    cell->best_line = -1; cell->best_score = cell->best_gain = 0;
    if (cell->sample_count < 4 || cell->error < 64*cell->sample_count) return;
    for (i = 0; i < w->line_count; ++i) {
        line = &w->lines[i];
        low = high = c_side(line, cell->vertices[0].x, cell->vertices[0].y);
        for (j = 1; j < cell->count; ++j) {
            side = c_side(line, cell->vertices[j].x, cell->vertices[j].y);
            if (side < low) low = side;
            if (side > high) high = side;
        }
        if (low >= -C_EPSILON || high <= C_EPSILON) continue;
        if (w->stats.evaluations + (unsigned long)cell->sample_count > C_EVALUATIONS) return;
        left[0] = left[1] = left[2] = 0; nl = 0;
        for (j = 0; j < cell->sample_count; ++j) {
            sample = &w->samples[cell->samples[j]];
            if (c_side(line, sample->x, sample->y) >= 0) {
                ++nl;
                for (k = 0; k < 3; ++k) left[k] += sample->rgb[k];
            }
        }
        w->stats.evaluations += (unsigned long)cell->sample_count;
        if (w->service && (w->stats.evaluations & 2047UL) < (unsigned long)cell->sample_count)
            w->service();
        nr = cell->sample_count-nl;
        if (nl < 2 || nr < 2) continue;
        for (k = 0; k < 3; ++k) right[k] = cell->sum[k]-left[k];
        gain = c_mean_term(left, nl)+c_mean_term(right, nr)-
            c_mean_term(cell->sum, cell->sample_count);
        if (gain < cell->error*.02f || gain < 64*cell->sample_count || gain <= cell->best_score)
            continue;
        vl = c_clip(cell->vertices, cell->count, line, 1, w->left);
        vr = c_clip(cell->vertices, cell->count, line, 0, w->right);
        if (!vl || !vr || fabs(c_area(w->left,vl)) < .000001 ||
            fabs(c_area(w->right,vr)) < .000001) continue;
        cost = vl+vr-cell->count-2;
        if (cost < 0 || cost > 2 || cost > allowance ||
            gain/(cost ? cost : 1) <= cell->best_score) continue;
        cell->best_line = i; cell->best_cost = cost;
        cell->best_score = gain/(cost ? cost : 1); cell->best_gain = gain;
    }
}

int XGL_ContourBuild(const xgl_contour_point_t *face, int face_count,
    const unsigned char *pixels, int width, int height,
    const unsigned char *palette, int triangle_budget,
    xgl_contour_vertex_t *output, xgl_contour_stats_t *stats,
    void (*service)(void))
{
    c_work_t *w;
    c_cell_t *first, *cell, *other;
    c_sample_t *sample;
    const c_line_t *line;
    double area, cross, original_area, emitted_area;
    float high[2], winding, initial_error, final_error, score;
    unsigned short original_samples[C_SAMPLES];
    unsigned char color[3];
    int i, j, k, x, y, cells, triangles, best, nl, nr, original_count, out_count;
    if (stats) memset(stats, 0, sizeof(*stats));
    if (!face || !pixels || !palette || !output || face_count < 3 ||
        face_count > XGL_CONTOUR_MAX_FACE_VERTICES || triangle_budget < face_count-2 ||
        width < 1 || height < 1 || width > 2048 || height > 2048 ||
        (unsigned long)width*height > 1024UL*1024UL) return 0;
    if (triangle_budget > XGL_CONTOUR_MAX_TRIANGLES) triangle_budget = XGL_CONTOUR_MAX_TRIANGLES;
    w = (c_work_t *)malloc(sizeof(*w));
    if (!w) return 0;
    memset(w, 0, sizeof(*w));
    w->pixels = pixels; w->palette = palette; w->width = width; w->height = height;
    w->service = service;
    first = &w->cells[0]; first->count = face_count;
    w->low[0] = high[0] = face[0].st[0];
    w->low[1] = high[1] = face[0].st[1];
    for (i = 0; i < face_count; ++i) {
        for (j = 0; j < 2; ++j) {
            if (!(face[i].st[j] > -1000000 && face[i].st[j] < 1000000)) goto fallback;
            if (face[i].st[j] < w->low[j]) w->low[j] = face[i].st[j];
            if (face[i].st[j] > high[j]) high[j] = face[i].st[j];
        }
        for (j = 0; j < 3; ++j)
            if (!(face[i].position[j] > -1000000 && face[i].position[j] < 1000000)) goto fallback;
    }
    for (j = 0; j < 2; ++j) {
        w->span[j] = high[j]-w->low[j];
        if (w->span[j] <= .000001f || w->span[j] > 4.000001f) goto fallback;
    }
    for (i = 0; i < face_count; ++i) {
        first->vertices[i].point = face[i];
        first->vertices[i].x = (face[i].st[0]-w->low[0])/w->span[0];
        first->vertices[i].y = (face[i].st[1]-w->low[1])/w->span[1];
    }
    first->count = c_simplify(first->vertices, first->count);
    if (first->count < 3) goto fallback;
    original_area = c_area(first->vertices, first->count);
    if (fabs(original_area) < .00001) goto fallback;
    winding = original_area > 0 ? 1 : -1;
    for (i = 0; i < first->count; ++i) {
        j = (i+1)%first->count; k = (i+2)%first->count;
        cross = (first->vertices[j].x-first->vertices[i].x)*(first->vertices[k].y-first->vertices[j].y)-
            (first->vertices[j].y-first->vertices[i].y)*(first->vertices[k].x-first->vertices[j].x);
        if (cross*winding < -C_EPSILON) goto fallback;
    }
    for (y = 0; y < C_GRID; ++y) {
        for (x = 0; x < C_GRID; ++x) {
            i = y*C_GRID+x; sample = &w->samples[i];
            sample->x = (x+.5f)/C_GRID; sample->y = (y+.5f)/C_GRID;
            sample->inside = c_inside(first->vertices, first->count, sample->x, sample->y, winding);
            c_sample(w, sample->x, sample->y, sample->rgb);
            if (sample->inside) first->samples[first->sample_count++] = (unsigned short)i;
        }
    }
    if (first->sample_count < 8) goto fallback;
    c_totals(w, first); initial_error = first->error;
    if (initial_error < C_EDGE_CONTRAST*first->sample_count) goto fallback;
    c_find_lines(w);
    if (w->complex || !w->line_count) goto fallback;
    c_corner_lines(w);
    c_edge_samples(w, winding);
    c_totals(w, first); initial_error = first->error;
    cells = 1; triangles = first->count-2;
    c_best(w, first, triangle_budget-triangles);
    while (cells < C_CELLS && triangles <= triangle_budget) {
        best = -1; score = 0;
        for (i = 0; i < cells; ++i) {
            cell = &w->cells[i];
            /* A cut through existing vertices can improve a diagonal at no
             * additional triangle cost. Reconsider old best cuts when the
             * remaining allowance shrinks, rather than hiding such a cut
             * behind a more expensive candidate. Cell count still bounds
             * refinement even when the triangle count does not increase. */
            if (cell->best_line >= 0 && triangles+cell->best_cost > triangle_budget)
                c_best(w, cell, triangle_budget-triangles);
            if (cell->best_line < 0) continue;
            if (cell->best_score > score) { best = i; score = cell->best_score; }
        }
        if (best < 0) break;
        cell = &w->cells[best]; other = &w->cells[cells];
        line = &w->lines[cell->best_line];
        nl = c_clip(cell->vertices, cell->count, line, 1, w->left);
        nr = c_clip(cell->vertices, cell->count, line, 0, w->right);
        if (!nl || !nr) goto fallback;
        triangles += nl+nr-cell->count-2;
        original_count = cell->sample_count;
        memcpy(original_samples, cell->samples, (size_t)original_count*sizeof(original_samples[0]));
        cell->sample_count = other->sample_count = 0;
        for (i = 0; i < original_count; ++i) {
            sample = &w->samples[original_samples[i]];
            if (c_side(line, sample->x, sample->y) >= 0)
                cell->samples[cell->sample_count++] = original_samples[i];
            else other->samples[other->sample_count++] = original_samples[i];
        }
        cell->count = nl; other->count = nr;
        memcpy(cell->vertices, w->left, (size_t)nl*sizeof(w->left[0]));
        memcpy(other->vertices, w->right, (size_t)nr*sizeof(w->right[0]));
        c_totals(w, cell); c_totals(w, other);
        c_best(w, cell, triangle_budget-triangles);
        c_best(w, other, triangle_budget-triangles);
        ++cells; ++w->stats.cuts;
        if (w->service) w->service();
    }
    if (!w->stats.cuts || triangles > triangle_budget) goto fallback;
    final_error = 0; emitted_area = 0;
    for (i = 0; i < cells; ++i) {
        cell = &w->cells[i]; final_error += cell->error;
        area = c_area(cell->vertices, cell->count);
        if (area*winding <= .000001) goto fallback;
        emitted_area += area;
    }
    /* Reject noise fits and insignificant changes rather than replacing a
     * filtered material by many arbitrary flat patches. */
    if (final_error > initial_error*.80f ||
        fabs(emitted_area-original_area) > .00001) goto fallback;
    out_count = 0;
    for (i = 0; i < cells; ++i) {
        cell = &w->cells[i];
        for (j = 0; j < 3; ++j)
            color[j] = (unsigned char)(cell->sum[j]/cell->sample_count + .5f);
        for (j = 1; j+1 < cell->count; ++j) {
            output[out_count].point = cell->vertices[0].point;
            output[out_count+1].point = cell->vertices[j].point;
            output[out_count+2].point = cell->vertices[j+1].point;
            for (k = 0; k < 3; ++k) memcpy(output[out_count+k].color, color, 3);
            out_count += 3;
        }
    }
    if (out_count != triangles*3) goto fallback;
    if (stats) *stats = w->stats;
    free(w);
    return triangles;
fallback:
    if (stats) *stats = w->stats;
    free(w);
    return 0;
}
