 .486P
 .model FLAT
 externdef _d_sdivzstepu
 externdef _d_tdivzstepu
 externdef _d_zistepu
 externdef _d_sdivzstepv
 externdef _d_tdivzstepv
 externdef _d_zistepv
 externdef _d_sdivzorigin
 externdef _d_tdivzorigin
 externdef _d_ziorigin
 externdef _sadjust
 externdef _tadjust
 externdef _bbextents
 externdef _bbextentt
 externdef _cacheblock
 externdef _cachewidth
 externdef _ds_transluc
 externdef _ds_transluc16
 externdef _r_lightptr
 externdef _r_lightptrr
 externdef _r_lightptrg
 externdef _r_lightptrb
 externdef _r_lightwidth
 externdef _r_numvblocks
 externdef _r_sourcemax
 externdef _r_stepback
 externdef _prowdestbase
 externdef _pbasesource
 externdef _sourcetstep
 externdef _surfrowbytes
 externdef _lightright
 externdef _lightrightstep
 externdef _lightdeltastep
 externdef _lightdelta
 externdef _lightrleft
 externdef _lightrright
 externdef _lightrleftstep
 externdef _lightrrightstep
 externdef _lightgleft
 externdef _lightgright
 externdef _lightgleftstep
 externdef _lightgrightstep
 externdef _lightbleft
 externdef _lightbright
 externdef _lightbleftstep
 externdef _lightbrightstep
 externdef sdivz8stepu
 externdef tdivz8stepu
 externdef zi8stepu
 externdef sdivz16stepu
 externdef tdivz16stepu
 externdef zi16stepu
 externdef s
 externdef t
 externdef snext
 externdef tnext
 externdef sstep
 externdef tstep
 externdef sfracf
 externdef tfracf
 externdef spancountminus1
 externdef izi
 externdef izistep
 externdef advancetable
 externdef pbase
 externdef pz
 externdef reciprocal_table
 externdef pspantemp
 externdef counttemp
 externdef jumptemp
 externdef mmbuf
 externdef fp_64k
 externdef fp_8
 externdef fp_16
 externdef Float2ToThe31nd
 externdef FloatMinus2ToThe31nd
 externdef fp_64kx64k
 externdef float_1
 externdef float_particle_z_clip
 externdef float_point5
 externdef DP_u
 externdef DP_v
 externdef DP_32768
 externdef DP_Color
 externdef DP_Pix
 externdef ceil_cw
 externdef single_cw
 externdef _ylookup
 externdef _zbuffer
 externdef _scrn
 externdef _scrn16
 externdef _pal8_to16
 externdef _mmx_mask4
 externdef _mmx_mask8
 externdef _mmx_mask16
 externdef _d_rowbytes
 externdef _d_zrowbytes
 externdef _vieworg
 externdef _r_ppn
 externdef _r_pup
 externdef _r_pright
 externdef _centerxfrac
 externdef _centeryfrac
 externdef _d_particle_right
 externdef _d_particle_top
 externdef _d_pix_min
 externdef _d_pix_max
 externdef _d_pix_shift
 externdef _d_y_aspect_shift
 externdef _d_rgbtable
 externdef _rshift
 externdef _gshift
 externdef _bshift
 externdef _fadetable
 externdef _fadetable16
 externdef _fadetable16r
 externdef _fadetable16g
 externdef _fadetable16b
 externdef _fadetable32
 externdef _pr_globals
 externdef _pr_stackPtr
 externdef _pr_statements
 externdef _pr_functions
 externdef _pr_globaldefs
 externdef _pr_builtins
 externdef _current_func
 externdef _D_DrawZSpan
 externdef _PR_RFInvalidOpcode
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
 public _D_DrawSpriteSpans_32
_D_DrawSpriteSpans_32:
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
 fld ds:dword ptr[_d_zistepu]
 fmul ds:dword ptr[fp_64kx64k]
 fxch st(3)
 fstp ds:dword ptr[sdivz8stepu]
 fstp ds:dword ptr[zi8stepu]
 fstp ds:dword ptr[tdivz8stepu]
 fistp ds:dword ptr[izistep]
 mov eax,ds:dword ptr[izistep]
 ror eax,16
 mov ecx,ds:dword ptr[8+ebx]
 mov ds:dword ptr[izistep],eax
 cmp ecx,0
 jle LNextSpan
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
 fld st(0)
 fmul ds:dword ptr[fp_64kx64k]
 fxch st(1)
 fdiv st(2),st(0)
 fxch st(1)
 fistp ds:dword ptr[izi]
 mov ebp,ds:dword ptr[izi]
 ror ebp,16
 mov eax,ds:dword ptr[4+ebx]
 mov ds:dword ptr[izi],ebp
 mov ebp,ds:dword ptr[0+ebx]
 mov eax,ds:dword ptr[_ylookup+eax*4]
 add eax,ebp
 shl eax,1
 add eax,ds:dword ptr[_zbuffer]
 mov ds:dword ptr[pz],eax
 mov ebp,ds:dword ptr[_scrn]
 mov eax,ds:dword ptr[4+ebx]
 push ebx
 mov edx,ds:dword ptr[_tadjust]
 mov esi,ds:dword ptr[_sadjust]
 mov edi,ds:dword ptr[_ylookup+eax*4]
 add edi,ds:dword ptr[0+ebx]
 shl edi,2
 add edi,ebp
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
 sal esi,2
 add esi,ebx
 imul eax,ds:dword ptr[_cachewidth]
 sal eax,2
 add esi,eax
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
 sub ecx,8
 mov ebp,ds:dword ptr[_sadjust]
 push ecx
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
 sar edx,19
 sal edx,2
 mov ebx,ds:dword ptr[_cachewidth]
 sal ebx,2
 sar eax,19
 jz LIsZero
 imul eax,ebx
LIsZero:
 add eax,edx
 mov edx,ds:dword ptr[tfracf]
 mov ds:dword ptr[advancetable+8],eax
 mov ds:dword ptr[advancetable+12],eax
 add eax,ebx
 shl ebp,13
 mov ds:dword ptr[sstep],ebp
 mov ebx,ds:dword ptr[sfracf]
 shl ecx,13
 mov ds:dword ptr[advancetable],eax
 mov ds:dword ptr[advancetable+4],eax
 mov ds:dword ptr[tstep],ecx
 add ds:dword ptr[advancetable+4],4
 add ds:dword ptr[advancetable+12],4
 mov ecx,ds:dword ptr[pz]
 mov ebp,ds:dword ptr[izi]
 cmp bp,ds:word ptr[ecx]
 jl Lp1
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp1
 mov ds:word ptr[ecx],bp
 mov ds:dword ptr[edi],eax
Lp1:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc eax,eax
 add esi,ds:dword ptr[advancetable+8+eax*4]
 cmp bp,ds:word ptr[2+ecx]
 jl Lp2
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp2
 mov ds:word ptr[2+ecx],bp
 mov ds:dword ptr[4+edi],eax
Lp2:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc eax,eax
 add esi,ds:dword ptr[advancetable+8+eax*4]
 cmp bp,ds:word ptr[4+ecx]
 jl Lp3
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp3
 mov ds:word ptr[4+ecx],bp
 mov ds:dword ptr[8+edi],eax
Lp3:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc eax,eax
 add esi,ds:dword ptr[advancetable+8+eax*4]
 cmp bp,ds:word ptr[6+ecx]
 jl Lp4
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp4
 mov ds:word ptr[6+ecx],bp
 mov ds:dword ptr[12+edi],eax
Lp4:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc eax,eax
 add esi,ds:dword ptr[advancetable+8+eax*4]
 cmp bp,ds:word ptr[8+ecx]
 jl Lp5
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp5
 mov ds:word ptr[8+ecx],bp
 mov ds:dword ptr[16+edi],eax
Lp5:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc eax,eax
 add esi,ds:dword ptr[advancetable+8+eax*4]
 pop eax
 cmp eax,8
 ja LSetupNotLast2
 dec eax
 jz LFDIVInFlight2
 mov ds:dword ptr[spancountminus1],eax
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
 push eax
 cmp bp,ds:word ptr[10+ecx]
 jl Lp6
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp6
 mov ds:word ptr[10+ecx],bp
 mov ds:dword ptr[20+edi],eax
Lp6:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc eax,eax
 add esi,ds:dword ptr[advancetable+8+eax*4]
 cmp bp,ds:word ptr[12+ecx]
 jl Lp7
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp7
 mov ds:word ptr[12+ecx],bp
 mov ds:dword ptr[24+edi],eax
Lp7:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc eax,eax
 add esi,ds:dword ptr[advancetable+8+eax*4]
 cmp bp,ds:word ptr[14+ecx]
 jl Lp8
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp8
 mov ds:word ptr[14+ecx],bp
 mov ds:dword ptr[28+edi],eax
Lp8:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc eax,eax
 add esi,ds:dword ptr[advancetable+8+eax*4]
 add edi,32
 add ecx,16
 mov ds:dword ptr[tfracf],edx
 mov edx,ds:dword ptr[snext]
 mov ds:dword ptr[sfracf],ebx
 mov ebx,ds:dword ptr[tnext]
 mov ds:dword ptr[s],edx
 mov ds:dword ptr[t],ebx
 mov ds:dword ptr[pz],ecx
 mov ds:dword ptr[izi],ebp
 pop ecx
 cmp ecx,8
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
 mov ebx,ds:dword ptr[_tadjust]
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
 mov ebx,ds:dword ptr[LEntryVecTable+ecx*4]
 mov eax,edx
 push ebx
 mov ecx,ebp
 sar ecx,16
 sal ecx,2
 mov ebx,ds:dword ptr[_cachewidth]
 sal ebx,2
 sar edx,16
 jz LIsZeroLast
 imul edx,ebx
LIsZeroLast:
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
 add ds:dword ptr[advancetable+4],4
 add ds:dword ptr[advancetable+12],4
 mov ds:dword ptr[tstep],eax
 mov ds:dword ptr[sstep],ebp
 mov edx,ecx
 mov ecx,ds:dword ptr[pz]
 mov ebp,ds:dword ptr[izi]
 ret
LNoSteps:
 mov ecx,ds:dword ptr[pz]
 sub edi,28
 sub ecx,14
 jmp LEndSpan
LOnlyOneStep:
 sub eax,ds:dword ptr[s]
 sub ebx,ds:dword ptr[t]
 mov ebp,eax
 mov edx,ebx
 jmp LSetEntryvec
LEntryVecTable:
 dd 0
 dd LEntry2
 dd LEntry3
 dd LEntry4
 dd LEntry5
 dd LEntry6
 dd LEntry7
 dd LEntry8
LEntry2:
 sub edi,24
 sub ecx,12
 mov eax,ds:dword ptr[esi]
 jmp LLEntry2
LEntry3:
 sub edi,20
 sub ecx,10
 jmp LLEntry3
LEntry4:
 sub edi,16
 sub ecx,8
 jmp LLEntry4
LEntry5:
 sub edi,12
 sub ecx,6
 jmp LLEntry5
LEntry6:
 sub edi,8
 sub ecx,4
 jmp LLEntry6
LEntry7:
 sub edi,4
 sub ecx,2
 jmp LLEntry7
LEntry8:
 cmp bp,ds:word ptr[ecx]
 jl Lp9
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp9
 mov ds:word ptr[ecx],bp
 mov ds:dword ptr[edi],eax
Lp9:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc eax,eax
 add esi,ds:dword ptr[advancetable+8+eax*4]
LLEntry7:
 cmp bp,ds:word ptr[2+ecx]
 jl Lp10
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp10
 mov ds:word ptr[2+ecx],bp
 mov ds:dword ptr[4+edi],eax
Lp10:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc eax,eax
 add esi,ds:dword ptr[advancetable+8+eax*4]
LLEntry6:
 cmp bp,ds:word ptr[4+ecx]
 jl Lp11
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp11
 mov ds:word ptr[4+ecx],bp
 mov ds:dword ptr[8+edi],eax
Lp11:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc eax,eax
 add esi,ds:dword ptr[advancetable+8+eax*4]
LLEntry5:
 cmp bp,ds:word ptr[6+ecx]
 jl Lp12
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp12
 mov ds:word ptr[6+ecx],bp
 mov ds:dword ptr[12+edi],eax
Lp12:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc eax,eax
 add esi,ds:dword ptr[advancetable+8+eax*4]
LLEntry4:
 cmp bp,ds:word ptr[8+ecx]
 jl Lp13
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp13
 mov ds:word ptr[8+ecx],bp
 mov ds:dword ptr[16+edi],eax
Lp13:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc eax,eax
 add esi,ds:dword ptr[advancetable+8+eax*4]
LLEntry3:
 cmp bp,ds:word ptr[10+ecx]
 jl Lp14
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp14
 mov ds:word ptr[10+ecx],bp
 mov ds:dword ptr[20+edi],eax
Lp14:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc eax,eax
 add esi,ds:dword ptr[advancetable+8+eax*4]
LLEntry2:
 cmp bp,ds:word ptr[12+ecx]
 jl Lp15
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp15
 mov ds:word ptr[12+ecx],bp
 mov ds:dword ptr[24+edi],eax
Lp15:
 add ebp,ds:dword ptr[izistep]
 adc ebp,0
 add edx,ds:dword ptr[tstep]
 sbb eax,eax
 add ebx,ds:dword ptr[sstep]
 adc eax,eax
 add esi,ds:dword ptr[advancetable+8+eax*4]
LEndSpan:
 cmp bp,ds:word ptr[14+ecx]
 jl Lp16
 mov eax,ds:dword ptr[esi]
 test eax,eax
 jz Lp16
 mov ds:word ptr[14+ecx],bp
 mov ds:dword ptr[28+edi],eax
Lp16:
 fstp st(0)
 fstp st(0)
 fstp st(0)
 pop ebx
LNextSpan:
 add ebx,12 
 mov ecx,ds:dword ptr[8+ebx]
 cmp ecx,0
 jg LSpanLoop
 jz LNextSpan
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
_TEXT ENDS
 END
