
//**************************************************************************
//**
//** common.h
//**
//**************************************************************************

#ifndef __COMMON_H__
#define __COMMON_H__

// HEADER FILES ------------------------------------------------------------

// MACROS ------------------------------------------------------------------

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef YES
#define YES 1
#endif
#ifndef NO
#define NO 0
#endif
#define MAX_IDENTIFIER_LENGTH 32
#define MAX_QUOTED_LENGTH 256
#define MAX_FILE_NAME_LENGTH 512
#define MAX_SCRIPT_COUNT 64
#define MAX_MAP_VARIABLES 32
#define MAX_SCRIPT_VARIABLES 10
#define MAX_WORLD_VARIABLES 64
#define MAX_STRINGS 128
#define DEFAULT_OBJECT_SIZE 65536
#define ASCII_SPACE 32
#define ASCII_QUOTE 34
#define ASCII_UNDERSCORE 95
#define EOF_CHARACTER 127
#if defined( __NeXT__) || defined(__unix__)
#define DIRECTORY_DELIMITER "/"
#define DIRECTORY_DELIMITER_CHAR ('/')
#else
#define DIRECTORY_DELIMITER "\\"
#define DIRECTORY_DELIMITER_CHAR ('\\')
#endif

// TYPES -------------------------------------------------------------------

typedef unsigned long	boolean;
typedef unsigned char	byte;
typedef signed char		S_BYTE;
typedef unsigned char	U_BYTE;
typedef signed short	S_WORD;
typedef unsigned short	U_WORD;
typedef signed long		S_LONG;
typedef unsigned long	U_LONG;

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

#endif
