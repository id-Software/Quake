/*
 * XGLQuake standalone benchmark. GPL-2.0-or-later; see COPYING.
 *
 * Procedural geometry and material sampling. No Quake assets or renderer code
 * are required. The 64x64 brick/checker image and its averaged RGB mip levels
 * are generated in CPU memory. A facet receives a small filtered sample around its
 * UV centroid. Optional per-corner samples use the same CPU filter at each
 * corner. This file does no per-pixel rasterization; the base image is also
 * exported for the optional native XGL texture-mapping comparison.
 *
 * Each base square contributes two triangles. Uniform edge subdivision yields
 * 2 * 4^depth triangles per square. The strip and independent-triangle arrays
 * describe exactly the same triangles in exactly the same facet order, so the
 * submission benchmarks do not compare different visible workloads.
 */
#include "xglbench.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define XB_SCENE_MAX_TRIANGLES 262144UL
#define XB_TEXTURE_SIZE XB_TEXTURE_WIDTH
#define XB_TEXTURE_LEVELS 4
#define XB_TEXTURE_PIXELS (4096 + 1024 + 256 + 64)

static xb_rgb xb_texture[XB_TEXTURE_PIXELS];
static int xb_texture_ready;
static const int xb_texture_offset[XB_TEXTURE_LEVELS] = { 0, 4096, 5120, 5376 };

static void
xb_texture_init(void)
{
    int x, y, level, size, prev_size, offset, prev_offset;
    int brick, mortar, checker;
    xb_rgb *pixel;
    const xb_rgb *a, *b, *c, *d;
    static const xb_rgb brick_colors[4] = {
        { 0.68f, 0.28f, 0.16f }, { 0.48f, 0.22f, 0.12f },
        { 0.56f, 0.36f, 0.20f }, { 0.39f, 0.42f, 0.24f }
    };

    if (xb_texture_ready)
        return;
    for (y = 0; y < XB_TEXTURE_SIZE; ++y) {
        for (x = 0; x < XB_TEXTURE_SIZE; ++x) {
            brick = ((x + ((y / 8) & 1) * 8) / 16 + y / 8) & 3;
            mortar = (y % 8 == 0) ||
                     ((x + ((y / 8) & 1) * 8) % 16 == 0);
            checker = ((x / 4) ^ (y / 4)) & 1;
            pixel = &xb_texture[y * XB_TEXTURE_SIZE + x];
            if (mortar) {
                pixel->r = 0.16f;
                pixel->g = 0.18f;
                pixel->b = 0.15f;
            } else {
                *pixel = brick_colors[brick];
                if (checker) {
                    pixel->r *= 0.85f;
                    pixel->g *= 0.85f;
                    pixel->b *= 0.85f;
                }
            }
        }
    }
    for (level = 1; level < XB_TEXTURE_LEVELS; ++level) {
        size = XB_TEXTURE_SIZE >> level;
        prev_size = size * 2;
        offset = xb_texture_offset[level];
        prev_offset = xb_texture_offset[level - 1];
        for (y = 0; y < size; ++y) {
            for (x = 0; x < size; ++x) {
                a = &xb_texture[prev_offset + y * 2 * prev_size + x * 2];
                b = a + 1;
                c = a + prev_size;
                d = c + 1;
                pixel = &xb_texture[offset + y * size + x];
                pixel->r = (a->r + b->r + c->r + d->r) * 0.25f;
                pixel->g = (a->g + b->g + c->g + d->g) * 0.25f;
                pixel->b = (a->b + b->b + c->b + d->b) * 0.25f;
            }
        }
    }
    xb_texture_ready = 1;
}

int
xb_texture_image(xb_rgb *pixels, size_t count)
{
    if (!pixels || count < (size_t)XB_TEXTURE_SIZE * XB_TEXTURE_SIZE) return 0;
    xb_texture_init();
    memcpy(pixels, xb_texture, (size_t)XB_TEXTURE_SIZE * XB_TEXTURE_SIZE * sizeof(*pixels));
    return 1;
}

static xb_rgb
xb_texture_lookup(double u, double v, int level)
{
    int size, x, y;

    /* Wrap only at lookup. floor(), unlike C truncation, handles negatives. */
    size = XB_TEXTURE_SIZE >> level;
    u -= floor(u);
    v -= floor(v);
    x = (int)(u * size);
    y = (int)(v * size);
    if (x >= size)
        x = 0;
    if (y >= size)
        y = 0;
    return xb_texture[xb_texture_offset[level] + y * size + x];
}

static xb_rgb
xb_material_sample(double u, double v, int depth, float light)
{
    xb_rgb result, sample;
    double delta;
    int sx, sy, level;

    /* Four mip-filtered samples approximate a facet footprint. The coarser
     * levels deliberately lose detail. These are CPU material samples, not a
     * claim of equivalence to a perspective-correct texture rasterizer. */
    result.r = result.g = result.b = 0.0f;
    level = 3 - depth;
    delta = 0.20 / (double)(1 << depth);
    for (sy = -1; sy <= 1; sy += 2) {
        for (sx = -1; sx <= 1; sx += 2) {
            sample = xb_texture_lookup(u + sx * delta,
                                       v + sy * delta, level);
            result.r += sample.r;
            result.g += sample.g;
            result.b += sample.b;
        }
    }
    result.r *= 0.25f * light;
    result.g *= 0.25f * light;
    result.b *= 0.25f * light;
    return result;
}

size_t
xb_mesh_expected_triangles(int base_triangles, int depth)
{
    size_t count, multiplier;

    if (base_triangles < 2 || (base_triangles & 1) || depth < 0 || depth > 2)
        return 0;
    multiplier = (size_t)1 << (depth * 2);
    count = (size_t)base_triangles;
    if (count > (size_t)XB_SCENE_MAX_TRIANGLES / multiplier)
        return 0;
    return count * multiplier;
}

void
xb_mesh_free(xb_mesh *mesh)
{
    if (mesh == NULL)
        return;
    free(mesh->points);
    free(mesh->strip_points);
    free(mesh->colors);
    free(mesh->base_colors);
    free(mesh->texcoords);
    free(mesh->vertex_materials);
    free(mesh->strips);
    free(mesh->facet_normals);
    free(mesh->vertex_normals);
    free(mesh->centers);
    free(mesh->vertex_colors);
    free(mesh->facet_styles);
    free(mesh->vertex_styles);
    memset(mesh, 0, sizeof(*mesh));
}

static int
xb_mesh_allocate(xb_mesh *mesh)
{
    size_t max_size;

    max_size = (size_t)-1;
    if (mesh->point_count > max_size / sizeof(xb_point) ||
        mesh->point_count > max_size / sizeof(xb_texcoord) ||
        mesh->strip_point_count > max_size / sizeof(xb_point) ||
        mesh->triangles > max_size / sizeof(xb_rgb) ||
        mesh->strip_count > max_size / sizeof(xb_strip))
        return 0;
    mesh->points = (xb_point *)malloc(mesh->point_count * sizeof(xb_point));
    mesh->strip_points = (xb_point *)malloc(mesh->strip_point_count * sizeof(xb_point));
    mesh->colors = (xb_rgb *)malloc(mesh->triangles * sizeof(xb_rgb));
    mesh->base_colors = (xb_rgb *)malloc(mesh->triangles * sizeof(xb_rgb));
    mesh->texcoords = (xb_texcoord *)malloc(mesh->point_count * sizeof(xb_texcoord));
    mesh->strips = (xb_strip *)malloc(mesh->strip_count * sizeof(xb_strip));
    if (mesh->points == NULL || mesh->strip_points == NULL ||
        mesh->colors == NULL || mesh->base_colors == NULL || mesh->texcoords == NULL ||
        mesh->strips == NULL) {
        xb_mesh_free(mesh);
        return 0;
    }
    mesh->bytes = mesh->point_count * (sizeof(xb_point) + sizeof(xb_texcoord)) +
                  mesh->strip_point_count * sizeof(xb_point) +
                  2 * mesh->triangles * sizeof(xb_rgb) +
                  mesh->strip_count * sizeof(xb_strip);
    return 1;
}

/* Strip rows have alternating top/bottom points. Preserve their unwrapped UV
 * coordinates independently of later clipping, transforms or lighting curves. */
static void
xb_store_texcoords(xb_mesh *mesh, size_t corner, size_t strip_start,
                    size_t a, size_t b, size_t c, size_t row, int divisions,
                    double origin_u, double origin_v, float gain)
{
    size_t indices[3], index;
    int j;
    indices[0] = a; indices[1] = b; indices[2] = c;
    for (j = 0; j < 3; ++j) {
        index = indices[j] - strip_start;
        mesh->texcoords[corner + j].u =
            (float)((double)(index / 2) / divisions + origin_u);
        mesh->texcoords[corner + j].v =
            (float)((double)(row + 1 - index % 2) / divisions + origin_v);
        mesh->texcoords[corner + j].gain = gain;
    }
}

int
xb_mesh_vertex_materials(xb_mesh *mesh, int depth)
{
    xb_rgb *materials, *colors;
    const xb_texcoord *coord;
    size_t i, extra;
    if (!mesh || depth < 0 || depth > 2 || !mesh->triangles ||
        mesh->triangles > XB_SCENE_MAX_TRIANGLES ||
        mesh->point_count != mesh->triangles * 3 || !mesh->texcoords ||
        mesh->vertex_materials || mesh->vertex_colors || mesh->lighting_ready)
        return 0;
    for (i = 0; i < mesh->point_count; ++i) {
        coord = mesh->texcoords + i;
        if (!(coord->u >= -FLT_MAX && coord->u <= FLT_MAX &&
              coord->v >= -FLT_MAX && coord->v <= FLT_MAX &&
              coord->gain >= 0.0f && coord->gain <= 1.0f)) return 0;
    }
    extra = mesh->point_count * 2 * sizeof(xb_rgb);
    if (mesh->bytes > (size_t)-1 - extra) return 0;
    materials = (xb_rgb *)malloc(mesh->point_count * sizeof(*materials));
    colors = (xb_rgb *)malloc(mesh->point_count * sizeof(*colors));
    if (!materials || !colors) { free(materials); free(colors); return 0; }
    xb_texture_init();
    for (i = 0; i < mesh->point_count; ++i) {
        coord = mesh->texcoords + i;
        materials[i] = xb_material_sample(coord->u, coord->v, depth, coord->gain);
        colors[i] = materials[i];
    }
    mesh->vertex_materials = materials;
    mesh->vertex_colors = colors;
    mesh->bytes += extra;
    return 1;
}

int
xb_mesh_texture_materials(xb_mesh *mesh, int depth, int vertex)
{
    xb_rgb *materials, *colors;
    const xb_texcoord *coord;
    size_t i, corner, extra;
    double u, v;
    if (!mesh || depth < 0 || depth > 2 || (vertex != 0 && vertex != 1) ||
        !mesh->triangles || mesh->triangles > XB_SCENE_MAX_TRIANGLES ||
        mesh->point_count != mesh->triangles * 3 || !mesh->texcoords ||
        !mesh->colors || !mesh->base_colors || mesh->lighting_ready ||
        mesh->vertex_materials || mesh->vertex_colors)
        return 0;
    for (i = 0; i < mesh->point_count; ++i) {
        coord = mesh->texcoords + i;
        if (!(coord->u >= -FLT_MAX && coord->u <= FLT_MAX &&
              coord->v >= -FLT_MAX && coord->v <= FLT_MAX &&
              coord->gain >= 0.0f && coord->gain <= 1.0f)) return 0;
    }
    extra = vertex ? mesh->point_count * 2 * sizeof(xb_rgb) : 0;
    if (mesh->bytes > (size_t)-1 - extra) return 0;
    materials = colors = NULL;
    if (vertex) {
        materials = (xb_rgb *)malloc(mesh->point_count * sizeof(*materials));
        colors = (xb_rgb *)malloc(mesh->point_count * sizeof(*colors));
        if (!materials || !colors) { free(materials); free(colors); return 0; }
    }
    /* All rejecting checks and allocations precede the commit. Use preserved
     * UVs instead of geometry, which may already have been transformed. The
     * native REPLACE texture path has no per-patch tint, so neither do these
     * matched approximation controls. Filtering policies still differ. */
    xb_texture_init();
    for (i = 0; i < mesh->triangles; ++i) {
        corner = i * 3;
        u = ((double)mesh->texcoords[corner].u + mesh->texcoords[corner + 1].u +
             mesh->texcoords[corner + 2].u) / 3.0;
        v = ((double)mesh->texcoords[corner].v + mesh->texcoords[corner + 1].v +
             mesh->texcoords[corner + 2].v) / 3.0;
        mesh->base_colors[i] = xb_material_sample(u, v, depth, 1.0f);
        mesh->colors[i] = mesh->base_colors[i];
    }
    for (i = 0; i < mesh->point_count; ++i) {
        mesh->texcoords[i].gain = 1.0f;
        if (vertex) {
            materials[i] = xb_material_sample(mesh->texcoords[i].u,
                mesh->texcoords[i].v, depth, 1.0f);
            colors[i] = materials[i];
        }
    }
    mesh->vertex_materials = materials;
    mesh->vertex_colors = colors;
    mesh->bytes += extra;
    return 1;
}

int
xb_mesh_build(xb_mesh *mesh, int base_triangles, int depth,
              float size, int overdraw)
{
    xb_mesh result;
    size_t patches, patch, strip_number, point_number, facet_number;
    size_t strip_start, a, b, c, grid_width, grid_height;
    int divisions, row, col, side, triangle;
    double center_x, center_y, patch_width, patch_height, x0, y0, u, v;
    double local_x, local_y, z;
    float light;
    xb_point *point;
    xb_strip *strip;

    /* The comparisons reject NaN and infinities without non-C89 isfinite(). */
    if (mesh == NULL || !(size > 0.0f && size <= 16.0f) ||
        (overdraw != 0 && overdraw != 1))
        return 0;
    memset(&result, 0, sizeof(result));
    result.triangles = xb_mesh_expected_triangles(base_triangles, depth);
    if (result.triangles == 0)
        return 0;
    patches = (size_t)base_triangles / 2;
    divisions = 1 << depth;
    result.point_count = result.triangles * 3;
    result.strip_count = patches * (size_t)divisions;
    result.strip_point_count = result.strip_count * (size_t)(2 * (divisions + 1));
    if (!xb_mesh_allocate(&result))
        return 0;

    xb_texture_init();
    grid_width = 1;
    while (grid_width * grid_width < patches)
        ++grid_width;
    grid_height = (patches + grid_width - 1) / grid_width;
    point_number = facet_number = strip_number = 0;
    for (patch = 0; patch < patches; ++patch) {
        if (overdraw) {
            /* Identical coverage at distinct depths. Near-to-far order stresses
             * depth rejection instead of silently changing triangle count. */
            center_x = center_y = 0.0;
            patch_width = patch_height = 1.8 * size;
            z = 0.15 + 0.70 * (double)patch / (double)patches;
        } else {
            center_x = -0.9 + ((double)(patch % grid_width) + 0.5) * 1.8 /
                       (double)grid_width;
            center_y = -0.9 + ((double)(patch / grid_width) + 0.5) * 1.8 /
                       (double)grid_height;
            patch_width = 1.8 * size / (double)grid_width;
            patch_height = 1.8 * size / (double)grid_height;
            z = 0.25 + 0.45 * (double)(patch % 17) / 17.0;
        }
        x0 = center_x - patch_width * 0.5;
        y0 = center_y - patch_height * 0.5;
        light = 0.65f + 0.35f * (float)(patch % 7) / 6.0f;
        for (row = 0; row < divisions; ++row) {
            strip = &result.strips[strip_number++];
            strip->first_point = point_number;
            strip->point_count = (size_t)(2 * (divisions + 1));
            strip->first_facet = facet_number;
            strip_start = point_number;
            for (col = 0; col <= divisions; ++col) {
                for (side = 0; side < 2; ++side) {
                    /* Top, bottom at each column gives counter-clockwise
                     * first facets. Alternate facets reverse the first edge. */
                    local_x = (double)col / divisions;
                    local_y = (double)(row + 1 - side) / divisions;
                    point = &result.strip_points[point_number++];
                    point->x = (float)(x0 + local_x * patch_width);
                    point->y = (float)(y0 + local_y * patch_height);
                    point->z = (float)z;
                }
            }
            for (triangle = 0; triangle < 2 * divisions; ++triangle) {
                a = strip_start + (size_t)triangle;
                b = a + 1;
                c = a + 2;
                if (triangle & 1) {
                    a = strip_start + (size_t)triangle + 1;
                    b = a - 1;
                }
                result.points[facet_number * 3] = result.strip_points[a];
                result.points[facet_number * 3 + 1] = result.strip_points[b];
                result.points[facet_number * 3 + 2] = result.strip_points[c];
                xb_store_texcoords(&result, facet_number * 3, strip_start,
                    a, b, c, (size_t)row, divisions,
                    (double)(patch % 5) * 0.23 - 0.37,
                    (double)(patch % 3) * 0.19 - 0.21, light);
                col = triangle / 2;
                if (triangle & 1) {
                    u = ((double)col + 2.0 / 3.0) / divisions;
                    v = ((double)row + 1.0 / 3.0) / divisions;
                } else {
                    u = ((double)col + 1.0 / 3.0) / divisions;
                    v = ((double)row + 2.0 / 3.0) / divisions;
                }
                /* Vary the unwrapped origin to exercise repeated texture
                 * lookup and negative UVs with a single shared CPU image. */
                u += (double)(patch % 5) * 0.23 - 0.37;
                v += (double)(patch % 3) * 0.19 - 0.21;
                result.base_colors[facet_number] =
                    xb_material_sample(u, v, depth, light);
                result.colors[facet_number] = result.base_colors[facet_number];
                ++facet_number;
            }
        }
    }

    /* Caller must zero-initialize its first mesh. Failure never destroys a
     * previous mesh, and a successful rebuild releases the previous arrays. */
    xb_mesh_free(mesh);
    *mesh = result;
    return 1;
}

int
xb_mesh_build_connected(xb_mesh *mesh, int base_triangles, int depth)
{
    xb_mesh result;
    size_t cells, factor, base_columns, columns, rows;
    size_t row, col, side, strip_start, point_number, facet_number;
    size_t triangle, a, b, c;
    int divisions;
    double u, v;
    xb_point *point;
    xb_strip *strip;

    if (mesh == NULL)
        return 0;
    memset(&result, 0, sizeof(result));
    result.triangles = xb_mesh_expected_triangles(base_triangles, depth);
    if (result.triangles == 0)
        return 0;
    cells = (size_t)base_triangles / 2;
    factor = 1;
    while ((factor + 1) * (factor + 1) <= cells)
        ++factor;
    while (cells % factor != 0)
        --factor;
    /* Exact factors retain the requested count. The longer dimension is
     * horizontal, so even a prime cell count exercises a long strip. */
    base_columns = cells / factor;
    divisions = 1 << depth;
    columns = base_columns * (size_t)divisions;
    rows = factor * (size_t)divisions;
    result.point_count = result.triangles * 3;
    result.strip_count = rows;
    result.strip_point_count = rows * 2 * (columns + 1);
    if (!xb_mesh_allocate(&result))
        return 0;
    xb_texture_init();
    point_number = facet_number = 0;
    for (row = 0; row < rows; ++row) {
        strip = &result.strips[row];
        strip->first_point = point_number;
        strip->point_count = 2 * (columns + 1);
        strip->first_facet = facet_number;
        strip_start = point_number;
        for (col = 0; col <= columns; ++col) {
            for (side = 0; side < 2; ++side) {
                point = &result.strip_points[point_number++];
                point->x = (float)(-0.9 + 1.8 * (double)col / (double)columns);
                point->y = (float)(-0.9 + 1.8 * (double)(row + 1 - side) /
                                   (double)rows);
                point->z = 0.5f;
            }
        }
        for (triangle = 0; triangle < 2 * columns; ++triangle) {
            a = strip_start + triangle;
            b = a + 1;
            c = a + 2;
            if (triangle & 1) {
                a = strip_start + triangle + 1;
                b = a - 1;
            }
            result.points[facet_number * 3] = result.strip_points[a];
            result.points[facet_number * 3 + 1] = result.strip_points[b];
            result.points[facet_number * 3 + 2] = result.strip_points[c];
            xb_store_texcoords(&result, facet_number * 3, strip_start,
                a, b, c, row, divisions, -0.37, -0.21, 0.85f);
            col = triangle / 2;
            if (triangle & 1) {
                u = ((double)col + 2.0 / 3.0) / divisions;
                v = ((double)row + 1.0 / 3.0) / divisions;
            } else {
                u = ((double)col + 1.0 / 3.0) / divisions;
                v = ((double)row + 2.0 / 3.0) / divisions;
            }
            result.base_colors[facet_number] =
                xb_material_sample(u - 0.37, v - 0.21, depth, 0.85f);
            result.colors[facet_number] = result.base_colors[facet_number];
            ++facet_number;
        }
    }
    xb_mesh_free(mesh);
    *mesh = result;
    return 1;
}

/* Projected geometric area, before viewport clipping or occlusion. Summing
 * these areas counts overlap repeatedly; it is not a covered-pixel count.
 * Use the same endpoint mapping as the XGL viewport: [-1,1] -> [0,size-1]. */
int
xb_mesh_projected_area(const xb_mesh *mesh, int width, int height,
                       double *mean, double *minimum, double *maximum)
{
    const xb_point *a, *b, *c, *point;
    size_t i;
    double scale, area, total, low, high;

    if (mesh == NULL || width < 2 || height < 2 || mean == NULL ||
        minimum == NULL || maximum == NULL || mesh->triangles == 0 ||
        mesh->triangles > (size_t)-1 / 3 || mesh->points == NULL ||
        mesh->point_count != mesh->triangles * 3)
        return 0;
    for (i = 0; i < mesh->point_count; ++i) {
        point = &mesh->points[i];
        /* Ordered comparisons also reject NaNs, without C99 isfinite(). */
        if (!(point->x >= -FLT_MAX && point->x <= FLT_MAX) ||
            !(point->y >= -FLT_MAX && point->y <= FLT_MAX) ||
            !(point->z >= -FLT_MAX && point->z <= FLT_MAX))
            return 0;
    }
    scale = ((double)width - 1.0) * ((double)height - 1.0) * 0.125;
    total = high = 0.0;
    low = DBL_MAX;
    for (i = 0; i < mesh->triangles; ++i) {
        a = &mesh->points[i * 3];
        b = a + 1;
        c = a + 2;
        area = fabs(((double)b->x - a->x) * ((double)c->y - a->y) -
                    ((double)b->y - a->y) * ((double)c->x - a->x)) * scale;
        if (!(area >= 0.0 && area <= DBL_MAX))
            return 0;
        total += area;
        if (area < low)
            low = area;
        if (area > high)
            high = area;
    }
    if (!(total >= 0.0 && total <= DBL_MAX))
        return 0;
    *mean = total / (double)mesh->triangles;
    *minimum = low;
    *maximum = high;
    return 1;
}

int
xb_mesh_build_calibrated(xb_mesh *mesh, int triangles, int width,
                         int height, double area, double aspect)
{
    xb_mesh result;
    xb_point *point;
    xb_strip *strip;
    size_t patches, patch, columns, rows, needed_rows, col, row, side;
    size_t first_point, first_facet, triangle, a, b, c;
    double span_x, span_y, double_area, patch_width, patch_height, slots, x0, y0;
    double u, v, mean, minimum, maximum;

    if (mesh == NULL || width < 2 || height < 2 ||
        !(area > 0.0 && area <= DBL_MAX) ||
        !(aspect > 0.0 && aspect <= DBL_MAX))
        return 0;
    memset(&result, 0, sizeof(result));
    result.triangles = xb_mesh_expected_triangles(triangles, 0);
    if (result.triangles == 0)
        return 0;
    span_x = (double)width - 1.0;
    span_y = (double)height - 1.0;
    if (area > span_x * span_y * 0.5)
        return 0;
    /* Reject arithmetic overflow before evaluating the bounding box. Keep
     * each square root intact so exactly fitting integer boxes stay exact. */
    double_area = 2.0 * area;
    if ((aspect > 1.0 && double_area > DBL_MAX / aspect) ||
        (aspect < 1.0 && double_area > DBL_MAX * aspect))
        return 0;
    patch_width = sqrt(double_area * aspect);
    patch_height = sqrt(double_area / aspect);
    if (!(patch_width > 0.0 && patch_width <= span_x) ||
        !(patch_height > 0.0 && patch_height <= span_y))
        return 0;

    patches = result.triangles / 2;
    slots = floor(span_x / patch_width);
    columns = slots >= (double)patches ? patches : (size_t)slots;
    slots = floor(span_y / patch_height);
    rows = slots >= (double)patches ? patches : (size_t)slots;
    needed_rows = (patches + columns - 1) / columns;
    if (rows > needed_rows)
        rows = needed_rows;
    result.point_count = result.triangles * 3;
    result.strip_count = patches;
    result.strip_point_count = patches * 4;
    if (!xb_mesh_allocate(&result))
        return 0;
    xb_texture_init();
    for (patch = 0; patch < patches; ++patch) {
        col = patch % columns;
        row = (patch / columns) % rows;
        /* Spread all available slots over the viewport. If the requested
         * workload does not fit without overlap, cycle through these slots:
         * each receives floor/ceil(patches/slots) identical-area patches.
         * Never shrink geometry or reduce the requested triangle count. */
        x0 = columns == 1 ? (span_x - patch_width) * 0.5 :
             (span_x - patch_width) * (double)col / (double)(columns - 1);
        y0 = rows == 1 ? (span_y - patch_height) * 0.5 :
             (span_y - patch_height) * (double)row / (double)(rows - 1);
        first_point = patch * 4;
        first_facet = patch * 2;
        strip = &result.strips[patch];
        strip->first_point = first_point;
        strip->point_count = 4;
        strip->first_facet = first_facet;
        for (col = 0; col < 2; ++col) {
            for (side = 0; side < 2; ++side) {
                point = &result.strip_points[first_point + col * 2 + side];
                point->x = (float)(2.0 * (x0 + col * patch_width) / span_x - 1.0);
                point->y = (float)(2.0 * (y0 + (1 - side) * patch_height) /
                                   span_y - 1.0);
                /* Equal depth makes every overlapping fragment pass LEQUAL,
                 * keeping this a fill workload rather than a rejection test. */
                point->z = 0.5f;
            }
        }
        for (triangle = 0; triangle < 2; ++triangle) {
            a = first_point + triangle;
            b = a + 1;
            c = a + 2;
            if (triangle & 1) {
                a = first_point + triangle + 1;
                b = a - 1;
            }
            result.points[(first_facet + triangle) * 3] = result.strip_points[a];
            result.points[(first_facet + triangle) * 3 + 1] = result.strip_points[b];
            result.points[(first_facet + triangle) * 3 + 2] = result.strip_points[c];
            xb_store_texcoords(&result, (first_facet + triangle) * 3, first_point,
                a, b, c, 0, 1, (double)(patch % 13) * 0.37,
                (double)(patch % 7) * 0.21, 0.85f);
            u = (double)(patch % 13) * 0.37 + (triangle ? 2.0 : 1.0) / 3.0;
            v = (double)(patch % 7) * 0.21 + (triangle ? 1.0 : 2.0) / 3.0;
            result.base_colors[first_facet + triangle] =
                xb_material_sample(u, v, 0, 0.85f);
            result.colors[first_facet + triangle] =
                result.base_colors[first_facet + triangle];
        }
    }
    /* Reject dimensions too small to retain the requested area in float
     * vertices, instead of quietly returning degenerate calibration meshes. */
    if (!xb_mesh_projected_area(&result, width, height, &mean, &minimum,
                                &maximum) ||
        minimum < area * 0.999 || maximum > area * 1.001) {
        xb_mesh_free(&result);
        return 0;
    }
    xb_mesh_free(mesh);
    *mesh = result;
    return 1;
}

int
xb_mesh_reverse_draw_order(xb_mesh *mesh)
{
    xb_mesh result;
    const xb_strip *source;
    xb_strip *destination;
    size_t i, triangles, facet_number, point_number;

    if (mesh == NULL || mesh->triangles == 0 ||
        mesh->triangles > XB_SCENE_MAX_TRIANGLES ||
        mesh->point_count != 3 * mesh->triangles ||
        mesh->strip_count == 0 || mesh->strip_count > mesh->triangles ||
        mesh->strip_point_count > 3 * mesh->triangles ||
        mesh->points == NULL || mesh->strip_points == NULL ||
        mesh->colors == NULL || mesh->base_colors == NULL || mesh->texcoords == NULL ||
        mesh->strips == NULL || mesh->lighting_ready || mesh->vertex_materials ||
        mesh->vertex_colors)
        return 0;
    memset(&result, 0, sizeof(result));
    result.triangles = mesh->triangles;
    result.point_count = mesh->point_count;
    result.strip_count = mesh->strip_count;
    result.strip_point_count = mesh->strip_point_count;
    if (!xb_mesh_allocate(&result))
        return 0;

    facet_number = point_number = 0;
    for (i = 0; i < mesh->strip_count; ++i) {
        source = &mesh->strips[mesh->strip_count - 1 - i];
        if (source->point_count < 3 ||
            source->first_point > mesh->strip_point_count ||
            source->point_count > mesh->strip_point_count - source->first_point ||
            source->point_count > mesh->strip_point_count - point_number) {
            xb_mesh_free(&result);
            return 0;
        }
        triangles = source->point_count - 2;
        if (source->first_facet > mesh->triangles ||
            triangles > mesh->triangles - source->first_facet ||
            triangles > mesh->triangles - facet_number) {
            xb_mesh_free(&result);
            return 0;
        }
        destination = &result.strips[i];
        destination->first_point = point_number;
        destination->point_count = source->point_count;
        destination->first_facet = facet_number;
        /* Reverse submission groups only. Reversing their internal vertices
         * would change strip parity/winding and potentially facet-color
         * associations. Keeping each strip intact makes all methods consume
         * exactly the same reordered geometry and material records. */
        memcpy(result.strip_points + point_number,
               mesh->strip_points + source->first_point,
               source->point_count * sizeof(xb_point));
        memcpy(result.points + facet_number * 3,
               mesh->points + source->first_facet * 3,
               triangles * 3 * sizeof(xb_point));
        memcpy(result.texcoords + facet_number * 3,
               mesh->texcoords + source->first_facet * 3,
               triangles * 3 * sizeof(xb_texcoord));
        memcpy(result.colors + facet_number,
               mesh->colors + source->first_facet,
               triangles * sizeof(xb_rgb));
        memcpy(result.base_colors + facet_number,
               mesh->base_colors + source->first_facet,
               triangles * sizeof(xb_rgb));
        point_number += source->point_count;
        facet_number += triangles;
    }
    if (point_number != mesh->strip_point_count || facet_number != mesh->triangles) {
        xb_mesh_free(&result);
        return 0;
    }
    xb_mesh_free(mesh);
    *mesh = result;
    return 1;
}

void
xb_mesh_colors(xb_mesh *mesh, unsigned long frame)
{
    size_t i;
    unsigned long phase;
    int ramp;
    float light;

    if (mesh == NULL || mesh->colors == NULL || mesh->base_colors == NULL)
        return;
    /* A cheap deterministic lightstyle-like ramp changes the supplied facet
     * colors. Always use cached base colors, so repeated updates never drift. */
    phase = frame % 128UL;
    for (i = 0; i < mesh->triangles; ++i) {
        ramp = (int)((phase + (i % 128) * 13) % 128);
        if (ramp > 64)
            ramp = 128 - ramp;
        light = 0.45f + 0.55f * (float)ramp / 64.0f;
        mesh->colors[i].r = mesh->base_colors[i].r * light;
        mesh->colors[i].g = mesh->base_colors[i].g * light;
        mesh->colors[i].b = mesh->base_colors[i].b * light;
    }
}

const char *
xb_method_name(enum xb_method method)
{
    switch (method) {
    case XB_POLYGON: return "polygon";
    case XB_MULTI: return "multi_polygon";
    case XB_TLIST: return "triangle_list";
    case XB_STRIP: return "triangle_strip";
    }
    return "unknown";
}
