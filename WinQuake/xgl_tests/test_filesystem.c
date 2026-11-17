/* Host regression for shareware compatibility and retained registration rules.
 * Synthetic in-memory PAK directories only; no Quake game assets are included.
 * GPL-2.0-or-later. */
#include <assert.h>
#include <setjmp.h>
#include <stdarg.h>
#include "../common.c"

#define TEST_PAK_LIMIT (12 + 340*64 + 256)
static byte test_pak[TEST_PAK_LIMIT];
static int test_size, test_position, test_reads;
static char test_log[8192], test_error[256];
static size_t test_log_used;
static jmp_buf test_error_jump;
static int test_expect_error, test_registered_sets;
static union { double alignment; byte bytes[65536]; } test_hunk;
static int test_hunk_used;
static searchpath_t test_search;

void Sys_Error (char *format, ...)
{
    va_list args;
    va_start (args, format);
    vsnprintf (test_error, sizeof(test_error), format, args);
    va_end (args);
    if (test_expect_error) longjmp (test_error_jump, 1);
    fprintf (stderr, "unexpected error: %s\n", test_error);
    abort ();
}

void Con_Printf (char *format, ...)
{
    va_list args;
    int count;
    va_start (args, format);
    count = vsnprintf (test_log + test_log_used, sizeof(test_log)-test_log_used,
                       format, args);
    va_end (args);
    assert (count >= 0 && (size_t)count < sizeof(test_log)-test_log_used);
    test_log_used += count;
}

void Sys_Printf (char *format, ...)
{
    (void)format;
}

void *Hunk_AllocName (int size, char *name)
{
    void *result;
    (void)name;
    assert (size > 0 && test_hunk_used + size < (int)sizeof(test_hunk.bytes));
    result = test_hunk.bytes + test_hunk_used;
    test_hunk_used += (size+15)&~15;
    memset (result, 0, size);
    return result;
}

void *Hunk_Alloc (int size)
{
    return Hunk_AllocName (size, "test");
}

int Sys_FileOpenRead (char *path, int *handle)
{
    assert (!strcmp(path, "synthetic/id1/pak0.pak"));
    *handle = 1;
    test_position = 0;
    return test_size;
}

int Sys_FileRead (int handle, void *destination, int count)
{
    assert (handle == 1 && count >= 0 && test_position+count <= test_size);
    memcpy (destination, test_pak+test_position, count);
    test_position += count;
    ++test_reads;
    return count;
}

void Sys_FileSeek (int handle, int position)
{
    assert (handle == 1 && position >= 0 && position <= test_size);
    test_position = position;
}

void Sys_FileClose (int handle)
{
    assert (handle == 1);
}

/* COM_FindFile references disk/cache fallbacks; these fixtures use only PAKs. */
int Sys_FileTime (char *path) { (void)path; return -1; }
int Sys_FileOpenWrite (char *path) { (void)path; assert (0); return -1; }
int Sys_FileWrite (int handle, void *data, int count)
{
    (void)handle; (void)data; (void)count; assert (0); return 0;
}
void Sys_mkdir (char *path) { (void)path; assert (0); }

void Cvar_Set (char *name, char *value)
{
    if (!strcmp(name, "registered"))
    {
        assert (!strcmp(value, "1"));
        registered.value = 1;
        ++test_registered_sets;
    }
    else
        assert (!strcmp(name, "cmdline"));
}

static void put_le32 (byte *out, unsigned value)
{
    out[0] = value;
    out[1] = value>>8;
    out[2] = value>>16;
    out[3] = value>>24;
}

static unsigned short directory_crc (const byte *data, int length)
{
    unsigned short crc;
    int i;
    CRC_Init (&crc);
    for (i = 0; i < length; ++i) CRC_ProcessByte (&crc, data[i]);
    return crc;
}

static void make_pak (int count, unsigned short wanted_crc, int with_pop)
{
    byte *directory, *entry, *data;
    unsigned short prefix, crc;
    int i, guess, patch, directory_size;
    assert (count > 0 && count <= 340);
    assert (sizeof(dpackfile_t) == 64 && sizeof(dpackheader_t) == 12);
    memset (test_pak, 0, sizeof(test_pak));
    directory_size = count*64;
    test_size = 12+directory_size+256;
    memcpy (test_pak, "PACK", 4);
    put_le32 (test_pak+4, 12);
    put_le32 (test_pak+8, directory_size);
    directory = test_pak+12;
    data = directory+directory_size;
    for (i = 0; i < count; ++i)
    {
        entry = directory+i*64;
        sprintf ((char *)entry, "synthetic%03d", i);
        put_le32 (entry+56, 12+directory_size);
        put_le32 (entry+60, 256);
    }
    if (with_pop)
    {
        strcpy ((char *)directory, "gfx/pop.lmp");
        /* Registration data is stored big-endian, independent of PAK fields. */
        for (i = 0; i < 128; ++i)
        {
            data[i*2] = pop[i]>>8;
            data[i*2+1] = pop[i];
        }
    }
    /* Two ignored bytes after the final filename terminator select a test
     * fingerprint. This is a synthetic CRC fixture, not an official archive. */
    patch = directory_size-10;
    prefix = directory_crc (directory, patch);
    for (guess = 0; guess <= 65535; ++guess)
    {
        crc = prefix;
        CRC_ProcessByte (&crc, guess&255);
        CRC_ProcessByte (&crc, guess>>8);
        for (i = patch+2; i < directory_size; ++i)
            CRC_ProcessByte (&crc, directory[i]);
        if (crc == wanted_crc) break;
    }
    assert (guess <= 65535);
    directory[patch] = guess&255;
    directory[patch+1] = guess>>8;
    assert (directory_crc(directory, directory_size) == wanted_crc);
}

static pack_t *load_fixture (int count, unsigned short crc, int modified, int with_pop)
{
    pack_t *pack;
    make_pak (count, crc, with_pop);
    test_log_used = 0;
    test_log[0] = test_error[0] = 0;
    test_reads = test_hunk_used = test_registered_sets = 0;
    registered.value = 0;
    static_registered = 1;
    com_modified = modified;
    com_searchpaths = NULL;
    pack = COM_LoadPackFile ("synthetic/id1/pak0.pak");
    assert (pack && pack->numfiles == count && test_reads == 2);
    assert (pack->files[0].filepos == 12+count*64 && pack->files[0].filelen == 256);
    memset (&test_search, 0, sizeof(test_search));
    test_search.pack = pack;
    com_searchpaths = &test_search;
    return pack;
}

static void registration_error (const char *message)
{
    test_expect_error = 1;
    if (!setjmp(test_error_jump))
    {
        COM_CheckRegistered ();
        assert (!"expected registration rejection");
    }
    test_expect_error = 0;
    assert (strstr(test_error, message));
    assert (!static_registered && !registered.value && !test_registered_sets);
}

int main (void)
{
    static const unsigned short accepted[] = {13900, 62751, 32981};
    static const char *versions[] = {"Quake 1.00", "Quake 1.01", "Quake 1.06"};
    unsigned native_one;
    byte little_one[4] = {1,0,0,0};
    int i;
    memcpy (&native_one, little_one, sizeof(native_one));
    LittleLong = native_one == 1 ? LongNoSwap : LongSwap;
    BigShort = native_one == 1 ? ShortSwap : ShortNoSwap;
    assert (directory_crc((const byte *)"123456789", 9) == 0x29b1);
    for (i = 0; i < 3; ++i)
    {
        load_fixture (339, accepted[i], 0, 0);
        assert (!com_modified && strstr(test_log, versions[i]));
        COM_CheckRegistered ();
        assert (!static_registered && !registered.value && !test_registered_sets);
        assert (strstr(test_log, "Playing shareware version."));
    }
    load_fixture (339, 12345, 0, 0);
    assert (com_modified && strstr(test_log, "339 files, CRC 12345"));
    registration_error ("registered version to use modified games");
    load_fixture (340, 32981, 0, 0);
    assert (com_modified && strstr(test_log, "340 files, CRC 32981"));
    assert (strstr(test_log, "unrecognized shareware directory"));
    registration_error ("registered version to use modified games");
    load_fixture (308, 28804, 0, 0); /* pre-release data remains unsupported */
    assert (com_modified && strstr(test_log, "unrecognized shareware directory"));
    registration_error ("registered version to use modified games");

    /* An accepted pak cannot erase a prior extra-pak/-game/-path reason. */
    for (i = 0; i < 3; ++i)
    {
        load_fixture (339, accepted[i], 1, 0);
        assert (com_modified);
        registration_error ("registered version to use modified games");
    }
    load_fixture (339, 32981, 1, 1);
    COM_CheckRegistered ();
    assert (com_modified && static_registered && registered.value == 1);
    assert (test_registered_sets == 1 && test_reads == 3);
    load_fixture (339, 32981, 0, 1);
    test_pak[test_size-1] ^= 1;
    registration_error ("Corrupted data file");
    assert (test_reads == 3);
    puts ("XGL filesystem: accepted versions, raw-directory CRC, sticky modifications and registration checks passed");
    return 0;
}
