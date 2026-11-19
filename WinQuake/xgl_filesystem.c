/* Shareware data compatibility for XGLQuake. GPL-2.0-or-later. */
#include "quakedef.h"

int XGL_CheckPackDirectory(const char *path, int count, unsigned short crc)
{
    const char *version;
    version = NULL;
    /* Official v1.0x pak0 directory fingerprints, also recognized by
     * Quakespasm. These describe the directory, not a whole-file checksum. */
    if (count == 339) {
        if (crc == 13900) version = "1.00";
        else if (crc == 62751) version = "1.01";
        else if (crc == 32981) version = "1.06";
    }
    if (version) {
        Con_Printf("XGL pack directory: %s: %d files, CRC %u; recognized Quake %s pak0\n",
                   path, count, (unsigned)crc, version);
        return 1;
    }
    Con_Printf("XGL pack directory: %s: %d files, CRC %u; unrecognized shareware directory\n",
               path, count, (unsigned)crc);
    /* Do not clear com_modified: another pack or a game/path override can
     * already have set it. Registration and mod rules belong to common.c. */
    return 0;
}

void XGL_SharewareDiagnostics(void)
{
    int index;
    index = COM_CheckParm("-game");
    if (index && index + 1 < com_argc)
        Con_Printf("XGL shareware: -game %s selects a modified game path.\n", com_argv[index + 1]);
    if (COM_CheckParm("-path"))
        Con_Printf("XGL shareware: -path selects a modified search path.\n");
    Con_Printf("For standard shareware, use id1/pak0.pak from Quake 1.00, 1.01 or 1.06,\n"
               "without extra PAK files, -game or -path. Use -basedir to select its parent directory.\n"
               "The pack-directory messages above identify unrecognized files.\n");
}
