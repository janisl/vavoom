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
_TEXT SEGMENT
 align 4
 public _D_PolysetAff16Start
_D_PolysetAff16Start:
 public _D_PolysetDrawSpans_16
_D_PolysetDrawSpans_16:
 push esi
 push ebx
 mov esi,dword ptr[4+8+esp]
 mov ecx,dword ptr[_r_zistepx]
 push ebp
 push edi
 ror ecx,16
 movsx edx,word ptr[24+esi]
 mov dword ptr[lzistepx],ecx
LSpanLoop:
 mov eax,dword ptr[_d_aspancount]
 sub eax,edx
 mov edx,dword ptr[_erroradjustup]
 mov ebx,dword ptr[_errorterm]
 add ebx,edx
 js LNoTurnover
 mov edx,dword ptr[_erroradjustdown]
 mov edi,dword ptr[_d_countextrastep]
 sub ebx,edx
 mov ebp,dword ptr[_d_aspancount]
 mov dword ptr[_errorterm],ebx
 add ebp,edi
 mov dword ptr[_d_aspancount],ebp
 jmp LRightEdgeStepped
LNoTurnover:
 mov edi,dword ptr[_d_aspancount]
 mov edx,dword ptr[_ubasestep]
 mov dword ptr[_errorterm],ebx
 add edi,edx
 mov dword ptr[_d_aspancount],edi
LRightEdgeStepped:
 cmp eax,1
 jl LNextSpan
 jz LExactlyOneLong
 mov ecx,dword ptr[_a_ststepxwhole]
 mov edx,dword ptr[_d_affinetridesc+4]
 mov dword ptr[advancetable+4],ecx
 add ecx,edx
 mov dword ptr[advancetable],ecx
 mov ecx,dword ptr[_a_tstepxfrac]
 mov cx,word ptr[_r_rstepx]
 mov edx,eax
 mov dword ptr[tstep],ecx
 add edx,7
 shr edx,3
 mov ebx,dword ptr[12+esi]
 mov bx,dx
 mov ecx,dword ptr[4+esi]
 neg eax
 mov edi,dword ptr[0+esi]
 and eax,7
 sub edi,eax
 sub ecx,eax
 sub edi,eax
 sub ecx,eax
 mov edx,dword ptr[16+esi]
 mov dx,word ptr[26+esi]
 mov ebp,dword ptr[20+esi]
 ror ebp,16
 push esi
 mov esi,dword ptr[8+esi]
 jmp  dword ptr[Lentryvec_table+eax*4]
Lentryvec_table:
 dd LDraw8, LDraw7, LDraw6, LDraw5
 dd LDraw4, LDraw3, LDraw2, LDraw1
LDrawLoop:
LDraw8:
 cmp bp,word ptr[ecx]
 jl Lp1
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LPatch8:
 mov word ptr[edi],ax
Lp1:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LDraw7:
 cmp bp,word ptr[2+ecx]
 jl Lp2
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[2+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LPatch7:
 mov word ptr[2+edi],ax
Lp2:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LDraw6:
 cmp bp,word ptr[4+ecx]
 jl Lp3
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[4+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LPatch6:
 mov word ptr[4+edi],ax
Lp3:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LDraw5:
 cmp bp,word ptr[6+ecx]
 jl Lp4
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[6+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LPatch5:
 mov word ptr[6+edi],ax
Lp4:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LDraw4:
 cmp bp,word ptr[8+ecx]
 jl Lp5
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[8+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LPatch4:
 mov word ptr[8+edi],ax
Lp5:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LDraw3:
 cmp bp,word ptr[10+ecx]
 jl Lp6
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[10+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LPatch3:
 mov word ptr[10+edi],ax
Lp6:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LDraw2:
 cmp bp,word ptr[12+ecx]
 jl Lp7
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[12+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LPatch2:
 mov word ptr[12+edi],ax
Lp7:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LDraw1:
 cmp bp,word ptr[14+ecx]
 jl Lp8
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[14+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LPatch1:
 mov word ptr[14+edi],ax
Lp8:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
 add edi,16
 add ecx,16
 dec bx
 jnz LDrawLoop
 pop esi
LNextSpan:
 add esi,32 
LNextSpanESISet:
 movsx edx,word ptr[24+esi]
 cmp edx,offset -9999
 jnz LSpanLoop
 pop edi
 pop ebp
 pop ebx
 pop esi
 ret
LExactlyOneLong:
 mov ecx,dword ptr[4+esi]
 mov ebp,dword ptr[20+esi]
 ror ebp,16
 mov ebx,dword ptr[8+esi]
 cmp bp,word ptr[ecx]
 jl LNextSpan
 xor eax,eax
 mov edi,dword ptr[0+esi]
 mov ah,byte ptr[26+1+esi]
 add esi,32 
 mov al,byte ptr[ebx]
 mov word ptr[ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LPatch9:
 mov word ptr[edi],ax
 jmp LNextSpanESISet
 public _D_PolysetDrawSpansRGB_16
_D_PolysetDrawSpansRGB_16:
 push esi
 push ebx
 mov esi,dword ptr[4+8+esp]
 mov ecx,dword ptr[_r_zistepx]
 push ebp
 push edi
 ror ecx,16
 movsx edx,word ptr[24+esi]
 mov dword ptr[lzistepx],ecx
LRGBSpanLoop:
 mov eax,dword ptr[_d_aspancount]
 sub eax,edx
 mov edx,dword ptr[_erroradjustup]
 mov ebx,dword ptr[_errorterm]
 add ebx,edx
 js LRGBNoTurnover
 mov edx,dword ptr[_erroradjustdown]
 mov edi,dword ptr[_d_countextrastep]
 sub ebx,edx
 mov ebp,dword ptr[_d_aspancount]
 mov dword ptr[_errorterm],ebx
 add ebp,edi
 mov dword ptr[_d_aspancount],ebp
 jmp LRGBRightEdgeStepped
LRGBNoTurnover:
 mov edi,dword ptr[_d_aspancount]
 mov edx,dword ptr[_ubasestep]
 mov dword ptr[_errorterm],ebx
 add edi,edx
 mov dword ptr[_d_aspancount],edi
LRGBRightEdgeStepped:
 cmp eax,1
 jl LRGBNextSpan
 jz LRGBExactlyOneLong
 mov ecx,dword ptr[_a_ststepxwhole]
 mov edx,dword ptr[_d_affinetridesc+4]
 mov dword ptr[advancetable+4],ecx
 add ecx,edx
 mov dword ptr[advancetable],ecx
 mov ecx,dword ptr[_a_tstepxfrac]
 mov cx,word ptr[_r_rstepx]
 mov edx,eax
 mov dword ptr[tstep],ecx
 add edx,7
 shr edx,3
 mov ebx,dword ptr[12+esi]
 mov bx,dx
 mov ecx,dword ptr[4+esi]
 neg eax
 mov edi,dword ptr[0+esi]
 and eax,7
 sub edi,eax
 sub ecx,eax
 sub edi,eax
 mov edx,dword ptr[28+esi]
 mov dword ptr[gb],edx
 mov edx,dword ptr[_r_bstepx]
 shr edx,16
 mov dx,word ptr[_r_gstepx]
 mov dword ptr[gbstep],edx
 sub ecx,eax
 mov edx,dword ptr[16+esi]
 mov dx,word ptr[26+esi]
 mov ebp,dword ptr[20+esi]
 ror ebp,16
 push esi
 mov esi,dword ptr[8+esi]
 jmp  dword ptr[LRGBentryvec_table+eax*4]
LRGBentryvec_table:
 dd LRGBDraw8, LRGBDraw7, LRGBDraw6, LRGBDraw5
 dd LRGBDraw4, LRGBDraw3, LRGBDraw2, LRGBDraw1
LRGBDrawLoop:
LRGBDraw8:
 cmp bp,word ptr[ecx]
 jl LRGBp1
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRPatch8:
 mov word ptr[edi],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch8:
 or word ptr[edi],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch8:
 or word ptr[edi],ax
LRGBp1:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBDraw7:
 cmp bp,word ptr[2+ecx]
 jl LRGBp2
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[2+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRPatch7:
 mov word ptr[2+edi],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch7:
 or word ptr[2+edi],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch7:
 or word ptr[2+edi],ax
LRGBp2:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBDraw6:
 cmp bp,word ptr[4+ecx]
 jl LRGBp3
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[4+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRPatch6:
 mov word ptr[4+edi],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch6:
 or word ptr[4+edi],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch6:
 or word ptr[4+edi],ax
LRGBp3:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBDraw5:
 cmp bp,word ptr[6+ecx]
 jl LRGBp4
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[6+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRPatch5:
 mov word ptr[6+edi],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch5:
 or word ptr[6+edi],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch5:
 or word ptr[6+edi],ax
LRGBp4:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBDraw4:
 cmp bp,word ptr[8+ecx]
 jl LRGBp5
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[8+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRPatch4:
 mov word ptr[8+edi],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch4:
 or word ptr[8+edi],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch4:
 or word ptr[8+edi],ax
LRGBp5:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBDraw3:
 cmp bp,word ptr[10+ecx]
 jl LRGBp6
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[10+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRPatch3:
 mov word ptr[10+edi],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch3:
 or word ptr[10+edi],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch3:
 or word ptr[10+edi],ax
LRGBp6:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBDraw2:
 cmp bp,word ptr[12+ecx]
 jl LRGBp7
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[12+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRPatch2:
 mov word ptr[12+edi],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch2:
 or word ptr[12+edi],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch2:
 or word ptr[12+edi],ax
LRGBp7:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBDraw1:
 cmp bp,word ptr[14+ecx]
 jl LRGBp8
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[14+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRPatch1:
 mov word ptr[14+edi],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch1:
 or word ptr[14+edi],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch1:
 or word ptr[14+edi],ax
LRGBp8:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
 add edi,16
 add ecx,16
 dec bx
 jnz LRGBDrawLoop
 pop esi
LRGBNextSpan:
 add esi,32 
LRGBNextSpanESISet:
 movsx edx,word ptr[24+esi]
 cmp edx,offset -9999
 jnz LRGBSpanLoop
 pop edi
 pop ebp
 pop ebx
 pop esi
 ret
LRGBExactlyOneLong:
 mov ecx,dword ptr[4+esi]
 mov ebp,dword ptr[20+esi]
 ror ebp,16
 mov ebx,dword ptr[8+esi]
 cmp bp,word ptr[ecx]
 jl LRGBNextSpan
 xor eax,eax
 mov edi,dword ptr[0+esi]
 mov ah,byte ptr[26+1+esi]
 mov al,byte ptr[ebx]
 mov word ptr[ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRPatch9:
 mov word ptr[edi],ax
 mov ah,byte ptr[28+1+esi]
 mov al,byte ptr[ebx]
 mov ax,word ptr[12345678h+eax*2]
LGPatch9:
 or word ptr[edi],ax
 mov ah,byte ptr[30+1+esi]
 add esi,32 
 mov al,byte ptr[ebx]
 mov ax,word ptr[12345678h+eax*2]
LBPatch9:
 or word ptr[edi],ax
 jmp LRGBNextSpanESISet
 public _D_PolysetAff16End
_D_PolysetAff16End:
 public _D_Aff16Patch
_D_Aff16Patch:
 mov eax,dword ptr[_fadetable16]
 mov dword ptr[LPatch1-4],eax
 mov dword ptr[LPatch2-4],eax
 mov dword ptr[LPatch3-4],eax
 mov dword ptr[LPatch4-4],eax
 mov dword ptr[LPatch5-4],eax
 mov dword ptr[LPatch6-4],eax
 mov dword ptr[LPatch7-4],eax
 mov dword ptr[LPatch8-4],eax
 mov dword ptr[LPatch9-4],eax
 mov eax,dword ptr[_fadetable16r]
 mov dword ptr[LRPatch1-4],eax
 mov dword ptr[LRPatch2-4],eax
 mov dword ptr[LRPatch3-4],eax
 mov dword ptr[LRPatch4-4],eax
 mov dword ptr[LRPatch5-4],eax
 mov dword ptr[LRPatch6-4],eax
 mov dword ptr[LRPatch7-4],eax
 mov dword ptr[LRPatch8-4],eax
 mov dword ptr[LRPatch9-4],eax
 mov eax,dword ptr[_fadetable16g]
 mov dword ptr[LGPatch1-4],eax
 mov dword ptr[LGPatch2-4],eax
 mov dword ptr[LGPatch3-4],eax
 mov dword ptr[LGPatch4-4],eax
 mov dword ptr[LGPatch5-4],eax
 mov dword ptr[LGPatch6-4],eax
 mov dword ptr[LGPatch7-4],eax
 mov dword ptr[LGPatch8-4],eax
 mov dword ptr[LGPatch9-4],eax
 mov eax,dword ptr[_fadetable16b]
 mov dword ptr[LBPatch1-4],eax
 mov dword ptr[LBPatch2-4],eax
 mov dword ptr[LBPatch3-4],eax
 mov dword ptr[LBPatch4-4],eax
 mov dword ptr[LBPatch5-4],eax
 mov dword ptr[LBPatch6-4],eax
 mov dword ptr[LBPatch7-4],eax
 mov dword ptr[LBPatch8-4],eax
 mov dword ptr[LBPatch9-4],eax
 ret
_TEXT ENDS
 END
