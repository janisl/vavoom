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
_TEXT SEGMENT
 align 4
 public _D_DrawParticle_15
_D_DrawParticle_15:
 push ebp
 push edi
 push ebx
 mov edi,dword ptr[12+4+esp]
 mov ebx,dword ptr[12+edi]
 mov cl,byte ptr[_rshift]
 mov ebp,ebx
 shr ebp,19
 mov eax,ebx
 shr eax,11
 and ebp,31
 sal ebp,cl
 shr ebx,3
 mov cl,byte ptr[_gshift]
 and eax,31
 and ebx,31
 sal eax,cl
 mov cl,byte ptr[_bshift]
 or ebp,eax
 sal ebx,cl
 or ebp,ebx
 mov dword ptr[DP_Color],ebp
 jmp L15bppEntry
 align 4
 public _D_DrawParticle_16
_D_DrawParticle_16:
 push ebp
 push edi
 push ebx
 mov edi,dword ptr[12+4+esp]
 mov ebx,dword ptr[12+edi]
 mov cl,byte ptr[_rshift]
 mov ebp,ebx
 shr ebp,19
 mov eax,ebx
 shr eax,10
 and ebp,31
 sal ebp,cl
 shr ebx,3
 mov cl,byte ptr[_gshift]
 and eax,63
 and ebx,31
 sal eax,cl
 mov cl,byte ptr[_bshift]
 or ebp,eax
 sal ebx,cl
 or ebp,ebx
 mov dword ptr[DP_Color],ebp
L15bppEntry:
 fld dword ptr[_vieworg]
 fsubr dword ptr[0+edi]
 fld dword ptr[0+4+edi]
 fsub dword ptr[_vieworg+4]
 fld dword ptr[0+8+edi]
 fsub dword ptr[_vieworg+8]
 fxch st(2)
 fld dword ptr[_r_ppn]
 fmul st(0),st(1)
 fld dword ptr[_r_ppn+4]
 fmul st(0),st(3)
 fld dword ptr[_r_ppn+8]
 fmul st(0),st(5)
 fxch st(2)
 faddp st(1),st(0)
 faddp st(1),st(0)
 fld st(0)
 fdivr dword ptr[float_1]
 fxch st(1)
 fcomp dword ptr[float_particle_z_clip]
 fxch st(3)
 fld dword ptr[_r_pup]
 fmul st(0),st(2)
 fld dword ptr[_r_pup+4]
 fnstsw ax
 test ah,1
 jnz LPop6AndDone
 fmul st(0),st(4)
 fld dword ptr[_r_pup+8]
 fmul st(0),st(3)
 fxch st(2)
 faddp st(1),st(0)
 faddp st(1),st(0)
 fxch st(3)
 fmul dword ptr[_r_pright+4]
 fxch st(2)
 fmul dword ptr[_r_pright]
 fxch st(1)
 fmul dword ptr[_r_pright+8]
 fxch st(2)
 faddp st(1),st(0)
 faddp st(1),st(0)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(1)
 fadd dword ptr[_centeryfrac]
 fxch st(1)
 fadd dword ptr[_centerxfrac]
 fxch st(1)
 fadd dword ptr[float_point5]
 fxch st(1)
 fadd dword ptr[float_point5]
 fxch st(2)
 fmul dword ptr[DP_32768]
 fxch st(2)
 fistp dword ptr[DP_u]
 fistp dword ptr[DP_v]
 mov eax,dword ptr[DP_u]
 mov edx,dword ptr[DP_v]
 cmp edx,0
 jl LPop1AndDone
 cmp eax,0
 jl LPop1AndDone
 mov ebx,dword ptr[_d_particle_top]
 mov ecx,dword ptr[_d_particle_right]
 cmp edx,ebx
 jg LPop1AndDone
 cmp eax,ecx
 jg LPop1AndDone
 mov ebx,dword ptr[_scrn]
 mov edi,dword ptr[_ylookup+edx*4]
 add edi,eax
 shl edi,1
 mov edx,edi
 mov eax,dword ptr[_zbuffer]
 fistp dword ptr[izi]
 add edi,ebx
 add edx,eax
 mov eax,dword ptr[izi]
 mov ecx,dword ptr[_d_pix_shift]
 shr eax,cl
 mov ebp,dword ptr[izi]
 mov ebx,dword ptr[_d_pix_min]
 mov ecx,dword ptr[_d_pix_max]
 cmp eax,ebx
 jnl LTestPixMax
 mov eax,ebx
 jmp LTestDone
LTestPixMax:
 cmp eax,ecx
 jng LTestDone
 mov eax,ecx
LTestDone:
 mov ebx,dword ptr[_d_y_aspect_shift]
 test ebx,ebx
 jnz LDefault
 cmp eax,4
 ja LDefault
 mov cx,word ptr[DP_Color]
 jmp  dword ptr[DP_EntryTable-4+eax*4]
 align 4
DP_EntryTable:
 dd LDP_1x1
 dd LDP_2x2
 dd LDP_3x3
 dd LDP_4x4
 align 4
LDP_1x1:
 cmp word ptr[edx],bp
 jg LDone
 mov word ptr[edx],bp
 mov word ptr[edi],cx
 jmp LDone
 align 4
LDP_2x2:
 push esi
 mov ebx,dword ptr[_d_rowbytes]
 mov esi,dword ptr[_d_zrowbytes]
 cmp word ptr[edx],bp
 jg L2x2_1
 mov word ptr[edx],bp
 mov word ptr[edi],cx
L2x2_1:
 cmp word ptr[2+edx],bp
 jg L2x2_2
 mov word ptr[2+edx],bp
 mov word ptr[2+edi],cx
L2x2_2:
 cmp word ptr[edx+esi*1],bp
 jg L2x2_3
 mov word ptr[edx+esi*1],bp
 mov word ptr[edi+ebx*1],cx
L2x2_3:
 cmp word ptr[2+edx+esi*1],bp
 jg L2x2_4
 mov word ptr[2+edx+esi*1],bp
 mov word ptr[2+edi+ebx*1],cx
L2x2_4:
 pop esi
 jmp LDone
 align 4
LDP_3x3:
 push esi
 mov ebx,dword ptr[_d_rowbytes]
 mov esi,dword ptr[_d_zrowbytes]
 cmp word ptr[edx],bp
 jg L3x3_1
 mov word ptr[edx],bp
 mov word ptr[edi],cx
L3x3_1:
 cmp word ptr[2+edx],bp
 jg L3x3_2
 mov word ptr[2+edx],bp
 mov word ptr[2+edi],cx
L3x3_2:
 cmp word ptr[4+edx],bp
 jg L3x3_3
 mov word ptr[4+edx],bp
 mov word ptr[4+edi],cx
L3x3_3:
 cmp word ptr[edx+esi*1],bp
 jg L3x3_4
 mov word ptr[edx+esi*1],bp
 mov word ptr[edi+ebx*1],cx
L3x3_4:
 cmp word ptr[2+edx+esi*1],bp
 jg L3x3_5
 mov word ptr[2+edx+esi*1],bp
 mov word ptr[2+edi+ebx*1],cx
L3x3_5:
 cmp word ptr[4+edx+esi*1],bp
 jg L3x3_6
 mov word ptr[4+edx+esi*1],bp
 mov word ptr[4+edi+ebx*1],cx
L3x3_6:
 cmp word ptr[edx+esi*2],bp
 jg L3x3_7
 mov word ptr[edx+esi*2],bp
 mov word ptr[edi+ebx*2],cx
L3x3_7:
 cmp word ptr[2+edx+esi*2],bp
 jg L3x3_8
 mov word ptr[2+edx+esi*2],bp
 mov word ptr[2+edi+ebx*2],cx
L3x3_8:
 cmp word ptr[4+edx+esi*2],bp
 jg L3x3_9
 mov word ptr[4+edx+esi*2],bp
 mov word ptr[4+edi+ebx*2],cx
L3x3_9:
 pop esi
 jmp LDone
 align 4
LDP_4x4:
 push esi
 mov ebx,dword ptr[_d_rowbytes]
 mov esi,dword ptr[_d_zrowbytes]
 cmp word ptr[edx],bp
 jg L4x4_1
 mov word ptr[edx],bp
 mov word ptr[edi],cx
L4x4_1:
 cmp word ptr[2+edx],bp
 jg L4x4_2
 mov word ptr[2+edx],bp
 mov word ptr[2+edi],cx
L4x4_2:
 cmp word ptr[4+edx],bp
 jg L4x4_3
 mov word ptr[4+edx],bp
 mov word ptr[4+edi],cx
L4x4_3:
 cmp word ptr[6+edx],bp
 jg L4x4_4
 mov word ptr[6+edx],bp
 mov word ptr[6+edi],cx
L4x4_4:
 cmp word ptr[edx+esi*1],bp
 jg L4x4_5
 mov word ptr[edx+esi*1],bp
 mov word ptr[edi+ebx*1],cx
L4x4_5:
 cmp word ptr[2+edx+esi*1],bp
 jg L4x4_6
 mov word ptr[2+edx+esi*1],bp
 mov word ptr[2+edi+ebx*1],cx
L4x4_6:
 cmp word ptr[4+edx+esi*1],bp
 jg L4x4_7
 mov word ptr[4+edx+esi*1],bp
 mov word ptr[4+edi+ebx*1],cx
L4x4_7:
 cmp word ptr[6+edx+esi*1],bp
 jg L4x4_8
 mov word ptr[6+edx+esi*1],bp
 mov word ptr[6+edi+ebx*1],cx
L4x4_8:
 lea edx,dword ptr[edx+esi*2]
 lea edi,dword ptr[edi+ebx*2]
 cmp word ptr[edx],bp
 jg L4x4_9
 mov word ptr[edx],bp
 mov word ptr[edi],cx
L4x4_9:
 cmp word ptr[2+edx],bp
 jg L4x4_10
 mov word ptr[2+edx],bp
 mov word ptr[2+edi],cx
L4x4_10:
 cmp word ptr[4+edx],bp
 jg L4x4_11
 mov word ptr[4+edx],bp
 mov word ptr[4+edi],cx
L4x4_11:
 cmp word ptr[6+edx],bp
 jg L4x4_12
 mov word ptr[6+edx],bp
 mov word ptr[6+edi],cx
L4x4_12:
 cmp word ptr[edx+esi*1],bp
 jg L4x4_13
 mov word ptr[edx+esi*1],bp
 mov word ptr[edi+ebx*1],cx
L4x4_13:
 cmp word ptr[2+edx+esi*1],bp
 jg L4x4_14
 mov word ptr[2+edx+esi*1],bp
 mov word ptr[2+edi+ebx*1],cx
L4x4_14:
 cmp word ptr[4+edx+esi*1],bp
 jg L4x4_15
 mov word ptr[4+edx+esi*1],bp
 mov word ptr[4+edi+ebx*1],cx
L4x4_15:
 cmp word ptr[6+edx+esi*1],bp
 jg L4x4_16
 mov word ptr[6+edx+esi*1],bp
 mov word ptr[6+edi+ebx*1],cx
L4x4_16:
 pop esi
 jmp LDone
 align 4
LDefault:
 mov ebx,eax
 mov dword ptr[DP_Pix],eax
 mov cl,byte ptr[_d_y_aspect_shift]
 shl ebx,cl
 mov cx,word ptr[DP_Color]
LGenRowLoop:
 mov eax,dword ptr[DP_Pix]
LGenColLoop:
 cmp word ptr[-2+edx+eax*2],bp
 jg LGSkip
 mov word ptr[-2+edx+eax*2],bp
 mov word ptr[-2+edi+eax*2],cx
LGSkip:
 dec eax
 jnz LGenColLoop
 add edx,dword ptr[_d_zrowbytes]
 add edi,dword ptr[_d_rowbytes]
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
