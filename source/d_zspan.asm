;**************************************************************************
;**
;**	   ##   ##    ##    ##   ##   ####     ####   ###     ###
;**	   ##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
;**	    ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
;**	    ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
;**	     ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
;**	      #    ##    ##    #      ####     ####   ##       ##
;**
;**	    Copyright (C) 1999-2001 JÆnis Legzdi·ý
;**
;**     This program is free software; you can redistribute it and/or
;**  modify it under the terms of the GNU General Public License
;**  as published by the Free Software Foundation; either version 2
;**  of the License, or (at your option) any later version.
;**
;**	    This program is distributed in the hope that it will be useful,
;**  but WITHOUT ANY WARRANTY; without even the implied warranty of
;**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;**  GNU General Public License for more details.
;**
;**************************************************************************

include asm_i386.inc

_TEXT SEGMENT

 align 4
LClamp:
 mov edx,040000000h
 xor ebx,ebx
 fstp st(0)
 jmp LZDraw
LClampNeg:
 mov edx,040000000h
 xor ebx,ebx
 fstp st(0)
 jmp LZDrawNeg
 align 4
 public _D_DrawZSpans
_D_DrawZSpans:
 push ebp
 push edi
 push esi
 push ebx
 fld ds:dword ptr[_d_zistepu]
 mov eax,ds:dword ptr[_d_zistepu]
 mov esi,ds:dword ptr[4+16+esp]
 test eax,eax
 jz LFNegSpan
 fmul ds:dword ptr[Float2ToThe31nd]
 fistp ds:dword ptr[izistep]
 mov ebx,ds:dword ptr[izistep]
LFSpanLoop:
 fild ds:dword ptr[4+esi]
 fild ds:dword ptr[0+esi]
 mov eax,ds:dword ptr[4+esi]
 mov edi,ds:dword ptr[_zbuffer]
 fmul ds:dword ptr[_d_zistepu]
 fxch st(1)
 fmul ds:dword ptr[_d_zistepv]
 fxch st(1)
 fadd ds:dword ptr[_d_ziorigin]
 mov eax,ds:dword ptr[_ylookup+eax*4]
 mov ecx,ds:dword ptr[8+esi]
 faddp st(1),st(0)
 fcom ds:dword ptr[float_point5]
 add eax,ds:dword ptr[0+esi]
 lea edi,ds:dword ptr[edi+eax*2]
 push esi
 fnstsw ax
 test ah,045h
 jz LClamp
 fmul ds:dword ptr[Float2ToThe31nd]
 fistp ds:dword ptr[izi]
 mov edx,ds:dword ptr[izi]
LZDraw:
 test edi,2
 jz LFMiddle
 mov eax,edx
 add edx,ebx
 shr eax,16
 dec ecx
 mov ds:word ptr[edi],ax
 add edi,2
LFMiddle:
 push ecx
 shr ecx,1
 jz LFLast
 shr ecx,1
 jnc LFMiddleLoop
 mov eax,edx
 add edx,ebx
 shr eax,16
 mov esi,edx
 add edx,ebx
 and esi,0FFFF0000h
 or eax,esi
 mov ds:dword ptr[edi],eax
 add edi,4
 and ecx,ecx
 jz LFLast
LFMiddleLoop:
 mov eax,edx
 add edx,ebx
 shr eax,16
 mov esi,edx
 add edx,ebx
 and esi,0FFFF0000h
 or eax,esi
 mov ebp,edx
 mov ds:dword ptr[edi],eax
 add edx,ebx
 shr ebp,16
 mov esi,edx
 add edx,ebx
 and esi,0FFFF0000h
 or ebp,esi
 mov ds:dword ptr[4+edi],ebp
 add edi,8
 dec ecx
 jnz LFMiddleLoop
LFLast:
 pop ecx
 and ecx,1
 jz LFSpanDone
 shr edx,16
 mov ds:word ptr[edi],dx
LFSpanDone:
 pop esi
 mov esi,ds:dword ptr[12+esi]
 test esi,esi
 jnz LFSpanLoop
 jmp LFDone
LFNegSpan:
 fmul ds:dword ptr[FloatMinus2ToThe31nd]
 fistp ds:dword ptr[izistep]
 mov ebx,ds:dword ptr[izistep]
LFNegSpanLoop:
 fild ds:dword ptr[4+esi]
 fild ds:dword ptr[0+esi]
 mov ecx,ds:dword ptr[4+esi]
 mov edi,ds:dword ptr[_zbuffer]
 fmul ds:dword ptr[_d_zistepu]
 fxch st(1)
 fmul ds:dword ptr[_d_zistepv]
 fxch st(1)
 fadd ds:dword ptr[_d_ziorigin]
 mov ecx,ds:dword ptr[_ylookup+ecx*4]
 faddp st(1),st(0)
 fcom ds:dword ptr[float_point5]
 lea edi,ds:dword ptr[edi+ecx*2]
 mov edx,ds:dword ptr[0+esi]
 add edx,edx
 mov ecx,ds:dword ptr[8+esi]
 add edi,edx
 push esi
 fnstsw ax
 test ah,045h
 jz LClampNeg
 fmul ds:dword ptr[Float2ToThe31nd]
 fistp ds:dword ptr[izi]
 mov edx,ds:dword ptr[izi]
LZDrawNeg:
 test edi,2
 jz LFNegMiddle
 mov eax,edx
 sub edx,ebx
 shr eax,16
 dec ecx
 mov ds:word ptr[edi],ax
 add edi,2
LFNegMiddle:
 push ecx
 shr ecx,1
 jz LFNegLast
 shr ecx,1
 jnc LFNegMiddleLoop
 mov eax,edx
 sub edx,ebx
 shr eax,16
 mov esi,edx
 sub edx,ebx
 and esi,0FFFF0000h
 or eax,esi
 mov ds:dword ptr[edi],eax
 add edi,4
 and ecx,ecx
 jz LFNegLast
LFNegMiddleLoop:
 mov eax,edx
 sub edx,ebx
 shr eax,16
 mov esi,edx
 sub edx,ebx
 and esi,0FFFF0000h
 or eax,esi
 mov ebp,edx
 mov ds:dword ptr[edi],eax
 sub edx,ebx
 shr ebp,16
 mov esi,edx
 sub edx,ebx
 and esi,0FFFF0000h
 or ebp,esi
 mov ds:dword ptr[4+edi],ebp
 add edi,8
 dec ecx
 jnz LFNegMiddleLoop
LFNegLast:
 pop ecx
 pop esi
 and ecx,1
 jz LFNegSpanDone
 shr edx,16
 mov ds:word ptr[edi],dx
LFNegSpanDone:
 mov esi,ds:dword ptr[12+esi]
 test esi,esi
 jnz LFNegSpanLoop
LFDone:
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
_TEXT ENDS
 END
