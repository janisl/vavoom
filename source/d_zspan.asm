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
