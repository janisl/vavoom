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
sb_v dd 0
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _D_Surf32Start
_D_Surf32Start:
 align 4
 public _D_DrawSurfaceBlock32_mip0
_D_DrawSurfaceBlock32_mip0:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx,dword ptr[_r_lightptr]
 mov eax,dword ptr[_r_numvblocks]
 mov dword ptr[sb_v],eax
 mov edi,dword ptr[_prowdestbase]
 mov esi,dword ptr[_pbasesource]
Lv_loop_mip0:
 mov eax,dword ptr[ebx]
 mov edx,dword ptr[4+ebx]
 mov ebp,eax
 mov ecx,dword ptr[_r_lightwidth]
 mov dword ptr[_lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx,dword ptr[ebx+ecx*4]
 mov dword ptr[_r_lightptr],ebx
 mov ecx,dword ptr[4+ebx]
 mov ebx,dword ptr[ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,4
 or ebp,0F0000000h
 sar ebx,4
 mov dword ptr[_lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 or ebx,0F0000000h
 sub ecx,ecx
 mov dword ptr[_lightdeltastep],ebx
 sub ebx,ebx
Lblockloop16_mip0:
 mov dword ptr[_lightdelta],ebp
 mov cl,byte ptr[14+esi]
 sar ebp,4
 mov bh,dh
 mov bl,byte ptr[15+esi]
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov eax,dword ptr[12345678h+ebx*4]
LPatch0:
 mov bl,byte ptr[13+esi]
 mov dword ptr[60+edi],eax
 mov eax,dword ptr[12345678h+ecx*4]
LPatch1:
 mov cl,byte ptr[12+esi]
 mov bh,dh
 add edx,ebp
 mov dword ptr[56+edi],eax
 mov ch,dh
 mov eax,dword ptr[12345678h+ebx*4]
LPatch2:
 add edx,ebp
 mov dword ptr[52+edi],eax
 mov eax,dword ptr[12345678h+ecx*4]
LPatch3:
 mov bl,byte ptr[11+esi]
 mov cl,byte ptr[10+esi]
 mov dword ptr[48+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov eax,dword ptr[12345678h+ebx*4]
LPatch4:
 mov bl,byte ptr[9+esi]
 mov dword ptr[44+edi],eax
 mov eax,dword ptr[12345678h+ecx*4]
LPatch5:
 mov cl,byte ptr[8+esi]
 mov bh,dh
 add edx,ebp
 mov dword ptr[40+edi],eax
 mov ch,dh
 mov eax,dword ptr[12345678h+ebx*4]
LPatch6:
 add edx,ebp
 mov dword ptr[36+edi],eax
 mov eax,dword ptr[12345678h+ecx*4]
LPatch7:
 mov bl,byte ptr[7+esi]
 mov cl,byte ptr[6+esi]
 mov dword ptr[32+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov eax,dword ptr[12345678h+ebx*4]
LPatch8:
 mov bl,byte ptr[5+esi]
 mov dword ptr[28+edi],eax
 mov eax,dword ptr[12345678h+ecx*4]
LPatch9:
 mov cl,byte ptr[4+esi]
 mov bh,dh
 add edx,ebp
 mov dword ptr[24+edi],eax
 mov ch,dh
 mov eax,dword ptr[12345678h+ebx*4]
LPatch10:
 add edx,ebp
 mov dword ptr[20+edi],eax
 mov eax,dword ptr[12345678h+ecx*4]
LPatch11:
 mov bl,byte ptr[3+esi]
 mov cl,byte ptr[2+esi]
 mov dword ptr[16+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov eax,dword ptr[12345678h+ebx*4]
LPatch12:
 mov bl,byte ptr[1+esi]
 mov dword ptr[12+edi],eax
 mov eax,dword ptr[12345678h+ecx*4]
LPatch13:
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov dword ptr[8+edi],eax
 mov ch,dh
 mov eax,dword ptr[12345678h+ebx*4]
LPatch14:
 mov edx,dword ptr[_lightright]
 mov dword ptr[4+edi],eax
 mov eax,dword ptr[12345678h+ecx*4]
LPatch15:
 mov ebp,dword ptr[_lightdelta]
 mov dword ptr[edi],eax
 add esi,dword ptr[_sourcetstep]
 add edi,dword ptr[_surfrowbytes]
 add edx,dword ptr[_lightrightstep]
 add ebp,dword ptr[_lightdeltastep]
 mov dword ptr[_lightright],edx
 jc Lblockloop16_mip0
 cmp esi,dword ptr[_r_sourcemax]
 jb LSkip_mip0
 sub esi,dword ptr[_r_stepback]
LSkip_mip0:
 mov ebx,dword ptr[_r_lightptr]
 dec dword ptr[sb_v]
 jnz Lv_loop_mip0
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 align 4
 public _D_DrawSurfaceBlock32_mip1
_D_DrawSurfaceBlock32_mip1:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx,dword ptr[_r_lightptr]
 mov eax,dword ptr[_r_numvblocks]
 mov dword ptr[sb_v],eax
 mov edi,dword ptr[_prowdestbase]
 mov esi,dword ptr[_pbasesource]
Lv_loop_mip1:
 mov eax,dword ptr[ebx]
 mov edx,dword ptr[4+ebx]
 mov ebp,eax
 mov ecx,dword ptr[_r_lightwidth]
 mov dword ptr[_lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx,dword ptr[ebx+ecx*4]
 mov dword ptr[_r_lightptr],ebx
 mov ecx,dword ptr[4+ebx]
 mov ebx,dword ptr[ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,3
 or ebp,070000000h
 sar ebx,3
 mov dword ptr[_lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 or ebx,0F0000000h
 sub ecx,ecx
 mov dword ptr[_lightdeltastep],ebx
 sub ebx,ebx
Lblockloop16_mip1:
 mov dword ptr[_lightdelta],ebp
 mov cl,byte ptr[6+esi]
 sar ebp,3
 mov bh,dh
 mov bl,byte ptr[7+esi]
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov eax,dword ptr[12345678h+ebx*4]
LPatch22:
 mov bl,byte ptr[5+esi]
 mov dword ptr[28+edi],eax
 mov eax,dword ptr[12345678h+ecx*4]
LPatch23:
 mov cl,byte ptr[4+esi]
 mov bh,dh
 add edx,ebp
 mov dword ptr[24+edi],eax
 mov ch,dh
 mov eax,dword ptr[12345678h+ebx*4]
LPatch24:
 add edx,ebp
 mov dword ptr[20+edi],eax
 mov eax,dword ptr[12345678h+ecx*4]
LPatch25:
 mov bl,byte ptr[3+esi]
 mov cl,byte ptr[2+esi]
 mov dword ptr[16+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov eax,dword ptr[12345678h+ebx*4]
LPatch26:
 mov bl,byte ptr[1+esi]
 mov dword ptr[12+edi],eax
 mov eax,dword ptr[12345678h+ecx*4]
LPatch27:
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov dword ptr[8+edi],eax
 mov ch,dh
 mov eax,dword ptr[12345678h+ebx*4]
LPatch28:
 mov edx,dword ptr[_lightright]
 mov dword ptr[4+edi],eax
 mov eax,dword ptr[12345678h+ecx*4]
LPatch29:
 mov ebp,dword ptr[_lightdelta]
 mov dword ptr[edi],eax
 mov eax,dword ptr[_sourcetstep]
 add esi,eax
 mov eax,dword ptr[_surfrowbytes]
 add edi,eax
 mov eax,dword ptr[_lightrightstep]
 add edx,eax
 mov eax,dword ptr[_lightdeltastep]
 add ebp,eax
 mov dword ptr[_lightright],edx
 jc Lblockloop16_mip1
 cmp esi,dword ptr[_r_sourcemax]
 jb LSkip_mip1
 sub esi,dword ptr[_r_stepback]
LSkip_mip1:
 mov ebx,dword ptr[_r_lightptr]
 dec dword ptr[sb_v]
 jnz Lv_loop_mip1
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 align 4
 public _D_DrawSurfaceBlock32_mip2
_D_DrawSurfaceBlock32_mip2:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx,dword ptr[_r_lightptr]
 mov eax,dword ptr[_r_numvblocks]
 mov dword ptr[sb_v],eax
 mov edi,dword ptr[_prowdestbase]
 mov esi,dword ptr[_pbasesource]
Lv_loop_mip2:
 mov eax,dword ptr[ebx]
 mov edx,dword ptr[4+ebx]
 mov ebp,eax
 mov ecx,dword ptr[_r_lightwidth]
 mov dword ptr[_lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx,dword ptr[ebx+ecx*4]
 mov dword ptr[_r_lightptr],ebx
 mov ecx,dword ptr[4+ebx]
 mov ebx,dword ptr[ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,2
 or ebp,030000000h
 sar ebx,2
 mov dword ptr[_lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 or ebx,0F0000000h
 sub ecx,ecx
 mov dword ptr[_lightdeltastep],ebx
 sub ebx,ebx
Lblockloop16_mip2:
 mov dword ptr[_lightdelta],ebp
 mov cl,byte ptr[2+esi]
 sar ebp,2
 mov bh,dh
 mov bl,byte ptr[3+esi]
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov eax,dword ptr[12345678h+ebx*4]
LPatch18:
 mov bl,byte ptr[1+esi]
 mov dword ptr[12+edi],eax
 mov eax,dword ptr[12345678h+ecx*4]
LPatch19:
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov dword ptr[8+edi],eax
 mov ch,dh
 mov eax,dword ptr[12345678h+ebx*4]
LPatch20:
 mov edx,dword ptr[_lightright]
 mov dword ptr[4+edi],eax
 mov eax,dword ptr[12345678h+ecx*4]
LPatch21:
 mov ebp,dword ptr[_lightdelta]
 mov dword ptr[edi],eax
 mov eax,dword ptr[_sourcetstep]
 add esi,eax
 mov eax,dword ptr[_surfrowbytes]
 add edi,eax
 mov eax,dword ptr[_lightrightstep]
 add edx,eax
 mov eax,dword ptr[_lightdeltastep]
 add ebp,eax
 mov dword ptr[_lightright],edx
 jc Lblockloop16_mip2
 cmp esi,dword ptr[_r_sourcemax]
 jb LSkip_mip2
 sub esi,dword ptr[_r_stepback]
LSkip_mip2:
 mov ebx,dword ptr[_r_lightptr]
 dec dword ptr[sb_v]
 jnz Lv_loop_mip2
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 align 4
 public _D_DrawSurfaceBlock32_mip3
_D_DrawSurfaceBlock32_mip3:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx,dword ptr[_r_lightptr]
 mov eax,dword ptr[_r_numvblocks]
 mov dword ptr[sb_v],eax
 mov edi,dword ptr[_prowdestbase]
 mov esi,dword ptr[_pbasesource]
Lv_loop_mip3:
 mov eax,dword ptr[ebx]
 mov edx,dword ptr[4+ebx]
 mov ebp,eax
 mov ecx,dword ptr[_r_lightwidth]
 mov dword ptr[_lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx,dword ptr[ebx+ecx*4]
 mov dword ptr[_lightdelta],ebp
 mov dword ptr[_r_lightptr],ebx
 mov ecx,dword ptr[4+ebx]
 mov ebx,dword ptr[ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,1
 sar ebx,1
 mov dword ptr[_lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 sar ebp,1
 or ebx,0F0000000h
 mov dword ptr[_lightdeltastep],ebx
 sub ebx,ebx
 mov bl,byte ptr[1+esi]
 sub ecx,ecx
 mov bh,dh
 mov cl,byte ptr[esi]
 mov eax,dword ptr[12345678h+ebx*4]
LPatch16:
 add edx,ebp
 mov dword ptr[4+edi],eax
 mov ch,dh
 mov eax,dword ptr[12345678h+ecx*4]
LPatch17:
 mov edx,dword ptr[_lightright]
 mov dword ptr[edi],eax
 mov eax,dword ptr[_sourcetstep]
 add esi,eax
 mov eax,dword ptr[_surfrowbytes]
 add edi,eax
 mov eax,dword ptr[_lightdeltastep]
 mov ebp,dword ptr[_lightdelta]
 mov cl,byte ptr[esi]
 add ebp,eax
 mov eax,dword ptr[_lightrightstep]
 sar ebp,1
 add edx,eax
 mov bh,dh
 mov bl,byte ptr[1+esi]
 mov eax,dword ptr[12345678h+ebx*4]
LPatch30:
 add edx,ebp
 mov dword ptr[4+edi],eax
 mov ch,dh
 mov eax,dword ptr[12345678h+ecx*4]
LPatch31:
 mov edx,dword ptr[_sourcetstep]
 mov dword ptr[edi],eax
 mov ebp,dword ptr[_surfrowbytes]
 add esi,edx
 add edi,ebp
 cmp esi,dword ptr[_r_sourcemax]
 jb LSkip_mip3
 sub esi,dword ptr[_r_stepback]
LSkip_mip3:
 mov ebx,dword ptr[_r_lightptr]
 dec dword ptr[sb_v]
 jnz Lv_loop_mip3
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 align 4
 public _D_DrawSurfaceBlock32RGB_mip0
_D_DrawSurfaceBlock32RGB_mip0:
 push ebp
 push edi
 push esi
 push ebx
 mov esi,dword ptr[_pbasesource]
 mov eax,dword ptr[_r_numvblocks]
 mov edi,dword ptr[_prowdestbase]
 mov dword ptr[sb_v],eax
Lv_loop_RGBmip0:
 mov ebx,dword ptr[_r_lightwidth]
 mov edx,dword ptr[_r_lightptrr]
 mov eax,dword ptr[edx]
 mov ecx,dword ptr[4+edx]
 lea edx,dword ptr[edx+ebx*4]
 mov dword ptr[_lightrleft],eax
 mov dword ptr[_lightrright],ecx
 mov dword ptr[_r_lightptrr],edx
 mov ebp,dword ptr[edx]
 sub ebp,eax
 mov eax,ebp
 shr eax,4
 mov dword ptr[_lightrleftstep],eax
 mov eax,dword ptr[4+edx]
 mov edx,dword ptr[_r_lightptrg]
 sub eax,ecx
 shr eax,4
 mov dword ptr[_lightrrightstep],eax
 mov eax,dword ptr[edx]
 mov ecx,dword ptr[4+edx]
 mov dword ptr[_lightgleft],eax
 mov dword ptr[_lightgright],ecx
 lea edx,dword ptr[edx+ebx*4]
 mov dword ptr[_r_lightptrg],edx
 mov ebp,dword ptr[edx]
 sub ebp,eax
 mov eax,ebp
 shr eax,4
 mov dword ptr[_lightgleftstep],eax
 mov eax,dword ptr[4+edx]
 mov edx,dword ptr[_r_lightptrb]
 sub eax,ecx
 shr eax,4
 mov dword ptr[_lightgrightstep],eax
 mov eax,dword ptr[edx]
 mov ecx,dword ptr[4+edx]
 mov dword ptr[_lightbleft],eax
 mov dword ptr[_lightbright],ecx
 lea edx,dword ptr[edx+ebx*4]
 mov dword ptr[_r_lightptrb],edx
 mov ebp,dword ptr[edx]
 sub ebp,eax
 mov eax,ebp
 shr eax,4
 mov dword ptr[_lightbleftstep],eax
 mov eax,dword ptr[4+edx]
 sub eax,ecx
 shr eax,4
 mov dword ptr[_lightbrightstep],eax
 mov dword ptr[counttemp],16
 xor ebx,ebx
 xor ecx,ecx
Lblockloop_RGBmip0:
 mov edx,dword ptr[_lightrright]
 mov ebp,dword ptr[_lightrleft]
 sub ebp,edx
 mov bl,byte ptr[15+esi]
 sar ebp,4
 mov cl,byte ptr[14+esi]
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edi,dword ptr[_roffs]
 mov al,byte ptr[12345678h+ebx]
LRPatch0:
 mov bl,byte ptr[13+esi]
 mov byte ptr[60+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LRPatch1:
 mov cl,byte ptr[12+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[56+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LRPatch2:
 mov bl,byte ptr[11+esi]
 mov byte ptr[52+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LRPatch3:
 mov cl,byte ptr[10+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[48+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LRPatch4:
 mov bl,byte ptr[9+esi]
 mov byte ptr[44+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LRPatch5:
 mov cl,byte ptr[8+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[40+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LRPatch6:
 mov bl,byte ptr[7+esi]
 mov byte ptr[36+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LRPatch7:
 mov cl,byte ptr[6+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[32+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LRPatch8:
 mov bl,byte ptr[5+esi]
 mov byte ptr[28+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LRPatch9:
 mov cl,byte ptr[4+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[24+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LRPatch10:
 mov bl,byte ptr[3+esi]
 mov byte ptr[20+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LRPatch11:
 mov cl,byte ptr[2+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[16+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LRPatch12:
 mov bl,byte ptr[1+esi]
 mov byte ptr[12+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LRPatch13:
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[8+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LRPatch14:
 mov edx,dword ptr[_lightgright]
 mov byte ptr[4+edi],al
 mov ebp,dword ptr[_lightgleft]
 mov al,byte ptr[12345678h+ecx]
LRPatch15:
 sub ebp,edx
 mov byte ptr[edi],al
 mov bl,byte ptr[15+esi]
 sub edi,dword ptr[_roffs]
 add edi,dword ptr[_goffs]
 sar ebp,4
 mov cl,byte ptr[14+esi]
 mov bh,dh
 add edx,ebp
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch0:
 mov bl,byte ptr[13+esi]
 mov byte ptr[60+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LGPatch1:
 mov cl,byte ptr[12+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[56+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch2:
 mov bl,byte ptr[11+esi]
 mov byte ptr[52+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LGPatch3:
 mov cl,byte ptr[10+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[48+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch4:
 mov bl,byte ptr[9+esi]
 mov byte ptr[44+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LGPatch5:
 mov cl,byte ptr[8+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[40+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch6:
 mov bl,byte ptr[7+esi]
 mov byte ptr[36+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LGPatch7:
 mov cl,byte ptr[6+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[32+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch8:
 mov bl,byte ptr[5+esi]
 mov byte ptr[28+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LGPatch9:
 mov cl,byte ptr[4+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[24+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch10:
 mov bl,byte ptr[3+esi]
 mov byte ptr[20+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LGPatch11:
 mov cl,byte ptr[2+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[16+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch12:
 mov bl,byte ptr[1+esi]
 mov byte ptr[12+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LGPatch13:
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[8+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch14:
 mov edx,dword ptr[_lightbright]
 mov byte ptr[4+edi],al
 mov ebp,dword ptr[_lightbleft]
 mov al,byte ptr[12345678h+ecx]
LGPatch15:
 sub ebp,edx
 mov byte ptr[edi],al
 sub edi,dword ptr[_goffs]
 add edi,dword ptr[_boffs]
 sar ebp,4
 mov bl,byte ptr[15+esi]
 mov bh,dh
 add edx,ebp
 mov ch,dh
 mov cl,byte ptr[14+esi]
 mov al,byte ptr[12345678h+ebx]
LBPatch0:
 mov bl,byte ptr[13+esi]
 mov byte ptr[60+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LBPatch1:
 mov cl,byte ptr[12+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[56+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LBPatch2:
 mov bl,byte ptr[11+esi]
 mov byte ptr[52+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LBPatch3:
 mov cl,byte ptr[10+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[48+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LBPatch4:
 mov bl,byte ptr[9+esi]
 mov byte ptr[44+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LBPatch5:
 mov cl,byte ptr[8+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[40+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LBPatch6:
 mov bl,byte ptr[7+esi]
 mov byte ptr[36+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LBPatch7:
 mov cl,byte ptr[6+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[32+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LBPatch8:
 mov bl,byte ptr[5+esi]
 mov byte ptr[28+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LBPatch9:
 mov cl,byte ptr[4+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[24+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LBPatch10:
 mov bl,byte ptr[3+esi]
 mov byte ptr[20+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LBPatch11:
 mov cl,byte ptr[2+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[16+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LBPatch12:
 mov bl,byte ptr[1+esi]
 mov byte ptr[12+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LBPatch13:
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[8+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LBPatch14:
 mov edx,dword ptr[_lightrrightstep]
 mov byte ptr[4+edi],al
 add dword ptr[_lightrright],edx
 mov al,byte ptr[12345678h+ecx]
LBPatch15:
 mov edx,dword ptr[_lightrleftstep]
 mov byte ptr[edi],al
 add edi,dword ptr[_surfrowbytes]
 sub edi,dword ptr[_boffs]
 add dword ptr[_lightrleft],edx
 mov eax,dword ptr[_lightgrightstep]
 add dword ptr[_lightgright],eax
 mov eax,dword ptr[_lightgleftstep]
 add dword ptr[_lightgleft],eax
 mov eax,dword ptr[_lightbrightstep]
 add dword ptr[_lightbright],eax
 mov eax,dword ptr[_lightbleftstep]
 add dword ptr[_lightbleft],eax
 add esi,dword ptr[_sourcetstep]
 dec dword ptr[counttemp]
 jnz Lblockloop_RGBmip0
 cmp esi,dword ptr[_r_sourcemax]
 jb LSkip_RGBmip0
 sub esi,dword ptr[_r_stepback]
LSkip_RGBmip0:
 dec dword ptr[sb_v]
 jnz Lv_loop_RGBmip0
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 align 4
 public _D_DrawSurfaceBlock32RGB_mip1
_D_DrawSurfaceBlock32RGB_mip1:
 push ebp
 push edi
 push esi
 push ebx
 mov esi,dword ptr[_pbasesource]
 mov eax,dword ptr[_r_numvblocks]
 mov edi,dword ptr[_prowdestbase]
 mov dword ptr[sb_v],eax
Lv_loop_RGBmip1:
 mov ebx,dword ptr[_r_lightwidth]
 mov edx,dword ptr[_r_lightptrr]
 mov eax,dword ptr[edx]
 mov ecx,dword ptr[4+edx]
 lea edx,dword ptr[edx+ebx*4]
 mov dword ptr[_lightrleft],eax
 mov dword ptr[_lightrright],ecx
 mov dword ptr[_r_lightptrr],edx
 mov ebp,dword ptr[edx]
 sub ebp,eax
 mov eax,ebp
 shr eax,3
 mov dword ptr[_lightrleftstep],eax
 mov eax,dword ptr[4+edx]
 mov edx,dword ptr[_r_lightptrg]
 sub eax,ecx
 shr eax,3
 mov dword ptr[_lightrrightstep],eax
 mov eax,dword ptr[edx]
 mov ecx,dword ptr[4+edx]
 mov dword ptr[_lightgleft],eax
 mov dword ptr[_lightgright],ecx
 lea edx,dword ptr[edx+ebx*4]
 mov dword ptr[_r_lightptrg],edx
 mov ebp,dword ptr[edx]
 sub ebp,eax
 mov eax,ebp
 shr eax,3
 mov dword ptr[_lightgleftstep],eax
 mov eax,dword ptr[4+edx]
 mov edx,dword ptr[_r_lightptrb]
 sub eax,ecx
 shr eax,3
 mov dword ptr[_lightgrightstep],eax
 mov eax,dword ptr[edx]
 mov ecx,dword ptr[4+edx]
 mov dword ptr[_lightbleft],eax
 mov dword ptr[_lightbright],ecx
 lea edx,dword ptr[edx+ebx*4]
 mov dword ptr[_r_lightptrb],edx
 mov ebp,dword ptr[edx]
 sub ebp,eax
 mov eax,ebp
 shr eax,3
 mov dword ptr[_lightbleftstep],eax
 mov eax,dword ptr[4+edx]
 sub eax,ecx
 shr eax,3
 mov dword ptr[_lightbrightstep],eax
 mov dword ptr[counttemp],8
 xor ebx,ebx
 xor ecx,ecx
Lblockloop_RGBmip1:
 mov edx,dword ptr[_lightrright]
 mov ebp,dword ptr[_lightrleft]
 sub ebp,edx
 mov bl,byte ptr[7+esi]
 sar ebp,3
 mov cl,byte ptr[6+esi]
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edi,dword ptr[_roffs]
 mov al,byte ptr[12345678h+ebx]
LRPatch16:
 mov bl,byte ptr[5+esi]
 mov byte ptr[28+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LRPatch17:
 mov cl,byte ptr[4+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[24+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LRPatch18:
 mov bl,byte ptr[3+esi]
 mov byte ptr[20+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LRPatch19:
 mov cl,byte ptr[2+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[16+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LRPatch20:
 mov bl,byte ptr[1+esi]
 mov byte ptr[12+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LRPatch21:
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[8+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LRPatch22:
 mov edx,dword ptr[_lightgright]
 mov byte ptr[4+edi],al
 mov ebp,dword ptr[_lightgleft]
 mov al,byte ptr[12345678h+ecx]
LRPatch23:
 sub ebp,edx
 mov byte ptr[edi],al
 mov bl,byte ptr[7+esi]
 sub edi,dword ptr[_roffs]
 add edi,dword ptr[_goffs]
 sar ebp,3
 mov cl,byte ptr[6+esi]
 mov bh,dh
 add edx,ebp
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch16:
 mov bl,byte ptr[5+esi]
 mov byte ptr[28+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LGPatch17:
 mov cl,byte ptr[4+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[24+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch18:
 mov bl,byte ptr[3+esi]
 mov byte ptr[20+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LGPatch19:
 mov cl,byte ptr[2+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[16+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch20:
 mov bl,byte ptr[1+esi]
 mov byte ptr[12+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LGPatch21:
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[8+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch22:
 mov edx,dword ptr[_lightbright]
 mov byte ptr[4+edi],al
 mov ebp,dword ptr[_lightbleft]
 mov al,byte ptr[12345678h+ecx]
LGPatch23:
 sub ebp,edx
 mov byte ptr[edi],al
 sub edi,dword ptr[_goffs]
 add edi,dword ptr[_boffs]
 sar ebp,3
 mov bl,byte ptr[7+esi]
 mov bh,dh
 add edx,ebp
 mov ch,dh
 mov cl,byte ptr[6+esi]
 mov al,byte ptr[12345678h+ebx]
LBPatch16:
 mov bl,byte ptr[5+esi]
 mov byte ptr[28+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LBPatch17:
 mov cl,byte ptr[4+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[24+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LBPatch18:
 mov bl,byte ptr[3+esi]
 mov byte ptr[20+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LBPatch19:
 mov cl,byte ptr[2+esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[16+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LBPatch20:
 mov bl,byte ptr[1+esi]
 mov byte ptr[12+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LBPatch21:
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[8+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LBPatch22:
 mov edx,dword ptr[_lightrrightstep]
 mov byte ptr[4+edi],al
 add dword ptr[_lightrright],edx
 mov al,byte ptr[12345678h+ecx]
LBPatch23:
 mov edx,dword ptr[_lightrleftstep]
 mov byte ptr[edi],al
 add edi,dword ptr[_surfrowbytes]
 sub edi,dword ptr[_boffs]
 add dword ptr[_lightrleft],edx
 mov eax,dword ptr[_lightgrightstep]
 add dword ptr[_lightgright],eax
 mov eax,dword ptr[_lightgleftstep]
 add dword ptr[_lightgleft],eax
 mov eax,dword ptr[_lightbrightstep]
 add dword ptr[_lightbright],eax
 mov eax,dword ptr[_lightbleftstep]
 add dword ptr[_lightbleft],eax
 add esi,dword ptr[_sourcetstep]
 dec dword ptr[counttemp]
 jnz Lblockloop_RGBmip1
 cmp esi,dword ptr[_r_sourcemax]
 jb LSkip_RGBmip1
 sub esi,dword ptr[_r_stepback]
LSkip_RGBmip1:
 dec dword ptr[sb_v]
 jnz Lv_loop_RGBmip1
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 align 4
 public _D_DrawSurfaceBlock32RGB_mip2
_D_DrawSurfaceBlock32RGB_mip2:
 push ebp
 push edi
 push esi
 push ebx
 mov esi,dword ptr[_pbasesource]
 mov eax,dword ptr[_r_numvblocks]
 mov edi,dword ptr[_prowdestbase]
 mov dword ptr[sb_v],eax
Lv_loop_RGBmip2:
 mov ebx,dword ptr[_r_lightwidth]
 mov edx,dword ptr[_r_lightptrr]
 mov eax,dword ptr[edx]
 mov ecx,dword ptr[4+edx]
 lea edx,dword ptr[edx+ebx*4]
 mov dword ptr[_lightrleft],eax
 mov dword ptr[_lightrright],ecx
 mov dword ptr[_r_lightptrr],edx
 mov ebp,dword ptr[edx]
 sub ebp,eax
 mov eax,ebp
 shr eax,2
 mov dword ptr[_lightrleftstep],eax
 mov eax,dword ptr[4+edx]
 mov edx,dword ptr[_r_lightptrg]
 sub eax,ecx
 shr eax,2
 mov dword ptr[_lightrrightstep],eax
 mov eax,dword ptr[edx]
 mov ecx,dword ptr[4+edx]
 mov dword ptr[_lightgleft],eax
 mov dword ptr[_lightgright],ecx
 lea edx,dword ptr[edx+ebx*4]
 mov dword ptr[_r_lightptrg],edx
 mov ebp,dword ptr[edx]
 sub ebp,eax
 mov eax,ebp
 shr eax,2
 mov dword ptr[_lightgleftstep],eax
 mov eax,dword ptr[4+edx]
 mov edx,dword ptr[_r_lightptrb]
 sub eax,ecx
 shr eax,2
 mov dword ptr[_lightgrightstep],eax
 mov eax,dword ptr[edx]
 mov ecx,dword ptr[4+edx]
 mov dword ptr[_lightbleft],eax
 mov dword ptr[_lightbright],ecx
 lea edx,dword ptr[edx+ebx*4]
 mov dword ptr[_r_lightptrb],edx
 mov ebp,dword ptr[edx]
 sub ebp,eax
 mov eax,ebp
 shr eax,2
 mov dword ptr[_lightbleftstep],eax
 mov eax,dword ptr[4+edx]
 sub eax,ecx
 shr eax,2
 mov dword ptr[_lightbrightstep],eax
 mov dword ptr[counttemp],4
 xor ebx,ebx
 xor ecx,ecx
Lblockloop_RGBmip2:
 mov edx,dword ptr[_lightrright]
 mov ebp,dword ptr[_lightrleft]
 sub ebp,edx
 mov bl,byte ptr[3+esi]
 sar ebp,2
 mov cl,byte ptr[2+esi]
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edi,dword ptr[_roffs]
 mov al,byte ptr[12345678h+ebx]
LRPatch24:
 mov bl,byte ptr[1+esi]
 mov byte ptr[12+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LRPatch25:
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[8+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LRPatch26:
 mov edx,dword ptr[_lightgright]
 mov byte ptr[4+edi],al
 mov ebp,dword ptr[_lightgleft]
 mov al,byte ptr[12345678h+ecx]
LRPatch27:
 sub ebp,edx
 mov byte ptr[edi],al
 mov bl,byte ptr[3+esi]
 sub edi,dword ptr[_roffs]
 add edi,dword ptr[_goffs]
 sar ebp,2
 mov cl,byte ptr[2+esi]
 mov bh,dh
 add edx,ebp
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch24:
 mov bl,byte ptr[1+esi]
 mov byte ptr[12+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LGPatch25:
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[8+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch26:
 mov edx,dword ptr[_lightbright]
 mov byte ptr[4+edi],al
 mov ebp,dword ptr[_lightbleft]
 mov al,byte ptr[12345678h+ecx]
LGPatch27:
 sub ebp,edx
 mov byte ptr[edi],al
 sub edi,dword ptr[_goffs]
 add edi,dword ptr[_boffs]
 sar ebp,2
 mov bl,byte ptr[3+esi]
 mov bh,dh
 add edx,ebp
 mov ch,dh
 mov cl,byte ptr[2+esi]
 mov al,byte ptr[12345678h+ebx]
LBPatch24:
 mov bl,byte ptr[1+esi]
 mov byte ptr[12+edi],al
 add edx,ebp
 mov al,byte ptr[12345678h+ecx]
LBPatch25:
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov byte ptr[8+edi],al
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LBPatch26:
 mov edx,dword ptr[_lightrrightstep]
 mov byte ptr[4+edi],al
 add dword ptr[_lightrright],edx
 mov al,byte ptr[12345678h+ecx]
LBPatch27:
 mov edx,dword ptr[_lightrleftstep]
 mov byte ptr[edi],al
 add edi,dword ptr[_surfrowbytes]
 sub edi,dword ptr[_boffs]
 add dword ptr[_lightrleft],edx
 mov eax,dword ptr[_lightgrightstep]
 add dword ptr[_lightgright],eax
 mov eax,dword ptr[_lightgleftstep]
 add dword ptr[_lightgleft],eax
 mov eax,dword ptr[_lightbrightstep]
 add dword ptr[_lightbright],eax
 mov eax,dword ptr[_lightbleftstep]
 add dword ptr[_lightbleft],eax
 add esi,dword ptr[_sourcetstep]
 dec dword ptr[counttemp]
 jnz Lblockloop_RGBmip2
 cmp esi,dword ptr[_r_sourcemax]
 jb LSkip_RGBmip2
 sub esi,dword ptr[_r_stepback]
LSkip_RGBmip2:
 dec dword ptr[sb_v]
 jnz Lv_loop_RGBmip2
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 align 4
 public _D_DrawSurfaceBlock32RGB_mip3
_D_DrawSurfaceBlock32RGB_mip3:
 push ebp
 push edi
 push esi
 push ebx
 mov esi,dword ptr[_pbasesource]
 mov eax,dword ptr[_r_numvblocks]
 mov edi,dword ptr[_prowdestbase]
 mov dword ptr[sb_v],eax
Lv_loop_RGBmip3:
 mov ebx,dword ptr[_r_lightwidth]
 mov edx,dword ptr[_r_lightptrr]
 mov eax,dword ptr[edx]
 mov ecx,dword ptr[4+edx]
 lea edx,dword ptr[edx+ebx*4]
 mov dword ptr[_lightrleft],eax
 mov dword ptr[_lightrright],ecx
 mov dword ptr[_r_lightptrr],edx
 mov ebp,dword ptr[edx]
 sub ebp,eax
 mov eax,ebp
 shr eax,1
 mov dword ptr[_lightrleftstep],eax
 mov eax,dword ptr[4+edx]
 mov edx,dword ptr[_r_lightptrg]
 sub eax,ecx
 shr eax,1
 mov dword ptr[_lightrrightstep],eax
 mov eax,dword ptr[edx]
 mov ecx,dword ptr[4+edx]
 mov dword ptr[_lightgleft],eax
 mov dword ptr[_lightgright],ecx
 lea edx,dword ptr[edx+ebx*4]
 mov dword ptr[_r_lightptrg],edx
 mov ebp,dword ptr[edx]
 sub ebp,eax
 mov eax,ebp
 shr eax,1
 mov dword ptr[_lightgleftstep],eax
 mov eax,dword ptr[4+edx]
 mov edx,dword ptr[_r_lightptrb]
 sub eax,ecx
 shr eax,1
 mov dword ptr[_lightgrightstep],eax
 mov eax,dword ptr[edx]
 mov ecx,dword ptr[4+edx]
 mov dword ptr[_lightbleft],eax
 mov dword ptr[_lightbright],ecx
 lea edx,dword ptr[edx+ebx*4]
 mov dword ptr[_r_lightptrb],edx
 mov ebp,dword ptr[edx]
 sub ebp,eax
 mov eax,ebp
 shr eax,1
 mov dword ptr[_lightbleftstep],eax
 mov eax,dword ptr[4+edx]
 sub eax,ecx
 shr eax,1
 mov dword ptr[_lightbrightstep],eax
 xor ebx,ebx
 xor ecx,ecx
 mov edx,dword ptr[_lightrright]
 mov ebp,dword ptr[_lightrleft]
 sub ebp,edx
 mov bl,byte ptr[1+esi]
 sar ebp,1
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edi,dword ptr[_roffs]
 mov al,byte ptr[12345678h+ebx]
LRPatch28:
 mov edx,dword ptr[_lightgright]
 mov byte ptr[4+edi],al
 mov ebp,dword ptr[_lightgleft]
 mov al,byte ptr[12345678h+ecx]
LRPatch29:
 sub ebp,edx
 mov byte ptr[edi],al
 mov bl,byte ptr[1+esi]
 sub edi,dword ptr[_roffs]
 add edi,dword ptr[_goffs]
 sar ebp,1
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch28:
 mov edx,dword ptr[_lightbright]
 mov byte ptr[4+edi],al
 mov ebp,dword ptr[_lightbleft]
 mov al,byte ptr[12345678h+ecx]
LGPatch29:
 sub ebp,edx
 mov byte ptr[edi],al
 sub edi,dword ptr[_goffs]
 add edi,dword ptr[_boffs]
 sar ebp,1
 mov bl,byte ptr[1+esi]
 mov bh,dh
 add edx,ebp
 mov ch,dh
 mov cl,byte ptr[esi]
 mov al,byte ptr[12345678h+ebx]
LBPatch28:
 mov edx,dword ptr[_lightrrightstep]
 mov byte ptr[4+edi],al
 add dword ptr[_lightrright],edx
 mov al,byte ptr[12345678h+ecx]
LBPatch29:
 mov edx,dword ptr[_lightrleftstep]
 mov byte ptr[edi],al
 add edi,dword ptr[_surfrowbytes]
 sub edi,dword ptr[_boffs]
 add dword ptr[_lightrleft],edx
 mov eax,dword ptr[_lightgrightstep]
 add dword ptr[_lightgright],eax
 mov eax,dword ptr[_lightgleftstep]
 add dword ptr[_lightgleft],eax
 mov eax,dword ptr[_lightbrightstep]
 add dword ptr[_lightbright],eax
 mov eax,dword ptr[_lightbleftstep]
 add dword ptr[_lightbleft],eax
 add esi,dword ptr[_sourcetstep]
 mov edx,dword ptr[_lightrright]
 mov ebp,dword ptr[_lightrleft]
 sub ebp,edx
 mov bl,byte ptr[1+esi]
 sar ebp,1
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edi,dword ptr[_roffs]
 mov al,byte ptr[12345678h+ebx]
LRPatch30:
 mov edx,dword ptr[_lightgright]
 mov byte ptr[4+edi],al
 mov ebp,dword ptr[_lightgleft]
 mov al,byte ptr[12345678h+ecx]
LRPatch31:
 sub ebp,edx
 mov byte ptr[edi],al
 mov bl,byte ptr[1+esi]
 sub edi,dword ptr[_roffs]
 add edi,dword ptr[_goffs]
 sar ebp,1
 mov cl,byte ptr[esi]
 mov bh,dh
 add edx,ebp
 mov ch,dh
 mov al,byte ptr[12345678h+ebx]
LGPatch30:
 mov edx,dword ptr[_lightbright]
 mov byte ptr[4+edi],al
 mov ebp,dword ptr[_lightbleft]
 mov al,byte ptr[12345678h+ecx]
LGPatch31:
 sub ebp,edx
 mov byte ptr[edi],al
 sub edi,dword ptr[_goffs]
 add edi,dword ptr[_boffs]
 sar ebp,1
 mov bl,byte ptr[1+esi]
 mov bh,dh
 add edx,ebp
 mov ch,dh
 mov cl,byte ptr[esi]
 mov al,byte ptr[12345678h+ebx]
LBPatch30:
 mov byte ptr[4+edi],al
 mov al,byte ptr[12345678h+ecx]
LBPatch31:
 add esi,dword ptr[_sourcetstep]
 mov byte ptr[edi],al
 add edi,dword ptr[_surfrowbytes]
 sub edi,dword ptr[_boffs]
 cmp esi,dword ptr[_r_sourcemax]
 jb LSkip_RGBmip3
 sub esi,dword ptr[_r_stepback]
LSkip_RGBmip3:
 dec dword ptr[sb_v]
 jnz Lv_loop_RGBmip3
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 public _D_Surf32End
_D_Surf32End:
_TEXT ENDS
_DATA SEGMENT
 align 4
LPatchTable:
 dd LPatch0-4
 dd LPatch1-4
 dd LPatch2-4
 dd LPatch3-4
 dd LPatch4-4
 dd LPatch5-4
 dd LPatch6-4
 dd LPatch7-4
 dd LPatch8-4
 dd LPatch9-4
 dd LPatch10-4
 dd LPatch11-4
 dd LPatch12-4
 dd LPatch13-4
 dd LPatch14-4
 dd LPatch15-4
 dd LPatch16-4
 dd LPatch17-4
 dd LPatch18-4
 dd LPatch19-4
 dd LPatch20-4
 dd LPatch21-4
 dd LPatch22-4
 dd LPatch23-4
 dd LPatch24-4
 dd LPatch25-4
 dd LPatch26-4
 dd LPatch27-4
 dd LPatch28-4
 dd LPatch29-4
 dd LPatch30-4
 dd LPatch31-4
LRPatchTable:
 dd LRPatch0-4
 dd LRPatch1-4
 dd LRPatch2-4
 dd LRPatch3-4
 dd LRPatch4-4
 dd LRPatch5-4
 dd LRPatch6-4
 dd LRPatch7-4
 dd LRPatch8-4
 dd LRPatch9-4
 dd LRPatch10-4
 dd LRPatch11-4
 dd LRPatch12-4
 dd LRPatch13-4
 dd LRPatch14-4
 dd LRPatch15-4
 dd LRPatch16-4
 dd LRPatch17-4
 dd LRPatch18-4
 dd LRPatch19-4
 dd LRPatch20-4
 dd LRPatch21-4
 dd LRPatch22-4
 dd LRPatch23-4
 dd LRPatch24-4
 dd LRPatch25-4
 dd LRPatch26-4
 dd LRPatch27-4
 dd LRPatch28-4
 dd LRPatch29-4
 dd LRPatch30-4
 dd LRPatch31-4
LGPatchTable:
 dd LGPatch0-4
 dd LGPatch1-4
 dd LGPatch2-4
 dd LGPatch3-4
 dd LGPatch4-4
 dd LGPatch5-4
 dd LGPatch6-4
 dd LGPatch7-4
 dd LGPatch8-4
 dd LGPatch9-4
 dd LGPatch10-4
 dd LGPatch11-4
 dd LGPatch12-4
 dd LGPatch13-4
 dd LGPatch14-4
 dd LGPatch15-4
 dd LGPatch16-4
 dd LGPatch17-4
 dd LGPatch18-4
 dd LGPatch19-4
 dd LGPatch20-4
 dd LGPatch21-4
 dd LGPatch22-4
 dd LGPatch23-4
 dd LGPatch24-4
 dd LGPatch25-4
 dd LGPatch26-4
 dd LGPatch27-4
 dd LGPatch28-4
 dd LGPatch29-4
 dd LGPatch30-4
 dd LGPatch31-4
LBPatchTable:
 dd LBPatch0-4
 dd LBPatch1-4
 dd LBPatch2-4
 dd LBPatch3-4
 dd LBPatch4-4
 dd LBPatch5-4
 dd LBPatch6-4
 dd LBPatch7-4
 dd LBPatch8-4
 dd LBPatch9-4
 dd LBPatch10-4
 dd LBPatch11-4
 dd LBPatch12-4
 dd LBPatch13-4
 dd LBPatch14-4
 dd LBPatch15-4
 dd LBPatch16-4
 dd LBPatch17-4
 dd LBPatch18-4
 dd LBPatch19-4
 dd LBPatch20-4
 dd LBPatch21-4
 dd LBPatch22-4
 dd LBPatch23-4
 dd LBPatch24-4
 dd LBPatch25-4
 dd LBPatch26-4
 dd LBPatch27-4
 dd LBPatch28-4
 dd LBPatch29-4
 dd LBPatch30-4
 dd LBPatch31-4
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _D_Surf32Patch
_D_Surf32Patch:
 push ebx
 mov eax,dword ptr[_fadetable32]
 mov ebx,offset LPatchTable
 mov ecx,32
LPatchLoop:
 mov edx,dword ptr[ebx]
 add ebx,4
 mov dword ptr[edx],eax
 dec ecx
 jnz LPatchLoop
 mov eax,dword ptr[_fadetable32r]
 mov ebx,offset LRPatchTable
 mov ecx,32
LRPatchLoop:
 mov edx,dword ptr[ebx]
 add ebx,4
 mov dword ptr[edx],eax
 dec ecx
 jnz LRPatchLoop
 mov eax,dword ptr[_fadetable32g]
 mov ebx,offset LGPatchTable
 mov ecx,32
LGPatchLoop:
 mov edx,dword ptr[ebx]
 add ebx,4
 mov dword ptr[edx],eax
 dec ecx
 jnz LGPatchLoop
 mov eax,dword ptr[_fadetable32b]
 mov ebx,offset LBPatchTable
 mov ecx,32
LBPatchLoop:
 mov edx,dword ptr[ebx]
 add ebx,4
 mov dword ptr[edx],eax
 dec ecx
 jnz LBPatchLoop
 pop ebx
 ret
_TEXT ENDS
 END
