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
 externdef _D_DrawZSpan
_DATA SEGMENT
 align 4
Lfp_near_clip dd 0.01
Lfp_1m dd 1048576.0
Lfp_1m_minus_1 dd 1048575.0
Lu0 dd 0
Lv0 dd 0
Ltemp dd 0
Ld0 dd 0
Ld1 dd 0
_DATA ENDS
_TEXT SEGMENT
 public _D_ClipEdge
_D_ClipEdge:
 push ebp
 push edi
 push esi
 push ebx
 mov esi,dword ptr[20+0+esp]
 mov edi,dword ptr[20+4+esp]
 mov ebx,dword ptr[20+8+esp]
 mov ebp,dword ptr[20+12+esp]
 test ebx,ebx
 jz LEmitEdge
LClipLoop:
 mov eax,ebp
 mov ecx,dword ptr[36+ebx]
 test eax,ecx
 jz LNoClip
 fld dword ptr[esi]
 fmul dword ptr[ebx]
 fld dword ptr[4+esi]
 fmul dword ptr[4+ebx]
 fld dword ptr[8+esi]
 fmul dword ptr[8+ebx]
 fxch st(2)
 faddp st(1),st(0)
 fld dword ptr[edi]
 fmul dword ptr[ebx]
 fld dword ptr[4+edi]
 fmul dword ptr[4+ebx]
 fld dword ptr[8+edi]
 fmul dword ptr[8+ebx]
 fxch st(2)
 faddp st(1),st(0)
 fxch st(3)
 fsub dword ptr[12+ebx]
 fxch st(1)
 fsub dword ptr[12+ebx]
 fxch st(1)
 faddp st(2),st(0)
 faddp st(2),st(0)
 fstp dword ptr[Ld0]
 fstp dword ptr[Ld1]
 mov eax,dword ptr[Ld0]
 mov ecx,dword ptr[Ld1]
 or ecx,eax
 js LIsClipped
LNoClip:
 mov ebx,dword ptr[32+ebx]
 test ebx,ebx
 jne LClipLoop
LEmitEdge:
 fldcw word ptr[ceil_cw]
 cmp dword ptr[_d_lastvertvalid],0
 je LTransformAndProjectV0
 mov eax,dword ptr[_d_u1]
 mov edx,dword ptr[_d_v1]
 mov dword ptr[Lu0],eax
 mov dword ptr[Lv0],edx
 mov ebx,dword ptr[_d_ceilv1]
 jmp LTransformAndProjectV1
LTransformAndProjectV0:
 fld dword ptr[esi]
 fsub dword ptr[_vieworg+0]
 fld dword ptr[4+esi]
 fsub dword ptr[_vieworg+4]
 fld dword ptr[8+esi]
 fsub dword ptr[_vieworg+8]
 fld st(2)
 fmul dword ptr[_viewforward+0]
 fld st(3)
 fmul dword ptr[_viewright]
 fxch st(4)
 fmul dword ptr[_viewup]
 fld st(3)
 fmul dword ptr[_viewforward+4]
 fld st(4)
 fmul dword ptr[_viewright+4]
 fxch st(5)
 fmul dword ptr[_viewup+4]
 fxch st(3)
 faddp st(1),st(0)
 fxch st(5)
 faddp st(4),st(0)
 faddp st(1),st(0)
 fld st(1)
 fmul dword ptr[_viewforward+8]
 fld st(2)
 fmul dword ptr[_viewright+8]
 fxch st(3)
 fmul dword ptr[_viewup+8]
 fxch st(1)
 faddp st(5),st(0)
 fxch st(2)
 faddp st(3),st(0)
 faddp st(1),st(0)
 fxch st(2)
 fcom dword ptr[Lfp_near_clip]
 fnstsw ax
 test ah,1
 jz LNoClip0
 fstp st(0)
 fld dword ptr[Lfp_near_clip]
LNoClip0:
 fld1
 fdivrp st(1),st(0)
 mov eax,dword ptr[esi]
 mov dword ptr[_firstvert+0],eax
 mov eax,dword ptr[4+esi]
 mov dword ptr[_firstvert+4],eax
 mov eax,dword ptr[8+esi]
 mov dword ptr[_firstvert+8],eax
 fcom dword ptr[_r_nearzi]
 fnstsw ax
 test ah,045h
 jnz LNoSetNearZi0
 fst dword ptr[_r_nearzi]
LNoSetNearZi0:
 fmul st(1),st(0)
 fmulp st(2),st(0)
 fmul dword ptr[_xprojection]
 fxch st(1)
 fmul dword ptr[_yprojection]
 fxch st(1)
 fadd dword ptr[_centerxfrac]
 fxch st(1)
 fadd dword ptr[_centeryfrac]
 fxch st(1)
 fcom dword ptr[_vrectx_adj]
 fnstsw ax
 test ah,1
 jz LClampP00
 fstp st(0)
 fld dword ptr[_vrectx_adj]
LClampP00:
 fcom dword ptr[_vrectw_adj]
 fnstsw ax
 test ah,045h
 jnz LClampP10
 fstp st(0)
 fld dword ptr[_vrectw_adj]
LClampP10:
 fxch st(1)
 fcom dword ptr[_vrecty_adj]
 fnstsw ax
 test ah,1
 jz LClampP20
 fstp st(0)
 fld dword ptr[_vrecty_adj]
LClampP20:
 fcom dword ptr[_vrecth_adj]
 fnstsw ax
 test ah,045h
 jnz LClampP30
 fstp st(0)
 fld dword ptr[_vrecth_adj]
LClampP30:
 fist dword ptr[Ltemp]
 mov ebx,dword ptr[Ltemp]
 fstp dword ptr[Lv0]
 fstp dword ptr[Lu0]
LTransformAndProjectV1:
 fld dword ptr[edi]
 fsub dword ptr[_vieworg+0]
 fld dword ptr[4+edi]
 fsub dword ptr[_vieworg+4]
 fld dword ptr[8+edi]
 fsub dword ptr[_vieworg+8]
 fld st(2)
 fmul dword ptr[_viewforward+0]
 fld st(3)
 fmul dword ptr[_viewright+0]
 fxch st(4)
 fmul dword ptr[_viewup+0]
 fld st(3)
 fmul dword ptr[_viewforward+4]
 fld st(4)
 fmul dword ptr[_viewright+4]
 fxch st(5)
 fmul dword ptr[_viewup+4]
 fxch st(3)
 faddp st(1),st(0)
 fxch st(5)
 faddp st(4),st(0)
 faddp st(1),st(0)
 fld st(1)
 fmul dword ptr[_viewforward+8]
 fld st(2)
 fmul dword ptr[_viewright+8]
 fxch st(3)
 fmul dword ptr[_viewup+8]
 fxch st(1)
 faddp st(5),st(0)
 fxch st(2)
 faddp st(3),st(0)
 faddp st(1),st(0)
 fxch st(2)
 fcom dword ptr[Lfp_near_clip]
 fnstsw ax
 test ah,1
 jz LNoClip1
 fstp st(0)
 fld dword ptr[Lfp_near_clip]
LNoClip1:
 fld1
 fdivrp st(1),st(0)
 fcom dword ptr[_r_nearzi]
 fnstsw ax
 test ah,045h
 jnz LNoSetNearZi1
 fst dword ptr[_r_nearzi]
LNoSetNearZi1:
 fmul st(1),st(0)
 fmulp st(2),st(0)
 fmul dword ptr[_xprojection]
 fxch st(1)
 fmul dword ptr[_yprojection]
 fxch st(1)
 fadd dword ptr[_centerxfrac]
 fxch st(1)
 fadd dword ptr[_centeryfrac]
 fxch st(1)
 fcom dword ptr[_vrectx_adj]
 fnstsw ax
 test ah,1
 jz LClampP01
 fstp st(0)
 fld dword ptr[_vrectx_adj]
LClampP01:
 fcom dword ptr[_vrectw_adj]
 fnstsw ax
 test ah,045h
 jnz LClampP11
 fstp st(0)
 fld dword ptr[_vrectw_adj]
LClampP11:
 fxch st(1)
 fcom dword ptr[_vrecty_adj]
 fnstsw ax
 test ah,1
 jz LClampP21
 fstp st(0)
 fld dword ptr[_vrecty_adj]
LClampP21:
 fcom dword ptr[_vrecth_adj]
 fnstsw ax
 test ah,045h
 jnz LClampP31
 fstp st(0)
 fld dword ptr[_vrecth_adj]
LClampP31:
 fist dword ptr[_d_ceilv1]
 fxch st(1)
 fst dword ptr[_d_u1]
 fxch st(1)
 fst dword ptr[_d_v1]
 fld dword ptr[Lv0]
 fld dword ptr[Lu0]
 fldcw word ptr[single_cw]
 mov eax,dword ptr[_d_ceilv1]
 mov dword ptr[_d_lastvertvalid],1
 cmp ebx,eax
 je LPop4AndDone
 mov esi,dword ptr[_edge_p]
 mov dword ptr[_r_emited],1
 add dword ptr[_edge_p],24
 cmp ebx,eax
 jle LEmitLeadingEdge
 fsub st(0),st(3)
 fxch st(1)
 fsub st(0),st(2)
 fdivp st(1),st(0)
 mov edx,eax
 mov eax,dword ptr[_surface_p]
 sub eax,offset _surfaces
 sar eax,6
 mov word ptr[18+esi],0
 mov word ptr[16+esi],ax
 mov dword ptr[Ltemp],edx
 lea ebp,dword ptr[-1+ebx]
 fild dword ptr[Ltemp]
 fsubrp st(2),st(0)
 fmul st(1),st(0)
 fxch st(2)
 faddp st(1),st(0)
 jmp LSideDone
LEmitLeadingEdge:
 fsub st(3),st(0)
 fxch st(2)
 fsub st(0),st(1)
 fdivp st(3),st(0)
 lea ebp,dword ptr[-1+eax]
 mov eax,dword ptr[_surface_p]
 sub eax,offset _surfaces
 sar eax,6
 mov edx,ebx
 mov word ptr[16+esi],0
 mov word ptr[18+esi],ax
 mov dword ptr[Ltemp],edx
 fild dword ptr[Ltemp]
 fsubrp st(1),st(0)
 fmul st(0),st(2)
 faddp st(1),st(0)
LSideDone:
 fmul dword ptr[Lfp_1m]
 fxch st(1)
 fmul dword ptr[Lfp_1m]
 fxch st(1)
 fadd dword ptr[Lfp_1m_minus_1]
 fxch st(1)
 fistp dword ptr[4+esi]
 fistp dword ptr[esi]
 mov ebx,dword ptr[esi]
 mov eax,dword ptr[4+esi]
 cmp word ptr[16+esi],0
 je LisLeader
 inc ebx
LisLeader:
 sal edx,2
 mov edi,offset _newedges
 mov eax,dword ptr[edx+edi]
 test eax,eax
 je LIsFirst
 cmp dword ptr[eax],ebx
 jl LNotFirst
LIsFirst:
 mov dword ptr[12+esi],eax
 mov dword ptr[edx+edi],esi
 jmp LSetRemove
LNotFirst:
LSortLoop:
 mov edx,eax
 mov eax,dword ptr[12+edx]
 test eax,eax
 je LFoundSpot
 cmp dword ptr[eax],ebx
 jl LSortLoop
LFoundSpot:
 mov eax,dword ptr[12+edx]
 mov dword ptr[12+esi],eax
 mov dword ptr[12+edx],esi
LSetRemove:
 mov eax,dword ptr[_removeedges+ebp*4]
 mov dword ptr[20+esi],eax
 mov dword ptr[_removeedges+ebp*4],esi
LClipDone:
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
LPop4AndDone:
 fstp st(0)
 fstp st(0)
 fstp st(0)
 fstp st(0)
 jmp LClipDone
LIsClipped:
 test eax,eax
 jns LPoint1IsClipped
 mov eax,dword ptr[Ld1]
 test eax,eax
 jns LPoint0IsClipped
 jmp LClipDone
LPoint0IsClipped:
 fld dword ptr[Ld0]
 fld dword ptr[Ld1]
 fld st(1)
 fsubrp st(1),st(0)
 fdivp st(1),st(0)
 mov dword ptr[68+ebx],1
 lea ecx,dword ptr[52+ebx]
 fld dword ptr[edi]
 fsub dword ptr[esi]
 fld dword ptr[4+edi]
 fsub dword ptr[4+esi]
 fld dword ptr[8+edi]
 fsub dword ptr[8+esi]
 fxch st(2)
 fmul st(0),st(3)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(2)
 fmulp st(3),st(0)
 fadd dword ptr[esi]
 fxch st(1)
 fadd dword ptr[4+esi]
 fxch st(2)
 fadd dword ptr[8+esi]
 fxch st(1)
 fstp dword ptr[ecx]
 fxch st(1)
 fstp dword ptr[4+ecx]
 fstp dword ptr[8+ecx]
 cmp dword ptr[64+ebx],0
 je LNoEntered
 mov dword ptr[64+ebx],0
 mov dword ptr[68+ebx],0
 mov eax,ebp
 xor eax,dword ptr[36+ebx]
 push eax
 push offset _view_clipplanes
 push ecx
 lea eax,dword ptr[40+ebx]
 push eax
 call _D_ClipEdge
 add esp,16
 lea ecx,dword ptr[52+ebx]
LNoEntered:
 push ebp
 mov eax,dword ptr[32+ebx]
 push eax
 push edi
 push ecx
 call _D_ClipEdge
 add esp,16
 jmp LClipDone
LPoint1IsClipped:
 fld dword ptr[Ld0]
 fld dword ptr[Ld1]
 fld st(1)
 fsubrp st(1),st(0)
 fdivp st(1),st(0)
 mov dword ptr[64+ebx],1
 lea ecx,dword ptr[40+ebx]
 push ebp
 mov eax,dword ptr[32+ebx]
 push eax
 push ecx
 push esi
 fld dword ptr[edi]
 fsub dword ptr[esi]
 fld dword ptr[4+edi]
 fsub dword ptr[4+esi]
 fld dword ptr[8+edi]
 fsub dword ptr[8+esi]
 fxch st(2)
 fmul st(0),st(3)
 fxch st(1)
 fmul st(0),st(3)
 fxch st(2)
 fmulp st(3),st(0)
 fadd dword ptr[esi]
 fxch st(1)
 fadd dword ptr[4+esi]
 fxch st(2)
 fadd dword ptr[8+esi]
 fxch st(1)
 fstp dword ptr[ecx]
 fxch st(1)
 fstp dword ptr[4+ecx]
 fstp dword ptr[8+ecx]
 call _D_ClipEdge
 add esp,16
 cmp dword ptr[68+ebx],0
 je LClipDone
 mov dword ptr[64+ebx],0
 mov dword ptr[68+ebx],0
 xor ebp,dword ptr[36+ebx]
 push ebp
 push offset _view_clipplanes
 lea eax,dword ptr[52+ebx]
 lea ecx,dword ptr[40+ebx]
 push eax
 push ecx
 call _D_ClipEdge
 add esp,16
 jmp LClipDone
 public _D_GenerateSpans
_D_GenerateSpans:
 push ebp
 push edi
 push esi
 push ebx
 mov eax,offset _surfaces+64
 mov dword ptr[_surfaces+80],0
 mov dword ptr[_surfaces+68],eax
 mov dword ptr[_surfaces+64],eax
 mov edi,dword ptr[_edge_head+12]
 cmp edi,offset _edge_tail
 je LCleanupSpan
LEdgesLoop:
 cmp word ptr[16+edi],0
 je LLeadingEdge
 xor ecx,ecx
 mov cx,word ptr[16+edi]
 sal ecx,6
 lea esi,dword ptr[_surfaces+ecx]
 dec dword ptr[_surfaces+20+ecx]
 jnz LLeadingEdge
 cmp esi,dword ptr[_surfaces+64]
 jne LUnlinkSurf
 mov ebx,dword ptr[edi]
 mov eax,dword ptr[_surfaces+16+ecx]
 sar ebx,20
 cmp ebx,eax
 jle LTrailingNoSpan
 mov ebp,dword ptr[_span_p]
 mov edx,ebx
 add dword ptr[_span_p],16
 sub edx,eax
 mov dword ptr[ebp],eax
 mov dword ptr[8+ebp],edx
 mov eax,dword ptr[_current_iv]
 mov edx,dword ptr[_surfaces+8+ecx]
 mov dword ptr[4+ebp],eax
 mov dword ptr[12+ebp],edx
 mov dword ptr[_surfaces+8+ecx],ebp
LTrailingNoSpan:
 mov eax,dword ptr[_surfaces+ecx]
 mov dword ptr[16+eax],ebx
LUnlinkSurf:
 mov edx,dword ptr[4+esi]
 mov eax,dword ptr[esi]
 mov dword ptr[edx],eax
 mov edx,dword ptr[esi]
 mov eax,dword ptr[4+esi]
 mov dword ptr[4+edx],eax
LLeadingEdge:
 cmp word ptr[18+edi],0
 je LNextEdge
 xor ebx,ebx
 mov bx,word ptr[18+edi]
 sal ebx,6
 lea ebp,dword ptr[_surfaces+ebx]
 mov eax,dword ptr[_surfaces+20+ebx]
 inc dword ptr[_surfaces+20+ebx]
 test eax,eax
 jnz LNextEdge
 mov ecx,dword ptr[_surfaces+64]
 mov eax,dword ptr[_surfaces+12+ebx]
 cmp eax,dword ptr[12+ecx]
 jl LNewTop
LSurfSearchLoop:
 mov ecx,dword ptr[ecx]
 cmp eax,dword ptr[12+ecx]
 jge LSurfSearchLoop
 jmp LGotPosition
LNewTop:
 mov esi,dword ptr[edi]
 sar esi,20
 mov dword ptr[_surfaces+16+ebx],esi
 mov eax,dword ptr[16+ecx]
 cmp esi,eax
 jle LGotPosition
 mov edx,dword ptr[_span_p]
 add dword ptr[_span_p],16
 sub esi,eax
 mov dword ptr[edx],eax
 mov dword ptr[8+edx],esi
 mov eax,dword ptr[_current_iv]
 mov dword ptr[4+edx],eax
 mov eax,dword ptr[8+ecx]
 mov dword ptr[12+edx],eax
 mov dword ptr[8+ecx],edx
LGotPosition:
 mov dword ptr[ebp],ecx
 mov eax,dword ptr[4+ecx]
 mov dword ptr[4+ebp],eax
 mov eax,dword ptr[4+ecx]
 mov dword ptr[eax],ebp
 mov dword ptr[4+ecx],ebp
LNextEdge:
 mov edi,dword ptr[12+edi]
 cmp edi,offset _edge_tail
 jne LEdgesLoop
LCleanupSpan:
 mov ecx,dword ptr[_surfaces+64]
 mov ebx,dword ptr[_viewwidth]
 mov edx,dword ptr[16+ecx]
 cmp ebx,edx
 jle LResetSurfacesLoop
 mov eax,dword ptr[_span_p]
 add dword ptr[_span_p],16
 mov dword ptr[eax],edx
 sub ebx,edx
 mov edx,dword ptr[_current_iv]
 mov dword ptr[8+eax],ebx
 mov dword ptr[4+eax],edx
 mov edx,dword ptr[8+ecx]
 mov dword ptr[12+eax],edx
 mov dword ptr[8+ecx],eax
LResetSurfacesLoop:
 mov dword ptr[20+ecx],0
 mov ecx,dword ptr[ecx]
 cmp ecx,offset _surfaces+64
 jne LResetSurfacesLoop
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
_TEXT ENDS
 END
