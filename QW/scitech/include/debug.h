/****************************************************************************
*
*                   Copyright (C) 1996 SciTech Software
*                           All rights reserved.
*
* Filename:     $Workfile:   debug.h  $
* Version:      $Revision:   1.17  $
*
* Language:     ANSI C
* Environment:  any
*
* Description:  General header file for operating system portable code.
*
* $Date:   04 Mar 1997 11:34:08  $ $Author:   KendallB  $
*
****************************************************************************/

#ifndef __DEBUG_H
#define __DEBUG_H

/* We have the following defines to identify the compilation environment:
 *
 *	__16BIT__		Compiling for 16 bit code (any environment)
 *  __32BIT__       Compiling for 32 bit code (any environment)
 *	__MSDOS__		Compiling for MS-DOS (includes __WINDOWS16__, __WIN386__)
 *  __REALDOS__     Compiling for MS-DOS (excludes __WINDOWS16__)
 *  __MSDOS16__ 	Compiling for 16 bit MS-DOS
 *  __MSDOS32__ 	Compiling for 32 bit MS-DOS
 *  __WINDOWS__ 	Compiling for Windows
 *	__WINDOWS16__	Compiling for 16 bit Windows (__MSDOS__ also defined)
 *	__WINDOWS32__	Compiling for 32 bit Windows
 *  __WIN386__      Compiling for Watcom C++ Win386 extended Windows
 *  __OS2__     	Compiling for OS/2
 *  __OS2_16__  	Compiling for 16 bit OS/2
 *  __OS2_32__ 		Compiling for 32 bit OS/2
 *  __UNIX__   		Compiling for Unix
 *
 */

#ifdef	__SC__
#if		__INTSIZE == 4
#define __SC386__
#endif
#endif

#ifdef	__GNUC__
#define	__cdecl			/* GCC doesn't know about __cdecl modifiers		*/
#define	__FLAT__		/* GCC is always 32 bit flat model				*/
#define	__HAS_BOOL__	/* Latest GNU C++ has bool type					*/
#endif

#ifdef	__BORLANDC__
#if	(__BORLANDC__ >= 0x500) || defined(CLASSLIB_DEFS_H)
#define	__HAS_BOOL__	/* Borland C++ 5.0 and later define bool type	*/
#endif
#endif

/* For the Metaware High C/C++ compiler, there is no _cdecl calling
 * convention. The conventions can be changed, but it is a complicated
 * process involving #pragmas, and all externally referenced functions
 * will use stack based calling conventions. We also need to change the
 * global aliasing conventions to use underscores for external function
 * and variables names, so that our assembler routines will link
 * correctly (except of course the main function - man what a PAIN!).
 */

#ifdef	__HIGHC__
#define	__cdecl
#define	__FLAT__	/* High C is always 32 bit flat model				*/
#pragma Global_aliasing_convention("_%r")
extern main();
#pragma Alias(main,"main")
#endif

#if     defined(__MSDOS__) || defined(__DOS__) || defined(__DPMI32__) || (defined(M_I86) && !defined(__SC386__))
#ifndef __MSDOS__
#define __MSDOS__
#endif
#if     defined(__386__) || defined(__FLAT__) || defined(__NT__) || defined(__SC386__)
#ifndef	__MSDOS32__
#define __MSDOS32__
#endif
#ifndef	__32BIT__
#define __32BIT__
#endif
#ifndef __REALDOS__
#define __REALDOS__
#endif
#elif   (defined(_Windows) || defined(_WINDOWS)) && !defined(__DPMI16__)
#ifndef	__16BIT__
#define	__16BIT__
#endif
#ifndef __WINDOWS16__
#define	__WINDOWS16__
#endif
#ifndef __WINDOWS__
#define __WINDOWS__
#endif
#ifndef	__MSDOS__
#define	__MSDOS__
#endif
#else
#ifndef	__16BIT__
#define	__16BIT__
#endif
#ifndef __MSDOS16__
#define __MSDOS16__
#endif
#ifndef __REALDOS__
#define __REALDOS__
#endif
#endif
#elif   defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#ifndef	__32BIT__
#define __32BIT__
#endif
#ifndef	__WINDOWS32__
#define	__WINDOWS32__
#endif
#ifndef	_WIN32
#define	_WIN32					/* Microsoft Win32 SDK headers use _WIN32 */
#endif
#ifndef	WIN32
#define	WIN32					/* OpenGL headers use WIN32 */
#endif
#ifndef __WINDOWS__
#define __WINDOWS__
#endif
#elif   defined(__WINDOWS_386__)
#ifndef	__32BIT__
#define __32BIT__
#endif
#ifndef __WIN386__
#define __WIN386__
#endif
#ifndef __WINDOWS__
#define __WINDOWS__
#endif
#ifndef	__MSDOS__
#define	__MSDOS__
#endif
#elif   defined(__OS2__)
#ifndef __OS2__                 /* TODO: to be completed */
#define __OS2__
#define	__OS2_32__				/* Default to 32 bit OS/2 */
#endif
#else
#define __UNIX__				/* TODO: to be completed */
#endif

/* We have the following defines to define the calling conventions for
 * publicly accesible functions:
 *
 *  _PUBAPI - Compiler default calling conventions for all public 'C' functions
 *  _ASMAPI - Calling conventions for all public assembler functions
 *  _DLLAPI - Calling conventions for all DLL exported functions
 *  _DLLVAR - Modifier to export/import globals in 32 bit DLL's
 *  _EXPORT - Expands to _export when compiling a DLL
 *  _VARAPI - Modifiers for variables; Watcom C++ mangles C++ globals
 */

#define _PUBAPI	
#define _ASMAPI __cdecl

#if defined(_MSC_VER) && defined(_WIN32) && !defined(__SC__)
#define __PASCAL    __stdcall
#define __export
#define	__import
#else
#define __PASCAL    __pascal
#endif

#if defined(__WATCOMC__)
#define	_VARAPI		__cdecl
#else
#define	_VARAPI
#endif

#if     defined(__WINDOWS__)
#ifdef  BUILD_DLL
#define _DLLASM __export __cdecl
#define _EXPORT __export
#ifdef  __WINDOWS32__
#define _DLLAPI __export __PASCAL
#define _DLLVAR __export
#else
#define _DLLAPI  __export __far __pascal
#define _DLLVAR
#endif
#else
#define _DLLASM __cdecl
#define	_EXPORT
#ifdef  __WINDOWS32__
#define _DLLAPI __PASCAL
#define _DLLVAR __import
#else
#define _DLLAPI __far __pascal
#define _DLLVAR
#endif
#endif
#else
#define _EXPORT
#define _DLLAPI
#define _DLLVAR
#endif

/* Useful macros */

#define PRIVATE static
#define PUBLIC

#ifdef  DEBUG
#       define DBG(x) x
#else
#       define DBG(x)
#endif

#ifndef NULL
#       define NULL 0L
#endif

#ifndef MAX
#       define MAX(a,b) ( ((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#       define MIN(a,b) ( ((a) < (b)) ? (a) : (b))
#endif
#ifndef ABS
#       define ABS(a)   ((a) >= 0 ? (a) : -(a))
#endif
#ifndef	SIGN
#		define SIGN(a)	((a) > 0 ? 1 : -1)
#endif

/* General typedefs */

#ifndef __GENDEFS
#define __GENDEFS
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;
typedef int             ibool;		/* Integer boolean type			*/
#ifndef	__cplusplus
#define	bool			ibool		/* Standard C					*/
#else
#ifndef	__HAS_BOOL__
#define	bool			ibool		/* Older C++ compilers			*/
#endif
#endif	/* __cplusplus */
#endif  /* __GENDEFS */

/* Includes Windows headers, as they define TRUE and FALSE */

#ifdef	__WINDOWS__
#ifndef _WINDOWS_                   /* Dont include if already included */
#ifndef __WIN386__
#define STRICT
#define	WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#endif

/* Boolean truth values */

#undef	false
#undef	true
#undef	NO
#undef	YES
#undef	FALSE
#undef	TRUE

#ifdef	__cplusplus					/* Cast to bool's for C++ code		*/
#define false       ((bool)0)
#define true        ((bool)1)
#define NO          ((bool)0)
#define YES         ((bool)1)
#define FALSE       ((bool)0)
#define TRUE        ((bool)1)
#else								/* Define to 0 and 1 for C code		*/
#define false       0
#define true        1
#define NO          0
#define YES         1
#define FALSE       0
#define TRUE        1
#endif

#endif  /* __DEBUG_H */
