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
 public _D_DrawParticle_32
_D_DrawParticle_32:
 push ebp
 push edi
 push ebx
 mov edi,ds:dword ptr[12+4+esp]
L15bppEntry:
 fld ds:dword ptr[_vieworg]
 fsubr ds:dword ptr[0+edi]
 fld ds:dword ptr[0+4+edi]
 fsub ds:dword ptr[_vieworg+4]
 fld ds:dword ptr[0+8+edi]
 fsub ds:dword ptr[_vieworg+8]
 fxch st(2)
 fld ds:dword ptr[_r_ppn]
 fmul st(0),st(1)
 fld ds:dword ptr[_r_ppn+4]
 fmul st(0),st(3)
 fld ds:dword ptr[_r_ppn+8]
 fmul st(0),st(5)
 fxch st(2)
 faddp st(1),st(0)
 faddp st(1),st(0)
 fld st(0)
 fdivr ds:dword ptr[float_1]
 fxch st(1)
 fcomp ds:dword ptr[float_particle_z_clip]
 fxch st(3)
 fld ds:dword ptr[_r_pup]
 fmul st(0),st(2)
 fld ds:dword ptr[_r_pup+4]
 fnstsw ax
 test ah,1
 jnz LPop6AndDone
 fmul st(0),st(4)
 fld ds:dword ptr[_r_pup+8]
 fmul st(0),st(3)
 fxch st(2)
 faddp st(1),st(0)
 faddp st(1),st(0)
 fxch st(3)
 fmul ds:dword ptr[_r_pright+4]
 fxch st(2)
 fmul ds:dword ptr[_r_pright]
 fxch st(1)
 fmul ds:dword ptr[_r_pright+8]
 fxch st(2)
 faddp st(1),st(0)
 faddp st(1),st(0)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(1)
 fadd ds:dword ptr[_centeryfrac]
 fxch st(1)
 fadd ds:dword ptr[_centerxfrac]
 fxch st(1)
 fadd ds:dword ptr[float_point5]
 fxch st(1)
 fadd ds:dword ptr[float_point5]
 fxch st(2)
 fmul ds:dword ptr[DP_32768]
 fxch st(2)
 fistp ds:dword ptr[DP_u]
 fistp ds:dword ptr[DP_v]
 mov eax,ds:dword ptr[DP_u]
 mov edx,ds:dword ptr[DP_v]
 cmp edx,0
 jl LPop1AndDone
 cmp eax,0
 jl LPop1AndDone
 mov ebx,ds:dword ptr[_d_particle_top]
 mov ecx,ds:dword ptr[_d_particle_right]
 cmp edx,ebx
 jg LPop1AndDone
 cmp eax,ecx
 jg LPop1AndDone
 mov ebp,ds:dword ptr[12+edi]
 mov ecx,ds:dword ptr[_rshift]
 cmp ecx,16
 je LColorDone
 bswap ebp
 ror ebp,8
LColorDone:
 mov ds:dword ptr[DP_Color],ebp
 mov ebx,ds:dword ptr[_scrn]
 mov edi,ds:dword ptr[_ylookup+edx*4]
 add edi,eax
 mov edx,edi
 mov eax,ds:dword ptr[_zbuffer]
 fistp ds:dword ptr[izi]
 lea edi,ds:dword ptr[ebx+edi*4]
 lea edx,ds:dword ptr[eax+edx*2]
 mov eax,ds:dword ptr[izi]
 mov ecx,ds:dword ptr[_d_pix_shift]
 shr eax,cl
 mov ebp,ds:dword ptr[izi]
 mov ebx,ds:dword ptr[_d_pix_min]
 mov ecx,ds:dword ptr[_d_pix_max]
 cmp eax,ebx
 jnl LTestPixMax
 mov eax,ebx
 jmp LTestDone
LTestPixMax:
 cmp eax,ecx
 jng LTestDone
 mov eax,ecx
LTestDone:
 mov ebx,ds:dword ptr[_d_y_aspect_shift]
 test ebx,ebx
 jnz LDefault
 cmp eax,4
 ja LDefault
 mov ecx,ds:dword ptr[DP_Color]
 jmp dword ptr[DP_EntryTable-4+eax*4]
 align 4
DP_EntryTable:
 dd LDP_1x1
 dd LDP_2x2
 dd LDP_3x3
 dd LDP_4x4
 align 4
LDP_1x1:
 cmp ds:word ptr[edx],bp
 jg LDone
 mov ds:word ptr[edx],bp
 mov ds:dword ptr[edi],ecx
 jmp LDone
 align 4
LDP_2x2:
 push esi
 mov ebx,ds:dword ptr[_d_rowbytes]
 mov esi,ds:dword ptr[_d_zrowbytes]
 cmp ds:word ptr[edx],bp
 jg L2x2_1
 mov ds:word ptr[edx],bp
 mov ds:dword ptr[edi],ecx
L2x2_1:
 cmp ds:word ptr[2+edx],bp
 jg L2x2_2
 mov ds:word ptr[2+edx],bp
 mov ds:dword ptr[4+edi],ecx
L2x2_2:
 cmp ds:word ptr[edx+esi*1],bp
 jg L2x2_3
 mov ds:word ptr[edx+esi*1],bp
 mov ds:dword ptr[edi+ebx*1],ecx
L2x2_3:
 cmp ds:word ptr[2+edx+esi*1],bp
 jg L2x2_4
 mov ds:word ptr[2+edx+esi*1],bp
 mov ds:dword ptr[4+edi+ebx*1],ecx
L2x2_4:
 pop esi
 jmp LDone
 align 4
LDP_3x3:
 push esi
 mov ebx,ds:dword ptr[_d_rowbytes]
 mov esi,ds:dword ptr[_d_zrowbytes]
 cmp ds:word ptr[edx],bp
 jg L3x3_1
 mov ds:word ptr[edx],bp
 mov ds:dword ptr[edi],ecx
L3x3_1:
 cmp ds:word ptr[2+edx],bp
 jg L3x3_2
 mov ds:word ptr[2+edx],bp
 mov ds:dword ptr[4+edi],ecx
L3x3_2:
 cmp ds:word ptr[4+edx],bp
 jg L3x3_3
 mov ds:word ptr[4+edx],bp
 mov ds:dword ptr[8+edi],ecx
L3x3_3:
 cmp ds:word ptr[edx+esi*1],bp
 jg L3x3_4
 mov ds:word ptr[edx+esi*1],bp
 mov ds:dword ptr[edi+ebx*1],ecx
L3x3_4:
 cmp ds:word ptr[2+edx+esi*1],bp
 jg L3x3_5
 mov ds:word ptr[2+edx+esi*1],bp
 mov ds:dword ptr[4+edi+ebx*1],ecx
L3x3_5:
 cmp ds:word ptr[4+edx+esi*1],bp
 jg L3x3_6
 mov ds:word ptr[4+edx+esi*1],bp
 mov ds:dword ptr[8+edi+ebx*1],ecx
L3x3_6:
 cmp ds:word ptr[edx+esi*2],bp
 jg L3x3_7
 mov ds:word ptr[edx+esi*2],bp
 mov ds:dword ptr[edi+ebx*2],ecx
L3x3_7:
 cmp ds:word ptr[2+edx+esi*2],bp
 jg L3x3_8
 mov ds:word ptr[2+edx+esi*2],bp
 mov ds:dword ptr[4+edi+ebx*2],ecx
L3x3_8:
 cmp ds:word ptr[4+edx+esi*2],bp
 jg L3x3_9
 mov ds:word ptr[4+edx+esi*2],bp
 mov ds:dword ptr[8+edi+ebx*2],ecx
L3x3_9:
 pop esi
 jmp LDone
 align 4
LDP_4x4:
 push esi
 mov ebx,ds:dword ptr[_d_rowbytes]
 mov esi,ds:dword ptr[_d_zrowbytes]
 cmp ds:word ptr[edx],bp
 jg L4x4_1
 mov ds:word ptr[edx],bp
 mov ds:dword ptr[edi],ecx
L4x4_1:
 cmp ds:word ptr[2+edx],bp
 jg L4x4_2
 mov ds:word ptr[2+edx],bp
 mov ds:dword ptr[4+edi],ecx
L4x4_2:
 cmp ds:word ptr[4+edx],bp
 jg L4x4_3
 mov ds:word ptr[4+edx],bp
 mov ds:dword ptr[8+edi],ecx
L4x4_3:
 cmp ds:word ptr[6+edx],bp
 jg L4x4_4
 mov ds:word ptr[6+edx],bp
 mov ds:dword ptr[12+edi],ecx
L4x4_4:
 cmp ds:word ptr[edx+esi*1],bp
 jg L4x4_5
 mov ds:word ptr[edx+esi*1],bp
 mov ds:dword ptr[edi+ebx*1],ecx
L4x4_5:
 cmp ds:word ptr[2+edx+esi*1],bp
 jg L4x4_6
 mov ds:word ptr[2+edx+esi*1],bp
 mov ds:dword ptr[4+edi+ebx*1],ecx
L4x4_6:
 cmp ds:word ptr[4+edx+esi*1],bp
 jg L4x4_7
 mov ds:word ptr[4+edx+esi*1],bp
 mov ds:dword ptr[8+edi+ebx*1],ecx
L4x4_7:
 cmp ds:word ptr[6+edx+esi*1],bp
 jg L4x4_8
 mov ds:word ptr[6+edx+esi*1],bp
 mov ds:dword ptr[12+edi+ebx*1],ecx
L4x4_8:
 lea edx,ds:dword ptr[edx+esi*2]
 lea edi,ds:dword ptr[edi+ebx*2]
 cmp ds:word ptr[edx],bp
 jg L4x4_9
 mov ds:word ptr[edx],bp
 mov ds:dword ptr[edi],ecx
L4x4_9:
 cmp ds:word ptr[2+edx],bp
 jg L4x4_10
 mov ds:word ptr[2+edx],bp
 mov ds:dword ptr[4+edi],ecx
L4x4_10:
 cmp ds:word ptr[4+edx],bp
 jg L4x4_11
 mov ds:word ptr[4+edx],bp
 mov ds:dword ptr[8+edi],ecx
L4x4_11:
 cmp ds:word ptr[6+edx],bp
 jg L4x4_12
 mov ds:word ptr[6+edx],bp
 mov ds:dword ptr[12+edi],ecx
L4x4_12:
 cmp ds:word ptr[edx+esi*1],bp
 jg L4x4_13
 mov ds:word ptr[edx+esi*1],bp
 mov ds:dword ptr[edi+ebx*1],ecx
L4x4_13:
 cmp ds:word ptr[2+edx+esi*1],bp
 jg L4x4_14
 mov ds:word ptr[2+edx+esi*1],bp
 mov ds:dword ptr[4+edi+ebx*1],ecx
L4x4_14:
 cmp ds:word ptr[4+edx+esi*1],bp
 jg L4x4_15
 mov ds:word ptr[4+edx+esi*1],bp
 mov ds:dword ptr[8+edi+ebx*1],ecx
L4x4_15:
 cmp ds:word ptr[6+edx+esi*1],bp
 jg L4x4_16
 mov ds:word ptr[6+edx+esi*1],bp
 mov ds:dword ptr[12+edi+ebx*1],ecx
L4x4_16:
 pop esi
 jmp LDone
 align 4
LDefault:
 mov ebx,eax
 mov ds:dword ptr[DP_Pix],eax
 mov cl,ds:byte ptr[_d_y_aspect_shift]
 shl ebx,cl
 mov ecx,ds:dword ptr[DP_Color]
LGenRowLoop:
 mov eax,ds:dword ptr[DP_Pix]
LGenColLoop:
 cmp ds:word ptr[-2+edx+eax*2],bp
 jg LGSkip
 mov ds:word ptr[-2+edx+eax*2],bp
 mov ds:dword ptr[-4+edi+eax*4],ecx
LGSkip:
 dec eax
 jnz LGenColLoop
 add edx,ds:dword ptr[_d_zrowbytes]
 add edi,ds:dword ptr[_d_rowbytes]
 dec ebx
 jnz LGenRowLoop
LDone:
 pop ebx
 pop edi
 pop ebp
 ret
LPop6AndDone:
 fstp st(0)
 fstp st(0)
 fstp st(0)
 fstp st(0)
 fstp st(0)
LPop1AndDone:
 fstp st(0)
 jmp LDone
_TEXT ENDS
 END
