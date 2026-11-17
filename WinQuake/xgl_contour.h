/* Bounded planar texture-contour approximation. GPL-2.0-or-later. */
#ifndef XGL_CONTOUR_H
#define XGL_CONTOUR_H

#define XGL_CONTOUR_MAX_FACE_VERTICES 32
#define XGL_CONTOUR_MAX_TRIANGLES 128

typedef struct {
    float position[3];
    float st[2];
} xgl_contour_point_t;

typedef struct {
    xgl_contour_point_t point;
    unsigned char color[3];
} xgl_contour_vertex_t;

typedef struct {
    unsigned long samples, evaluations;
    int candidates, cuts;
} xgl_contour_stats_t;

/* st is normalized, repeating texture space. Output has room for
 * 3 * triangle_budget vertices. Return the triangle count, or zero to keep
 * the caller's ordinary mesh. No allocation or source pointer is retained.
 * The optional service callback runs periodically during bounded work. */
int XGL_ContourBuild(const xgl_contour_point_t *face, int face_count,
    const unsigned char *pixels, int width, int height,
    const unsigned char *palette, int triangle_budget,
    xgl_contour_vertex_t *output, xgl_contour_stats_t *stats,
    void (*service)(void));

#endif
