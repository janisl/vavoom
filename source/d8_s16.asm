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
 externdef _d_transluc
 externdef _d_srctranstab
 externdef _d_dsttranstab
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
 externdef lzistepx
 externdef gb
 externdef gbstep
 externdef full_cw
 externdef single_cw
 externdef floor_cw
 externdef ceil_cw
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
 externdef _roffs
 externdef _goffs
 externdef _boffs
 externdef _fadetable
 externdef _fadetable16
 externdef _fadetable16r
 externdef _fadetable16g
 externdef _fadetable16b
 externdef _fadetable32
 externdef _fadetable32r
 externdef _fadetable32g
 externdef _fadetable32b
 externdef _viewwidth
 externdef _viewheight
 externdef _vrectx_adj
 externdef _vrecty_adj
 externdef _vrectw_adj
 externdef _vrecth_adj
 externdef _r_nearzi
 externdef _r_emited
 externdef _d_u1
 externdef _d_v1
 externdef _d_ceilv1
 externdef _d_lastvertvalid
 externdef _firstvert
 externdef _edge_p
 externdef _surfaces
 externdef _surface_p
 externdef _newedges
 externdef _removeedges
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
 externdef _d_affinetridesc
 externdef _d_apverts
 externdef _d_anumverts
 externdef _aliastransform
 externdef _xprojection
 externdef _yprojection
 externdef _aliasxcenter
 externdef _aliasycenter
 externdef _ziscale
 externdef _d_plightvec
 externdef _d_avertexnormals
 externdef _d_ambientlightr
 externdef _d_ambientlightg
 externdef _d_ambientlightb
 externdef _d_shadelightr
 externdef _d_shadelightg
 externdef _d_shadelightb
 externdef _ubasestep
 externdef _errorterm
 externdef _erroradjustup
 externdef _erroradjustdown
 externdef _r_p0
 externdef _r_p1
 externdef _r_p2
 externdef _d_denom
 externdef _a_sstepxfrac
 externdef _a_tstepxfrac
 externdef _r_rstepx
 externdef _r_gstepx
 externdef _r_bstepx
 externdef _a_ststepxwhole
 externdef _r_sstepx
 externdef _r_tstepx
 externdef _r_rstepy
 externdef _r_gstepy
 externdef _r_bstepy
 externdef _r_sstepy
 externdef _r_tstepy
 externdef _r_zistepx
 externdef _r_zistepy
 externdef _d_aspancount
 externdef _d_countextrastep
 externdef _d_pedgespanpackage
 externdef _d_pdest
 externdef _d_ptex
 externdef _d_pz
 externdef _d_sfrac
 externdef _d_tfrac
 externdef _d_r
 externdef _d_g
 externdef _d_b
 externdef _d_zi
 externdef _d_ptexextrastep
 externdef _d_ptexbasestep
 externdef _d_pdestextrastep
 externdef _d_pdestbasestep
 externdef _d_sfracextrastep
 externdef _d_sfracbasestep
 externdef _d_tfracextrastep
 externdef _d_tfracbasestep
 externdef _d_rextrastep
 externdef _d_rbasestep
 externdef _d_gextrastep
 externdef _d_gbasestep
 externdef _d_bextrastep
 externdef _d_bbasestep
 externdef _d_ziextrastep
 externdef _d_zibasestep
 externdef _d_pzextrastep
 externdef _d_pzbasestep
 externdef _a_spans
 externdef _adivtab
 externdef _pr_globals
 externdef _pr_stackPtr
 externdef _pr_statements
 externdef _pr_functions
 externdef _pr_globaldefs
 externdef _pr_builtins
 externdef _current_func
 externdef _D_DrawZSpan
 externdef _PR_RFInvalidOpcode
_DATA SEGMENT
_DATA ENDS
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
 mov ebp,4096
 jmp LClampReentry2
LClampHigh2:
 mov ebp,ds:dword ptr[_bbextents]
 jmp LClampReentry2
LClampLow3:
 mov ecx,4096
 jmp LClampReentry3
LClampHigh3:
 mov ecx,ds:dword ptr[_bbextentt]
 jmp LClampReentry3
LClampLow4:
 mov eax,4096
 jmp LClampReentry4
LClampHigh4:
 mov eax,ds:dword ptr[_bbextents]
 jmp LClampReentry4
LClampLow5:
 mov ebx,4096
 jmp LClampReentry5
LClampHigh5:
 mov ebx,ds:dword ptr[_bbextentt]
 jmp LClampReentry5
 align 4
 public _D_DrawSpans16_8
_D_DrawSpans16_8:
 push ebp
 push edi
 push esi
 push ebx
 fld ds:dword ptr[_d_sdivzstepu]
 fmul ds:dword ptr[fp_16]
 mov edx,ds:dword ptr[_cacheblock]
 fld ds:dword ptr[_d_tdivzstepu]
 fmul ds:dword ptr[fp_16]
 mov ebx,ds:dword ptr[4+16+esp]
 fld ds:dword ptr[_d_zistepu]
 fmul ds:dword ptr[fp_16]
 mov ds:dword ptr[pbase],edx
 fstp ds:dword ptr[zi16stepu]
 fstp ds:dword ptr[tdivz16stepu]
 fstp ds:dword ptr[sdivz16stepu]
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
 mov ecx,ds:dword ptr[_scrn]
 mov eax,ds:dword ptr[4+ebx]
 mov ds:dword ptr[pspantemp],ebx
 mov edx,ds:dword ptr[_tadjust]
 mov esi,ds:dword ptr[_sadjust]
 mov edi,ds:dword ptr[_ylookup+eax*4]
 add edi,ecx
 mov ecx,ds:dword ptr[0+ebx]
 add edi,ecx
 mov ecx,ds:dword ptr[8+ebx]
 cmp ecx,16
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
 fadd ds:dword ptr[zi16stepu]
 fxch st(2)
 fadd ds:dword ptr[sdivz16stepu]
 fxch st(2)
 fld ds:dword ptr[tdivz16stepu]
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
 add esi,ebx
 add esi,eax
 cmp ecx,16
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
 mov bl,ds:byte ptr[esi]
 sub ecx,16
 mov ebp,ds:dword ptr[_sadjust]
 mov ds:dword ptr[counttemp],ecx
 mov ecx,ds:dword ptr[_tadjust]
 mov ds:byte ptr[edi],bl
 add ebp,eax
 add ecx,edx
 mov eax,ds:dword ptr[_bbextents]
 mov edx,ds:dword ptr[_bbextentt]
 cmp ebp,4096
 jl LClampLow2
 cmp ebp,eax
 ja LClampHigh2
LClampReentry2:
 cmp ecx,4096
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
 sar eax,20
 jz LZero
 sar edx,20
 mov ebx,ds:dword ptr[_cachewidth]
 imul eax,ebx
 jmp LSetUp1
LZero:
 sar edx,20
 mov ebx,ds:dword ptr[_cachewidth]
LSetUp1:
 add eax,edx
 mov edx,ds:dword ptr[tfracf]
 mov ds:dword ptr[advancetable+4],eax
 add eax,ebx
 shl ebp,12
 mov ebx,ds:dword ptr[sfracf]
 shl ecx,12
 mov ds:dword ptr[advancetable],eax
 mov ds:dword ptr[tstep],ecx
 add edx,ecx
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov al,ds:byte ptr[esi]
 add ebx,ebp
 mov ds:byte ptr[1+edi],al
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[2+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[3+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[4+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[5+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[6+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[7+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 mov ecx,ds:dword ptr[counttemp]
 cmp ecx,16
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
 fadd ds:dword ptr[zi16stepu]
 fxch st(2)
 fadd ds:dword ptr[sdivz16stepu]
 fxch st(2)
 fld ds:dword ptr[tdivz16stepu]
 faddp st(2),st(0)
 fld ds:dword ptr[fp_64k]
 fdiv st(0),st(1)
LFDIVInFlight2:
 mov ds:dword ptr[counttemp],ecx
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[8+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[9+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[10+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[11+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[12+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[13+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[14+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edi,16
 mov ds:dword ptr[tfracf],edx
 mov edx,ds:dword ptr[snext]
 mov ds:dword ptr[sfracf],ebx
 mov ebx,ds:dword ptr[tnext]
 mov ds:dword ptr[s],edx
 mov ds:dword ptr[t],ebx
 mov ecx,ds:dword ptr[counttemp]
 cmp ecx,16
 mov ds:byte ptr[-1+edi],al
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
 mov al,ds:byte ptr[esi]
 mov ebx,ds:dword ptr[_tadjust]
 mov ds:byte ptr[edi],al
 mov eax,ds:dword ptr[_sadjust]
 add eax,ds:dword ptr[snext]
 add ebx,ds:dword ptr[tnext]
 mov ebp,ds:dword ptr[_bbextents]
 mov edx,ds:dword ptr[_bbextentt]
 cmp eax,4096
 jl LClampLow4
 cmp eax,ebp
 ja LClampHigh4
LClampReentry4:
 mov ds:dword ptr[snext],eax
 cmp ebx,4096
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
 mov ebx,ds:dword ptr[entryvec_table_16+ecx*4]
 mov eax,edx
 mov ds:dword ptr[jumptemp],ebx
 mov ecx,ebp
 sar edx,16
 mov ebx,ds:dword ptr[_cachewidth]
 sar ecx,16
 imul edx,ebx
 add edx,ecx
 mov ecx,ds:dword ptr[tfracf]
 mov ds:dword ptr[advancetable+4],edx
 add edx,ebx
 shl ebp,16
 mov ebx,ds:dword ptr[sfracf]
 shl eax,16
 mov ds:dword ptr[advancetable],edx
 mov ds:dword ptr[tstep],eax
 mov edx,ecx
 add edx,eax
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 jmp dword ptr[jumptemp]
LNoSteps:
 mov al,ds:byte ptr[esi]
 sub edi,15
 jmp LEndSpan
LOnlyOneStep:
 sub eax,ds:dword ptr[s]
 sub ebx,ds:dword ptr[t]
 mov ebp,eax
 mov edx,ebx
 jmp LSetEntryvec
entryvec_table_16: dd 0, Entry2_16, Entry3_16, Entry4_16
 dd Entry5_16, Entry6_16, Entry7_16, Entry8_16
 dd Entry9_16, Entry10_16, Entry11_16, Entry12_16
 dd Entry13_16, Entry14_16, Entry15_16, Entry16_16
Entry2_16:
 sub edi,14
 mov al,ds:byte ptr[esi]
 jmp LEntry2_16
Entry3_16:
 sub edi,13
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 jmp LEntry3_16
Entry4_16:
 sub edi,12
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry4_16
Entry5_16:
 sub edi,11
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry5_16
Entry6_16:
 sub edi,10
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry6_16
Entry7_16:
 sub edi,9
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry7_16
Entry8_16:
 sub edi,8
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry8_16
Entry9_16:
 sub edi,7
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry9_16
Entry10_16:
 sub edi,6
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry10_16
Entry11_16:
 sub edi,5
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry11_16
Entry12_16:
 sub edi,4
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry12_16
Entry13_16:
 sub edi,3
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry13_16
Entry14_16:
 sub edi,2
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry14_16
Entry15_16:
 dec edi
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 jmp LEntry15_16
Entry16_16:
 add edx,eax
 mov al,ds:byte ptr[esi]
 sbb ecx,ecx
 add ebx,ebp
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
 sbb ecx,ecx
 mov ds:byte ptr[1+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry15_16:
 sbb ecx,ecx
 mov ds:byte ptr[2+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry14_16:
 sbb ecx,ecx
 mov ds:byte ptr[3+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry13_16:
 sbb ecx,ecx
 mov ds:byte ptr[4+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry12_16:
 sbb ecx,ecx
 mov ds:byte ptr[5+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry11_16:
 sbb ecx,ecx
 mov ds:byte ptr[6+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry10_16:
 sbb ecx,ecx
 mov ds:byte ptr[7+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry9_16:
 sbb ecx,ecx
 mov ds:byte ptr[8+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry8_16:
 sbb ecx,ecx
 mov ds:byte ptr[9+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry7_16:
 sbb ecx,ecx
 mov ds:byte ptr[10+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry6_16:
 sbb ecx,ecx
 mov ds:byte ptr[11+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry5_16:
 sbb ecx,ecx
 mov ds:byte ptr[12+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
 add edx,ds:dword ptr[tstep]
LEntry4_16:
 sbb ecx,ecx
 mov ds:byte ptr[13+edi],al
 add ebx,ebp
 mov al,ds:byte ptr[esi]
 adc esi,ds:dword ptr[advancetable+4+ecx*4]
LEntry3_16:
 mov ds:byte ptr[14+edi],al
 mov al,ds:byte ptr[esi]
LEntry2_16:
LEndSpan:
 fstp st(0)
 fstp st(0)
 fstp st(0)
 mov ebx,ds:dword ptr[pspantemp]
 mov ebx,ds:dword ptr[12+ebx]
 test ebx,ebx
 mov ds:byte ptr[15+edi],al
 jnz LSpanLoop
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
_TEXT ENDS
 END
