//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id$
//**
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
//**
//**	This program is free software; you can redistribute it and/or
//**  modify it under the terms of the GNU General Public License
//**  as published by the Free Software Foundation; either version 2
//**  of the License, or (at your option) any later version.
//**
//**	This program is distributed in the hope that it will be useful,
//**  but WITHOUT ANY WARRANTY; without even the implied warranty of
//**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//**  GNU General Public License for more details.
//**
//**************************************************************************

#ifdef _MSC_VER
// Disable some unwanted warnings
#pragma warning(disable : 4097) // typedef-name '' used as synonym for class-name ''
#pragma warning(disable : 4127) // Conditional expression is constant
#pragma warning(disable : 4244) // conversion to float, possible loss of data
#pragma warning(disable : 4284) // return type is not a UDT or reference to a UDT
#pragma warning(disable : 4291) // no matching operator delete found
#pragma warning(disable : 4305) // truncation from 'const double' to 'float'
#pragma warning(disable : 4512) // assignment operator could not be generated
#pragma warning(disable : 4514)	// unreferenced inline function has been removed
#pragma warning(disable : 4702) // unreachable code in inline expanded function
#pragma warning(disable : 4710) // inline function not expanded

// Workaround of for variable scoping
#define for		if (0) ; else for
#endif

#ifdef __BORLANDC__
// Disable some unwanted warnings
#pragma warn -8027		// inline function not expanded
#pragma warn -8071		// conversation may loose significant digits
#endif

#ifndef __GNUC__
#define __attribute__(whatever)
#endif

#ifndef _WIN32
#define __declspec(whatever)
#endif

#if !defined _WIN32 && !defined DJGPP
#undef stricmp	//	Allegro defines them
#undef strnicmp
#define stricmp		strcasecmp
#define strnicmp	strncasecmp
#endif

//==========================================================================
//
//	Basic types
//
//==========================================================================

#define MINCHAR		((char)-128)
#define MINSHORT	((short)-32768)
#define MININT		((int)-2147483648)
#define MINLONG		((long)-2147483648)

#define MAXCHAR		((char)0x7f)
#define MAXSHORT	((short)0x7fff)
#define MAXINT		((int)0x7fffffff)
#define MAXLONG		((long)0x7fffffff)

enum ENoInit { E_NoInit };

typedef int					boolean;	//	Must be 4 bytes long
typedef unsigned char 		byte;
typedef unsigned short	 	word;
typedef unsigned long	 	dword;

typedef char				vint8;
typedef unsigned char		vuint8;
typedef short				vint16;
typedef unsigned short		vuint16;
typedef int					vint32;
typedef unsigned int		vuint32;

//==========================================================================
//
//	Standard macros
//
//==========================================================================

//	Number of elements in an array.
#define ARRAY_COUNT(array)				(sizeof(array) / sizeof((array)[0]))

//	Offset of a struct member.
#define STRUCT_OFFSET(struc, member)	((int)&((struc *)NULL)->member)

//==========================================================================
//
//	Guard macros
//
//==========================================================================

//	Turn on usage of context in guard macros on platforms where it's not
// safe to throw an exception in signal handler.
#ifdef __linux__
#define USE_GUARD_SIGNAL_CONTEXT
#endif

#ifdef USE_GUARD_SIGNAL_CONTEXT
#include <setjmp.h>
//	Stack control.
class __Context
{
public:
	__Context() { memcpy(&Last, &Env, sizeof(jmp_buf)); }
	~__Context() { memcpy(&Env, &Last, sizeof(jmp_buf)); }
	static jmp_buf Env;
	static const char* ErrToThrow;

protected:
	jmp_buf Last;
};
#endif

#if defined(_DEBUG) || !DO_GUARD
#define guard(name)		static const char __FUNC_NAME__[] = #name; {
#define unguard			}
#define unguardf(msg)	}
#elif defined(USE_GUARD_SIGNAL_CONTEXT)
#define guard(name)		{static const char __FUNC_NAME__[] = #name; \
	__Context __LOCAL_CONTEXT__; try { if (setjmp(__Context::Env)) { \
	throw VavoomError(__Context::ErrToThrow); } else {
#define unguard			}} catch (RecoverableError &e) { throw e; } \
	catch (...) { Host_CoreDump(__FUNC_NAME__); throw; }}
#define unguardf(msg)	}} catch (RecoverableError &e) { throw e; } \
	catch (...) { Host_CoreDump(__FUNC_NAME__); Host_CoreDump msg; throw; }}
#else
#define guard(name)		{static const char __FUNC_NAME__[] = #name; try {
#define unguard			} catch (RecoverableError &e) { throw e; } \
	catch (...) { Host_CoreDump(__FUNC_NAME__); throw; }}
#define unguardf(msg)	} catch (RecoverableError &e) { throw e; } \
	catch (...) { Host_CoreDump(__FUNC_NAME__); Host_CoreDump msg; throw; }}
#endif

#if !defined(_DEBUG) && DO_GUARD_SLOW
#define guardSlow(name)		guard(name)
#define unguardSlow			unguard
#define unguardfSlow(msg)	unguardf(msg)
#else
#define guardSlow(name)		{
#define unguardSlow			}
#define unguardfSlow(msg)	}
#endif

//==========================================================================
//
//	Errors
//
//==========================================================================

#define MAX_ERROR_TEXT_SIZE		1024

class VavoomError
{
public:
	char message[MAX_ERROR_TEXT_SIZE];

	explicit VavoomError(const char *text)
	{
		strncpy(message, text, MAX_ERROR_TEXT_SIZE - 1);
		message[MAX_ERROR_TEXT_SIZE - 1] = 0;
	}
};

class RecoverableError : public VavoomError
{
public:
	explicit RecoverableError(const char *text) : VavoomError(text) { }
};

//==========================================================================
//
//	Assertion macros
//
//==========================================================================

#if DO_CHECK
#define check(e)	if (!(e)) throw VavoomError("Assertion failed: " #e)
#define verify(e)	if (!(e)) throw VavoomError("Assertion failed: " #e)
#else
#define check(e)
#define verify(e)	(e)
#endif

#if DO_CHECK_SLOW
#define checkSlow(e)	check(e)
#define verifySlow(e)	verify(e)
#else
#define checkSlow(e)
#define verifySlow(e)	(e)
#endif

//==========================================================================
//
//	Forward declarations
//
//==========================================================================

class	VName;
class	VStr;
class	VStream;

class	VField;
class	VStruct;
class	VClass;

class	VObject;

//**************************************************************************
//
//	$Log$
//	Revision 1.24  2006/03/04 16:01:34  dj_jl
//	File system API now uses strings.
//
//	Revision 1.23  2006/02/27 20:45:26  dj_jl
//	Rewrote names class.
//	
//	Revision 1.22  2006/02/27 18:43:41  dj_jl
//	Added definitions for common integer types.
//	
//	Revision 1.21  2006/02/26 14:43:39  dj_jl
//	Added workaround to for statement variable scoping.
//	
//	Revision 1.20  2006/02/25 17:14:19  dj_jl
//	Implemented proper serialisation of the objects.
//	
//	Revision 1.19  2006/02/22 20:33:51  dj_jl
//	Created stream class.
//	
//	Revision 1.18  2006/02/10 22:17:00  dj_jl
//	Some platform fixes.
//	
//	Revision 1.17  2003/10/22 06:15:00  dj_jl
//	Safer handling of signals in Linux
//	
//	Revision 1.16  2003/03/08 11:33:39  dj_jl
//	Got rid of some warnings.
//	
//	Revision 1.15  2002/11/16 17:14:22  dj_jl
//	Some changes for release.
//	
//	Revision 1.14  2002/07/23 16:29:55  dj_jl
//	Replaced console streams with output device class.
//	
//	Revision 1.13  2002/07/20 14:47:25  dj_jl
//	Changed function name in guard macros from pointer to static array.
//	
//	Revision 1.12  2002/05/29 16:52:42  dj_jl
//	Disabled another warnong.
//	
//	Revision 1.11  2002/05/18 16:56:34  dj_jl
//	Added FArchive and FOutputDevice classes.
//	
//	Revision 1.10  2002/04/11 16:40:06  dj_jl
//	Added __declspec modifiers.
//	
//	Revision 1.9  2002/03/28 18:05:51  dj_jl
//	Disabled some Borland warnings.
//	
//	Revision 1.8  2002/01/21 18:23:56  dj_jl
//	Disabled some MSVC warnings
//	
//	Revision 1.7  2002/01/07 12:16:41  dj_jl
//	Changed copyright year
//	
//	Revision 1.6  2002/01/03 18:38:25  dj_jl
//	Added guard macros and core dumps
//	
//	Revision 1.5  2001/12/12 19:27:08  dj_jl
//	Added some macros
//	
//	Revision 1.4  2001/10/08 17:26:17  dj_jl
//	Started to use exceptions
//	
//	Revision 1.3  2001/07/31 17:16:30  dj_jl
//	Just moved Log to the end of file
//	
//	Revision 1.2  2001/07/27 14:27:54  dj_jl
//	Update with Id-s and Log-s, some fixes
//
//**************************************************************************
