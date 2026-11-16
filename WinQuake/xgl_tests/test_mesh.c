/* Alias command/pose contract tests. GPL-2.0-or-later. */
#include "quakedef.h"
#include <assert.h>
#include <stdarg.h>

aliashdr_t *pheader;
stvert_t stverts[MAXALIASVERTS];
mtriangle_t triangles[MAXALIASTRIS];
trivertx_t *poseverts[MAXALIASFRAMES];
static trivertx_t source_poses[2][MAXALIASVERTS];
static union { double aligned; byte bytes[1024*1024]; } hunk;
static int hunk_used;
static int consumed[MAXALIASTRIS];

void Sys_Error (char *format, ...)
{
    va_list args;
    va_start (args, format);
    vfprintf (stderr, format, args);
    va_end (args);
    abort ();
}

void Con_DPrintf (char *format, ...)
{
    (void)format;
}

void *Hunk_Alloc (int size)
{
    void *result;
    assert (size > 0 && hunk_used + size < (int)sizeof(hunk.bytes));
    result = hunk.bytes + hunk_used;
    hunk_used += (size+15)&~15;
    memset (result, 0, size);
    return result;
}

static void setup (int count, int vertices)
{
    int i, pose;
    hunk_used = 0;
    memset (hunk.bytes, 0, sizeof(hunk.bytes));
    memset (consumed, 0, sizeof(consumed));
    pheader = Hunk_Alloc (sizeof(*pheader));
    pheader->numtris = count;
    pheader->numverts = vertices;
    pheader->numposes = 2;
    pheader->skinwidth = pheader->skinheight = 64;
    for (i = 0; i < vertices; ++i)
    {
        stverts[i].s = (i*3)%32;
        stverts[i].t = (i*7)%64;
        stverts[i].onseam = i%3 == 0;
        for (pose = 0; pose < 2; ++pose)
        {
            source_poses[pose][i].v[0] = i&255;
            source_poses[pose][i].v[1] = i>>8;
            source_poses[pose][i].v[2] = pose;
            source_poses[pose][i].lightnormalindex = i%162;
        }
    }
    poseverts[0] = source_poses[0];
    poseverts[1] = source_poses[1];
}

static void check_triangle (const int *indices, const float uv[][2])
{
    int i, rotation, corner, index, match;
    float expected_s, expected_t;
    for (i = 0; i < pheader->numtris; ++i)
    {
        if (consumed[i]) continue;
        for (rotation = 0; rotation < 3; ++rotation)
        {
            match = 1;
            for (corner = 0; corner < 3; ++corner)
            {
                index = triangles[i].vertindex[(corner+rotation)%3];
                expected_s = stverts[index].s;
                if (!triangles[i].facesfront && stverts[index].onseam)
                    expected_s += pheader->skinwidth/2;
                expected_s = (expected_s+.5f)/pheader->skinwidth;
                expected_t = (stverts[index].t+.5f)/pheader->skinheight;
                if (indices[corner] != index || fabs(uv[corner][0]-expected_s) > .000001 ||
                    fabs(uv[corner][1]-expected_t) > .000001)
                    match = 0;
            }
            if (match) { consumed[i] = 1; return; }
        }
    }
    assert (!"commands changed triangle winding, UV seams or topology");
}

static int check_commands (void)
{
    int *commands;
    trivertx_t *poses;
    int order[MAXALIASTRIS*3];
    float uv[MAXALIASTRIS*3][2], triangle_uv[3][2];
    int count, fan, offset, vertex, index, pose, i, word, triangles_seen;
    int corners[3], indices[3];
    commands = (int *)((byte *)pheader + pheader->commands);
    poses = (trivertx_t *)((byte *)pheader + pheader->posedata);
    offset = word = triangles_seen = 0;
    while ((count = commands[word++]) != 0)
    {
        fan = count < 0;
        if (fan) count = -count;
        assert (count >= 3 && offset+count <= pheader->poseverts);
        for (vertex = 0; vertex < count; ++vertex)
        {
            index = poses[offset+vertex].v[0] + (poses[offset+vertex].v[1]<<8);
            assert (index >= 0 && index < pheader->numverts);
            order[vertex] = index;
            memcpy (&uv[vertex][0], &commands[word++], sizeof(float));
            memcpy (&uv[vertex][1], &commands[word++], sizeof(float));
            for (pose = 0; pose < 2; ++pose)
                assert (!memcmp(&poses[pose*pheader->poseverts+offset+vertex],
                                &source_poses[pose][index], sizeof(trivertx_t)));
        }
        for (vertex = 2; vertex < count; ++vertex)
        {
            corners[0] = fan ? 0 : (vertex&1 ? vertex-1 : vertex-2);
            corners[1] = fan ? vertex-1 : (vertex&1 ? vertex-2 : vertex-1);
            corners[2] = vertex;
            for (i = 0; i < 3; ++i)
            {
                indices[i] = order[corners[i]];
                memcpy (triangle_uv[i], uv[corners[i]], sizeof(triangle_uv[i]));
            }
            check_triangle (indices, triangle_uv);
            ++triangles_seen;
        }
        offset += count;
    }
    assert (offset == pheader->poseverts && triangles_seen == pheader->numtris);
    for (i = 0; i < pheader->numtris; ++i) assert (consumed[i]);
    return word;
}

int main (void)
{
    static const int strip[4][3] = {{0,1,2},{2,1,3},{2,3,4},{4,3,5}};
    model_t model;
    int i, words, saved_words, saved_poseverts, saved_bytes;
    byte *snapshot;
    memset (&model, 0, sizeof(model));
    strcpy (model.name, "synthetic.mdl");
    setup (8, 6);
    for (i = 0; i < 8; ++i)
    {
        triangles[i].facesfront = i < 4;
        memcpy (triangles[i].vertindex, strip[i%4], sizeof(strip[0]));
    }
    XGL_MakeAliasModelDisplayLists (&model, pheader);
    check_commands ();

    /* Equal directed triangles cannot join a strip/fan. This reaches the
     * worst-case 7 command words and 3 reordered vertices per triangle. */
    setup (MAXALIASTRIS, 3);
    for (i = 0; i < MAXALIASTRIS; ++i)
    {
        triangles[i].facesfront = 1;
        triangles[i].vertindex[0] = 0;
        triangles[i].vertindex[1] = 1;
        triangles[i].vertindex[2] = 2;
    }
    XGL_MakeAliasModelDisplayLists (&model, pheader);
    assert (check_commands () == MAXALIASTRIS*7+1);
    assert (pheader->poseverts == MAXALIASTRIS*3);

    /* A connected 2048-triangle fan exceeds the old 128-entry strip scratch.
     * Repeated edges are legal and exercise the maximum bounded command path. */
    setup (MAXALIASTRIS, MAXALIASVERTS);
    for (i = 0; i < MAXALIASTRIS; ++i)
    {
        triangles[i].facesfront = 1;
        triangles[i].vertindex[0] = 0;
        triangles[i].vertindex[1] = 1+i%(MAXALIASVERTS-1);
        triangles[i].vertindex[2] = 1+(i+1)%(MAXALIASVERTS-1);
    }
    XGL_MakeAliasModelDisplayLists (&model, pheader);
    words = check_commands ();
    assert (pheader->poseverts >= MAXALIASTRIS+2 && pheader->poseverts <= MAXALIASTRIS*3);
    saved_words = words;
    saved_poseverts = pheader->poseverts;
    saved_bytes = hunk_used;
    snapshot = malloc (saved_bytes);
    assert (snapshot);
    memcpy (snapshot, hunk.bytes, saved_bytes);
    setup (MAXALIASTRIS, MAXALIASVERTS);
    XGL_MakeAliasModelDisplayLists (&model, pheader);
    assert (check_commands () == saved_words && pheader->poseverts == saved_poseverts);
    assert (hunk_used == saved_bytes && !memcmp(snapshot, hunk.bytes, saved_bytes));
    free (snapshot);
    puts ("XGL alias mesh: topology, seams, pose reordering, 2048-triangle bounds and determinism passed");
    return 0;
}
