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
 externdef _d_skysmask
 externdef _d_skytmask
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
 externdef _view_clipplanes
 externdef _viewforward
 externdef _viewright
 externdef _viewup
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
 externdef _edge_head
 externdef _edge_tail
 externdef _surfaces
 externdef _surface_p
 externdef _newedges
 externdef _removeedges
 externdef _span_p
 externdef _current_iv
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
 externdef _pr_strings
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
Lcoords dd 0, 0, 0
Ltemp dd 0
_DATA ENDS
_TEXT SEGMENT
 public _D_AliasTransformFinalVert
_D_AliasTransformFinalVert:
 push ebx
 mov edx,dword ptr[16+esp]
 mov ebx,dword ptr[8+esp]
 mov ecx,dword ptr[12+esp]
 mov al,byte ptr[edx]
 mov byte ptr[Lcoords],al
 mov al,byte ptr[1+edx]
 mov byte ptr[Lcoords+4],al
 mov al,byte ptr[2+edx]
 mov byte ptr[Lcoords+8],al
 xor eax,eax
 mov al,byte ptr[3+edx]
 lea eax,dword ptr[eax+eax*2]
 lea eax,dword ptr[_d_avertexnormals+eax*4]
 fild dword ptr[Lcoords]
 fild dword ptr[Lcoords+4]
 fild dword ptr[Lcoords+8]
 fld st(2)
 fmul dword ptr[_aliastransform+0]
 fld st(2)
 fmul dword ptr[_aliastransform+4]
 fxch st(1)
 fadd dword ptr[_aliastransform+12]
 fld st(2)
 fmul dword ptr[_aliastransform+8]
 fxch st(1)
 faddp st(2),st(0)
 fld st(4)
 fmul dword ptr[_aliastransform+16]
 fld st(4)
 fmul dword ptr[_aliastransform+20]
 fxch st(1)
 fadd dword ptr[_aliastransform+28]
 fld st(4)
 fmul dword ptr[_aliastransform+24]
 fxch st(1)
 faddp st(2),st(0)
 fxch st(6)
 fmul dword ptr[_aliastransform+32]
 fxch st(5)
 fmul dword ptr[_aliastransform+36]
 fxch st(4)
 fmul dword ptr[_aliastransform+40]
 fxch st(5)
 fadd dword ptr[_aliastransform+44]
 fxch st(2)
 faddp st(3),st(0)
 fxch st(3)
 faddp st(4),st(0)
 fxch st(2)
 faddp st(4),st(0)
 fstp dword ptr[ecx]
 faddp st(1),st(0)
 fld dword ptr[eax]
 fmul dword ptr[_d_plightvec]
 fld dword ptr[4+eax]
 fmul dword ptr[_d_plightvec+4]
 fld dword ptr[8+eax]
 fmul dword ptr[_d_plightvec+8]
 fxch st(2)
 faddp st(1),st(0)
 fxch st(3)
 fstp dword ptr[4+ecx]
 faddp st(2),st(0)
 fstp dword ptr[8+ecx]
 mov edx,dword ptr[_d_ambientlightr]
 mov eax,dword ptr[_d_ambientlightg]
 mov dword ptr[16+ebx],eax
 mov eax,dword ptr[_d_ambientlightb]
 mov dword ptr[12+ebx],edx
 mov dword ptr[20+ebx],eax
 fst dword ptr[Ltemp]
 mov dword ptr[24+ebx],0
 mov al,byte ptr[Ltemp+3]
 test al,080h
 jz LTFVPopAndDone
 fld dword ptr[_d_shadelightr]
 fmul st(0),st(1)
 fld dword ptr[_d_shadelightg]
 fmul st(0),st(2)
 fld dword ptr[_d_shadelightb]
 fmulp st(3),st(0)
 fxch st(1)
 fistp dword ptr[Ltemp]
 mov eax,dword ptr[Ltemp]
 add eax,edx
 mov dword ptr[12+ebx],eax
 jns LTFVSkip1
 mov dword ptr[12+ebx],0
LTFVSkip1:
 fistp dword ptr[Ltemp]
 mov eax,dword ptr[Ltemp]
 add eax,dword ptr[16+ebx]
 mov dword ptr[16+ebx],eax
 jns LTFVSkip2
 mov dword ptr[16+ebx],0
LTFVSkip2:
 fistp dword ptr[Ltemp]
 mov eax,dword ptr[Ltemp]
 add eax,dword ptr[20+ebx]
 mov dword ptr[20+ebx],eax
 jns LTFVDone
 mov dword ptr[20+ebx],0
 jmp LTFVDone
LTFVPopAndDone:
 fstp st(0)
LTFVDone:
 pop ebx
 ret
 public _D_AliasProjectFinalVert
_D_AliasProjectFinalVert:
 mov ecx,dword ptr[4+esp]
 mov edx,dword ptr[8+esp]
 fld1
 fld dword ptr[8+edx]
 fdivp st(1),st(0)
 fld dword ptr[edx]
 fmul dword ptr[_xprojection]
 fld dword ptr[4+edx]
 fmul dword ptr[_yprojection]
 fld dword ptr[_ziscale]
 fxch st(3)
 fmul st(2),st(0)
 fmul st(1),st(0)
 fmulp st(3),st(0)
 fxch st(1)
 fadd dword ptr[_aliasxcenter]
 fxch st(1)
 fadd dword ptr[_aliasycenter]
 fxch st(2)
 fistp dword ptr[8+ecx]
 fistp dword ptr[ecx]
 fistp dword ptr[4+ecx]
 ret
 public _D_AliasTransformAndProjectFinalVerts
_D_AliasTransformAndProjectFinalVerts:
 push ebp
 push edi
 push esi
 push ebx
 mov ebp,dword ptr[_d_anumverts]
 mov edi,dword ptr[_d_apverts]
 mov esi,dword ptr[20+esp]
LTLoop:
 xor eax,eax
 mov al,byte ptr[edi]
 mov byte ptr[Lcoords],al
 mov al,byte ptr[1+edi]
 mov byte ptr[Lcoords+4],al
 mov al,byte ptr[2+edi]
 mov byte ptr[Lcoords+8],al
 mov al,byte ptr[3+edi]
 fild dword ptr[Lcoords]
 fild dword ptr[Lcoords+4]
 lea eax,dword ptr[eax+eax*2]
 fild dword ptr[Lcoords+8]
 sal eax,2
 fld st(2)
 fmul dword ptr[_aliastransform+32]
 fld st(2)
 fmul dword ptr[_aliastransform+36]
 fxch st(1)
 fadd dword ptr[_aliastransform+44]
 fld dword ptr[_d_avertexnormals+0+eax]
 fmul dword ptr[_d_plightvec+0]
 fld dword ptr[_d_avertexnormals+4+eax]
 fmul dword ptr[_d_plightvec+4]
 fld dword ptr[_d_avertexnormals+8+eax]
 fmul dword ptr[_d_plightvec+8]
 fxch st(2)
 faddp st(1),st(0)
 fxch st(2)
 faddp st(3),st(0)
 fld st(3)
 fmul dword ptr[_aliastransform+40]
 fxch st(2)
 faddp st(1),st(0)
 fld1
 fxch st(2)
 faddp st(3),st(0)
 fst dword ptr[Ltemp]
 fxch st(2)
 fdivp st(1),st(0)
 mov ecx,dword ptr[_d_ambientlightr]
 mov edx,dword ptr[_d_ambientlightg]
 mov ebx,dword ptr[_d_ambientlightb]
 mov al,byte ptr[Ltemp+3]
 test al,080h
 jz Lp4
 fld dword ptr[_d_shadelightr]
 fmul st(0),st(2)
 fld dword ptr[_d_shadelightg]
 fmul st(0),st(3)
 fld dword ptr[_d_shadelightb]
 fmulp st(4),st(0)
 fxch st(1)
 fistp dword ptr[Ltemp]
 mov eax,dword ptr[Ltemp]
 add ecx,eax
 jns Lp1
 xor ecx,ecx
Lp1:
 fistp dword ptr[Ltemp]
 mov eax,dword ptr[Ltemp]
 add edx,eax
 jns Lp2
 xor edx,edx
Lp2:
 fxch st(1)
 fistp dword ptr[Ltemp]
 mov eax,dword ptr[Ltemp]
 add ebx,eax
 jns Lp3
 xor ebx,ebx
 jmp Lp3
Lp4:
 fstp st(1)
Lp3:
 fld st(3)
 fmul dword ptr[_aliastransform+0]
 fld st(3)
 fmul dword ptr[_aliastransform+4]
 fxch st(1)
 fadd dword ptr[_aliastransform+12]
 fld st(3)
 fmul dword ptr[_aliastransform+8]
 fxch st(1)
 faddp st(2),st(0)
 fxch st(5)
 fmul dword ptr[_aliastransform+16]
 fxch st(4)
 fmul dword ptr[_aliastransform+20]
 fxch st(1)
 faddp st(5),st(0)
 fxch st(3)
 fadd dword ptr[_aliastransform+28]
 fxch st(2)
 fmul dword ptr[_aliastransform+24]
 fxch st(4)
 fmul st(0),st(1)
 fxch st(2)
 faddp st(3),st(0)
 mov dword ptr[12+esi],ecx
 mov dword ptr[16+esi],edx
 fxch st(1)
 fadd dword ptr[_aliasxcenter]
 fxch st(2)
 faddp st(3),st(0)
 fist dword ptr[8+esi]
 fmulp st(2),st(0)
 fistp dword ptr[esi]
 fadd dword ptr[_aliasycenter]
 mov dword ptr[20+esi],ebx
 mov dword ptr[24+esi],0
 fistp dword ptr[4+esi]
 add edi,4
 add esi,32
 dec ebp
 jnz LTLoop
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
_TEXT ENDS
 END
