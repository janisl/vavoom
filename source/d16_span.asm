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
LClampHigh0:
 mov esi,ds:dword ptr[_bbextents]
 jmp LClampReentry0
LClampHighOrLow0:
 jg LClampHigh0
 xor esi,esi
 jmp LClampReentry0
LClampHigh1:
 mov edx,ds:dword ptr[_bbextentt]
 jmp LClampReentry1
LClampHighOrLow1:
 jg LClampHigh1
 xor edx,edx
 jmp LClampReentry1
LClampLow2:
 mov ebp,2048
 jmp LClampReentry2
LClampHigh2:
 mov ebp,ds:dword ptr[_bbextents]
 jmp LClampReentry2
LClampLow3:
 mov ecx,2048
 jmp LClampReentry3
LClampHigh3:
 mov ecx,ds:dword ptr[_bbextentt]
 jmp LClampReentry3
LClampLow4:
 mov eax,2048
 jmp LClampReentry4
LClampHigh4:
 mov eax,ds:dword ptr[_bbextents]
 jmp LClampReentry4
LClampLow5:
 mov ebx,2048
 jmp LClampReentry5
LClampHigh5:
 mov ebx,ds:dword ptr[_bbextentt]
 jmp LClampReentry5
 align 4
 public _D_DrawSpans8_16
_D_DrawSpans8_16:
 push ebp
 push edi
 push esi
 push ebx
 fld ds:dword ptr[_d_sdivzstepu]
 fmul ds:dword ptr[fp_8]
 mov edx,ds:dword ptr[_cacheblock]
 fld ds:dword ptr[_d_tdivzstepu]
 fmul ds:dword ptr[fp_8]
 mov ebx,ds:dword ptr[4+16+esp]
 fld ds:dword ptr[_d_zistepu]
 fmul ds:dword ptr[fp_8]
 mov ds:dword ptr[pbase],edx
 fstp ds:dword ptr[zi8stepu]
 fstp ds:dword ptr[tdivz8stepu]
 fstp ds:dword ptr[sdivz8stepu]
LSpanLoop:
 fild ds:dword ptr[4+ebx]
 fild ds:dword ptr[0+ebx]
 fld st(1)
 fmul ds:dword ptr[_d_sdivzstepv]
 fld st(1)
 fmul ds:dword ptr[_d_sdivzstepu]
 fld st(2)
 fmul ds:dword ptr[_d_tdivzstepu]
 fxch st(1)
 faddp st(2),st(0)
 fxch st(1)
 fld st(3)
 fmul ds:dword ptr[_d_tdivzstepv]
 fxch st(1)
 fadd ds:dword ptr[_d_sdivzorigin]
 fxch st(4)
 fmul ds:dword ptr[_d_zistepv]
 fxch st(1)
 faddp st(2),st(0)
 fxch st(2)
 fmul ds:dword ptr[_d_zistepu]
 fxch st(1)
 fadd ds:dword ptr[_d_tdivzorigin]
 fxch st(2)
 faddp st(1),st(0)
 fld ds:dword ptr[fp_64k]
 fxch st(1)
 fadd ds:dword ptr[_d_ziorigin]
 fdiv st(1),st(0)
 mov edi,ds:dword ptr[_scrn]
 mov eax,ds:dword ptr[4+ebx]
 mov ds:dword ptr[pspantemp],ebx
 mov edx,ds:dword ptr[_tadjust]
 mov esi,ds:dword ptr[_sadjust]
 mov ecx,ds:dword ptr[_ylookup+eax*4]
 add ecx,ds:dword ptr[0+ebx]
 add ecx,ecx
 add edi,ecx
 mov ecx,ds:dword ptr[8+ebx]
 cmp ecx,8
 ja LSetupNotLast1
 dec ecx
 jz LCleanup1
 mov ds:dword ptr[spancountminus1],ecx
 fxch st(1)
 fld st(0)
 fmul st(0),st(4)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[s]
 fistp ds:dword ptr[t]
 fild ds:dword ptr[spancountminus1]
 fld ds:dword ptr[_d_tdivzstepu]
 fld ds:dword ptr[_d_zistepu]
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(2)
 fmul ds:dword ptr[_d_sdivzstepu]
 fxch st(1)
 faddp st(3),st(0)
 fxch st(1)
 faddp st(3),st(0)
 faddp st(3),st(0)
 fld ds:dword ptr[fp_64k]
 fdiv st(0),st(1)
 jmp LFDIVInFlight1
LCleanup1:
 fxch st(1)
 fld st(0)
 fmul st(0),st(4)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[s]
 fistp ds:dword ptr[t]
 jmp LFDIVInFlight1
 align 4
LSetupNotLast1:
 fxch st(1)
 fld st(0)
 fmul st(0),st(4)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[s]
 fistp ds:dword ptr[t]
 fadd ds:dword ptr[zi8stepu]
 fxch st(2)
 fadd ds:dword ptr[sdivz8stepu]
 fxch st(2)
 fld ds:dword ptr[tdivz8stepu]
 faddp st(2),st(0)
 fld ds:dword ptr[fp_64k]
 fdiv st(0),st(1)
LFDIVInFlight1:
 add esi,ds:dword ptr[s]
 add edx,ds:dword ptr[t]
 mov ebx,ds:dword ptr[_bbextents]
 mov ebp,ds:dword ptr[_bbextentt]
 cmp esi,ebx
 ja LClampHighOrLow0
LClampReentry0:
 mov ds:dword ptr[s],esi
 mov ebx,ds:dword ptr[pbase]
 shl esi,16
 cmp edx,ebp
 mov ds:dword ptr[sfracf],esi
 ja LClampHighOrLow1
LClampReentry1:
 mov ds:dword ptr[t],edx
 mov esi,ds:dword ptr[s]
 shl edx,16
 mov eax,ds:dword ptr[t]
 sar esi,16
 mov ds:dword ptr[tfracf],edx
 sar eax,16
 mov edx,ds:dword ptr[_cachewidth]
 imul eax,edx
 add esi,eax
 lea esi,ds:dword ptr[ebx+esi*2]
 cmp ecx,8
 jna LLastSegment
LNotLastSegment:
 fld st(0)
 fmul st(0),st(4)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[snext]
 fistp ds:dword ptr[tnext]
 mov eax,ds:dword ptr[snext]
 mov edx,ds:dword ptr[tnext]
 mov bx,ds:word ptr[esi]
 sub ecx,8
 mov ebp,ds:dword ptr[_sadjust]
 mov ds:dword ptr[counttemp],ecx
 mov ds:word ptr[edi],bx
 mov ecx,ds:dword ptr[_tadjust]
 add ebp,eax
 add ecx,edx
 mov eax,ds:dword ptr[_bbextents]
 mov edx,ds:dword ptr[_bbextentt]
 cmp ebp,2048
 jl LClampLow2
 cmp ebp,eax
 ja LClampHigh2
LClampReentry2:
 cmp ecx,2048
 jl LClampLow3
 cmp ecx,edx
 ja LClampHigh3
LClampReentry3:
 mov ds:dword ptr[snext],ebp
 mov ds:dword ptr[tnext],ecx
 sub ebp,ds:dword ptr[s]
 sub ecx,ds:dword ptr[t]
 mov eax,ecx
 mov edx,ebp
 sar eax,19
 jz LZero
 sar edx,19
 mov ebx,ds:dword ptr[_cachewidth]
 add ebx,ebx
 imul eax,ebx
 jmp LSetUp1
LZero:
 sar edx,19
 mov ebx,ds:dword ptr[_cachewidth]
 add ebx,ebx
LSetUp1:
 add edx,edx
 add eax,edx
 mov edx,ds:dword ptr[tfracf]
 mov ds:dword ptr[advancetable+8],eax
 mov ds:dword ptr[advancetable+12],eax
 add eax,ebx
 shl ebp,13
 mov ebx,ds:dword ptr[sfracf]
 shl ecx,13
 mov ds:dword ptr[advancetable],eax
 mov ds:dword ptr[advancetable+4],eax
 add ds:dword ptr[advancetable+4],2
 add ds:dword ptr[advancetable+12],2
 mov ds:dword ptr[tstep],ecx
 add edx,ecx
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov ds:byte ptr[2+edi],al
 mov ds:byte ptr[3+edi],ah
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov ds:byte ptr[4+edi],al
 mov ds:byte ptr[5+edi],ah
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov ds:byte ptr[6+edi],al
 mov ds:byte ptr[7+edi],ah
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 mov ecx,ds:dword ptr[counttemp]
 cmp ecx,8
 ja LSetupNotLast2
 dec ecx
 jz LFDIVInFlight2
 mov ds:dword ptr[spancountminus1],ecx
 fild ds:dword ptr[spancountminus1]
 fld ds:dword ptr[_d_zistepu]
 fmul st(0),st(1)
 fld ds:dword ptr[_d_tdivzstepu]
 fmul st(0),st(2)
 fxch st(1)
 faddp st(3),st(0)
 fxch st(1)
 fmul ds:dword ptr[_d_sdivzstepu]
 fxch st(1)
 faddp st(3),st(0)
 fld ds:dword ptr[fp_64k]
 fxch st(1)
 faddp st(4),st(0)
 fdiv st(0),st(1)
 jmp LFDIVInFlight2
 align 4
LSetupNotLast2:
 fadd ds:dword ptr[zi8stepu]
 fxch st(2)
 fadd ds:dword ptr[sdivz8stepu]
 fxch st(2)
 fld ds:dword ptr[tdivz8stepu]
 faddp st(2),st(0)
 fld ds:dword ptr[fp_64k]
 fdiv st(0),st(1)
LFDIVInFlight2:
 mov ds:dword ptr[counttemp],ecx
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov ds:byte ptr[8+edi],al
 mov ds:byte ptr[9+edi],ah
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov ds:byte ptr[10+edi],al
 mov ds:byte ptr[11+edi],ah
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov ds:byte ptr[12+edi],al
 mov ds:byte ptr[13+edi],ah
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edi,16
 mov ds:dword ptr[tfracf],edx
 mov edx,ds:dword ptr[snext]
 mov ds:dword ptr[sfracf],ebx
 mov ebx,ds:dword ptr[tnext]
 mov ds:dword ptr[s],edx
 mov ds:dword ptr[t],ebx
 mov ecx,ds:dword ptr[counttemp]
 cmp ecx,8
 mov ds:byte ptr[-2+edi],al
 mov ds:byte ptr[-1+edi],ah
 ja LNotLastSegment
LLastSegment:
 test ecx,ecx
 jz LNoSteps
 fld st(0)
 fmul st(0),st(4)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(1)
 fistp ds:dword ptr[snext]
 fistp ds:dword ptr[tnext]
 mov ax,ds:word ptr[esi]
 mov ebx,ds:dword ptr[_tadjust]
 mov ds:word ptr[edi],ax
 mov eax,ds:dword ptr[_sadjust]
 add eax,ds:dword ptr[snext]
 add ebx,ds:dword ptr[tnext]
 mov ebp,ds:dword ptr[_bbextents]
 mov edx,ds:dword ptr[_bbextentt]
 cmp eax,2048
 jl LClampLow4
 cmp eax,ebp
 ja LClampHigh4
LClampReentry4:
 mov ds:dword ptr[snext],eax
 cmp ebx,2048
 jl LClampLow5
 cmp ebx,edx
 ja LClampHigh5
LClampReentry5:
 cmp ecx,1
 je LOnlyOneStep
 sub eax,ds:dword ptr[s]
 sub ebx,ds:dword ptr[t]
 add eax,eax
 add ebx,ebx
 imul ds:dword ptr[reciprocal_table-8+ecx*4]
 mov ebp,edx
 mov eax,ebx
 imul ds:dword ptr[reciprocal_table-8+ecx*4]
LSetEntryvec:
 mov ebx,ds:dword ptr[Lentryvec_table+ecx*4]
 mov eax,edx
 mov ds:dword ptr[jumptemp],ebx
 mov ecx,ebp
 sar edx,16
 mov ebx,ds:dword ptr[_cachewidth]
 sar ecx,16
 sal ecx,1
 add ebx,ebx
 imul edx,ebx
 add edx,ecx
 mov ecx,ds:dword ptr[tfracf]
 mov ds:dword ptr[advancetable+8],edx
 mov ds:dword ptr[advancetable+12],edx
 add edx,ebx
 shl ebp,16
 mov ebx,ds:dword ptr[sfracf]
 shl eax,16
 mov ds:dword ptr[advancetable],edx
 mov ds:dword ptr[advancetable+4],edx
 add ds:dword ptr[advancetable+4],2
 add ds:dword ptr[advancetable+12],2
 mov ds:dword ptr[tstep],eax
 mov edx,ecx
 add edx,eax
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 jmp dword ptr[jumptemp]
 align 4
Lentryvec_table:
 dd 0
 dd LEntry2_8
 dd LEntry3_8
 dd LEntry4_8
 dd LEntry5_8
 dd LEntry6_8
 dd LEntry7_8
 dd LEntry8_8
LNoSteps:
 mov ax,ds:word ptr[esi]
 sub edi,14
 jmp LEndSpan
LOnlyOneStep:
 sub eax,ds:dword ptr[s]
 sub ebx,ds:dword ptr[t]
 mov ebp,eax
 mov edx,ebx
 jmp LSetEntryvec
LEntry2_8:
 sub edi,12
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 jmp LLEntry2_8
LEntry3_8:
 sub edi,10
 add edx,eax
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 jmp LLEntry3_8
LEntry4_8:
 sub edi,8
 add edx,eax
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LLEntry4_8
LEntry5_8:
 sub edi,6
 add edx,eax
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LLEntry5_8
LEntry6_8:
 sub edi,4
 add edx,eax
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LLEntry6_8
LEntry7_8:
 sub edi,2
 add edx,eax
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LLEntry7_8
LEntry8_8:
 add edx,eax
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov ds:byte ptr[2+edi],al
 mov ds:byte ptr[3+edi],ah
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edx,ds:dword ptr[tstep]
LLEntry7_8:
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov ds:byte ptr[4+edi],al
 mov ds:byte ptr[5+edi],ah
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edx,ds:dword ptr[tstep]
LLEntry6_8:
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov ds:byte ptr[6+edi],al
 mov ds:byte ptr[7+edi],ah
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edx,ds:dword ptr[tstep]
LLEntry5_8:
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov ds:byte ptr[8+edi],al
 mov ds:byte ptr[9+edi],ah
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
 add edx,ds:dword ptr[tstep]
LLEntry4_8:
 sbb ecx,ecx
 add ebx,ebp
 adc ecx,ecx
 mov ds:byte ptr[10+edi],al
 mov ds:byte ptr[11+edi],ah
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
 add esi,ds:dword ptr[advancetable+8+ecx*4]
LLEntry3_8:
 mov ds:byte ptr[12+edi],al
 mov ds:byte ptr[13+edi],ah
 mov al,ds:byte ptr[esi]
 mov ah,ds:byte ptr[1+esi]
LLEntry2_8:
LEndSpan:
 fstp st(0)
 fstp st(0)
 fstp st(0)
 mov ebx,ds:dword ptr[pspantemp]
 mov ebx,ds:dword ptr[12+ebx]
 test ebx,ebx
 mov ds:byte ptr[14+edi],al
 mov ds:byte ptr[15+edi],ah
 jnz LSpanLoop
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
_TEXT ENDS
 END
