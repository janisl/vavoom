//----------------------------------------------------------------------------
//  EDGE Cyclic Rendundancy Checks
//----------------------------------------------------------------------------
// 
//  Copyright (c) 1999-2001  The EDGE Team.
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//----------------------------------------------------------------------------
//
//  Based on the Adler-32 algorithm as described in RFC-1950.
//
//----------------------------------------------------------------------------

#include "i_defs.h"
#include "m_math.h"

#include <math.h>


//
//  CORE ROUTINES
//

void CRC32_Init(unsigned long *crc)
{
  (*crc) = 1;
}

void CRC32_ProcessByte(unsigned long *crc, byte data)
{
  unsigned long s1 = (*crc) & 0xFFFF;
  unsigned long s2 = ((*crc) >> 16) & 0xFFFF;

  s1 = (s1 + data) % 65521;
  s2 = (s2 + s1)   % 65521;
  
  (*crc) = (s2 << 16) | s1;
}

void CRC32_ProcessBlock(unsigned long *crc, const byte *data, int len)
{
  unsigned long s1 = (*crc) & 0xFFFF;
  unsigned long s2 = ((*crc) >> 16) & 0xFFFF;

  for (; len > 0; data++, len--)
  {
    s1 = (s1 + data[0]) % 65521;
    s2 = (s2 + s1)      % 65521;
  }

  (*crc) = (s2 << 16) | s1;
}

void CRC32_Done(unsigned long *crc)
{
  // nothing to do
}


//
//  UTILITY ROUTINES
//

void CRC32_ProcessInt(unsigned long *crc, int value)
{
  CRC32_ProcessByte(crc, (byte) (value >> 24));
  CRC32_ProcessByte(crc, (byte) (value >> 16));
  CRC32_ProcessByte(crc, (byte) (value >> 8));
  CRC32_ProcessByte(crc, (byte) (value));
}

void CRC32_ProcessFixed(unsigned long *crc, fixed_t value)
{
  CRC32_ProcessInt(crc, (int) value);
}

void CRC32_ProcessFloat(unsigned long *crc, float_t value)
{
  int exp;
  int mant;
  boolean_t neg;

  neg = (value < 0.0);
  value = fabs(value);

  mant = (int) ldexp(frexp(value, &exp), 30);

  CRC32_ProcessByte(crc, (byte) (neg ? '-' : '+'));
  CRC32_ProcessInt(crc, exp);
  CRC32_ProcessInt(crc, mant);
}

void CRC32_ProcessStr(unsigned long *crc, const char *str)
{
  for (; str[0]; str++)
    CRC32_ProcessByte(crc, (byte) str[0]);
}

