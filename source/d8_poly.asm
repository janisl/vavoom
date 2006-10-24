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
 externdef DP_Colour
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
 externdef _centrexfrac
 externdef _centreyfrac
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
 externdef _aliasxcentre
 externdef _aliasycentre
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
Ltemp dd 0
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _D_PolysetAff8Start
_D_PolysetAff8Start:
 public _D_PolysetDrawSpans_8
_D_PolysetDrawSpans_8:
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
 mov al,byte ptr[12345678h+eax]
LPatch8:
 mov byte ptr[edi],al
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
 mov al,byte ptr[12345678h+eax]
LPatch7:
 mov byte ptr[1+edi],al
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
 mov al,byte ptr[12345678h+eax]
LPatch6:
 mov byte ptr[2+edi],al
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
 mov al,byte ptr[12345678h+eax]
LPatch5:
 mov byte ptr[3+edi],al
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
 mov al,byte ptr[12345678h+eax]
LPatch4:
 mov byte ptr[4+edi],al
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
 mov al,byte ptr[12345678h+eax]
LPatch3:
 mov byte ptr[5+edi],al
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
 mov al,byte ptr[12345678h+eax]
LPatch2:
 mov byte ptr[6+edi],al
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
 mov al,byte ptr[12345678h+eax]
LPatch1:
 mov byte ptr[7+edi],al
Lp8:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
 add edi,8
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
 mov al,byte ptr[12345678h+eax]
LPatch9:
 mov byte ptr[edi],al
 jmp LNextSpanESISet
 public _D_PolysetDrawSpansFuzz_8
_D_PolysetDrawSpansFuzz_8:
 push esi
 push ebx
 mov esi,dword ptr[4+8+esp]
 mov ecx,dword ptr[_r_zistepx]
 push ebp
 push edi
 ror ecx,16
 movsx edx,word ptr[24+esi]
 mov dword ptr[lzistepx],ecx
LFuzzSpanLoop:
 mov eax,dword ptr[_d_aspancount]
 sub eax,edx
 mov edx,dword ptr[_erroradjustup]
 mov ebx,dword ptr[_errorterm]
 add ebx,edx
 js LFuzzNoTurnover
 mov edx,dword ptr[_erroradjustdown]
 mov edi,dword ptr[_d_countextrastep]
 sub ebx,edx
 mov ebp,dword ptr[_d_aspancount]
 mov dword ptr[_errorterm],ebx
 add ebp,edi
 mov dword ptr[_d_aspancount],ebp
 jmp LFuzzRightEdgeStepped
LFuzzNoTurnover:
 mov edi,dword ptr[_d_aspancount]
 mov edx,dword ptr[_ubasestep]
 mov dword ptr[_errorterm],ebx
 add edi,edx
 mov dword ptr[_d_aspancount],edi
LFuzzRightEdgeStepped:
 cmp eax,1
 jl LFuzzNextSpan
 jz LFuzzExactlyOneLong
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
 sub ecx,eax
 mov edx,dword ptr[16+esi]
 mov dx,word ptr[26+esi]
 mov ebp,dword ptr[20+esi]
 ror ebp,16
 push esi
 mov esi,dword ptr[8+esi]
 jmp  dword ptr[LFuzzentryvec_table+eax*4]
LFuzzentryvec_table:
 dd LFuzzDraw8, LFuzzDraw7, LFuzzDraw6, LFuzzDraw5
 dd LFuzzDraw4, LFuzzDraw3, LFuzzDraw2, LFuzzDraw1
LFuzzDrawLoop:
LFuzzDraw8:
 cmp bp,word ptr[ecx]
 jl LFuzzp1
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[ecx],bp
 mov ah,byte ptr[12345678h+eax]
LFuzzPatch8:
 mov al,byte ptr[edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[edi],al
LFuzzp1:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LFuzzDraw7:
 cmp bp,word ptr[2+ecx]
 jl LFuzzp2
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[2+ecx],bp
 mov ah,byte ptr[12345678h+eax]
LFuzzPatch7:
 mov al,byte ptr[1+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[1+edi],al
LFuzzp2:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LFuzzDraw6:
 cmp bp,word ptr[4+ecx]
 jl LFuzzp3
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[4+ecx],bp
 mov ah,byte ptr[12345678h+eax]
LFuzzPatch6:
 mov al,byte ptr[2+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[2+edi],al
LFuzzp3:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LFuzzDraw5:
 cmp bp,word ptr[6+ecx]
 jl LFuzzp4
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[6+ecx],bp
 mov ah,byte ptr[12345678h+eax]
LFuzzPatch5:
 mov al,byte ptr[3+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[3+edi],al
LFuzzp4:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LFuzzDraw4:
 cmp bp,word ptr[8+ecx]
 jl LFuzzp5
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[8+ecx],bp
 mov ah,byte ptr[12345678h+eax]
LFuzzPatch4:
 mov al,byte ptr[4+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[4+edi],al
LFuzzp5:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LFuzzDraw3:
 cmp bp,word ptr[10+ecx]
 jl LFuzzp6
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[10+ecx],bp
 mov ah,byte ptr[12345678h+eax]
LFuzzPatch3:
 mov al,byte ptr[5+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[5+edi],al
LFuzzp6:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LFuzzDraw2:
 cmp bp,word ptr[12+ecx]
 jl LFuzzp7
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[12+ecx],bp
 mov ah,byte ptr[12345678h+eax]
LFuzzPatch2:
 mov al,byte ptr[6+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[6+edi],al
LFuzzp7:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LFuzzDraw1:
 cmp bp,word ptr[14+ecx]
 jl LFuzzp8
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[14+ecx],bp
 mov ah,byte ptr[12345678h+eax]
LFuzzPatch1:
 mov al,byte ptr[7+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[7+edi],al
LFuzzp8:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
 add edi,8
 add ecx,16
 dec bx
 jnz LFuzzDrawLoop
 pop esi
LFuzzNextSpan:
 add esi,32
LFuzzNextSpanESISet:
 movsx edx,word ptr[24+esi]
 cmp edx,offset -9999
 jnz LFuzzSpanLoop
 pop edi
 pop ebp
 pop ebx
 pop esi
 ret
LFuzzExactlyOneLong:
 mov ecx,dword ptr[4+esi]
 mov ebp,dword ptr[20+esi]
 ror ebp,16
 mov ebx,dword ptr[8+esi]
 cmp bp,word ptr[ecx]
 jl LFuzzNextSpan
 xor eax,eax
 mov edi,dword ptr[0+esi]
 mov ah,byte ptr[26+1+esi]
 add esi,32
 mov al,byte ptr[ebx]
 mov word ptr[ecx],bp
 mov ah,byte ptr[12345678h+eax]
LFuzzPatch9:
 mov al,byte ptr[edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[edi],al
 jmp LFuzzNextSpanESISet
 public _D_PolysetDrawSpansAltFuzz_8
_D_PolysetDrawSpansAltFuzz_8:
 push esi
 push ebx
 mov esi,dword ptr[4+8+esp]
 mov ecx,dword ptr[_r_zistepx]
 push ebp
 push edi
 ror ecx,16
 movsx edx,word ptr[24+esi]
 mov dword ptr[lzistepx],ecx
LAltFuzzSpanLoop:
 mov eax,dword ptr[_d_aspancount]
 sub eax,edx
 mov edx,dword ptr[_erroradjustup]
 mov ebx,dword ptr[_errorterm]
 add ebx,edx
 js LAltFuzzNoTurnover
 mov edx,dword ptr[_erroradjustdown]
 mov edi,dword ptr[_d_countextrastep]
 sub ebx,edx
 mov ebp,dword ptr[_d_aspancount]
 mov dword ptr[_errorterm],ebx
 add ebp,edi
 mov dword ptr[_d_aspancount],ebp
 jmp LAltFuzzRightEdgeStepped
LAltFuzzNoTurnover:
 mov edi,dword ptr[_d_aspancount]
 mov edx,dword ptr[_ubasestep]
 mov dword ptr[_errorterm],ebx
 add edi,edx
 mov dword ptr[_d_aspancount],edi
LAltFuzzRightEdgeStepped:
 cmp eax,1
 jl LAltFuzzNextSpan
 jz LAltFuzzExactlyOneLong
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
 sub ecx,eax
 mov edx,dword ptr[16+esi]
 mov dx,word ptr[26+esi]
 mov ebp,dword ptr[20+esi]
 ror ebp,16
 push esi
 mov esi,dword ptr[8+esi]
 jmp  dword ptr[LAltFuzzentryvec_table+eax*4]
LAltFuzzentryvec_table:
 dd LAltFuzzDraw8, LAltFuzzDraw7, LAltFuzzDraw6, LAltFuzzDraw5
 dd LAltFuzzDraw4, LAltFuzzDraw3, LAltFuzzDraw2, LAltFuzzDraw1
LAltFuzzDrawLoop:
LAltFuzzDraw8:
 cmp bp,word ptr[ecx]
 jl LAltFuzzp1
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[ecx],bp
 mov al,byte ptr[12345678h+eax]
LAltFuzzPatch8:
 mov ah,byte ptr[edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[edi],al
LAltFuzzp1:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LAltFuzzDraw7:
 cmp bp,word ptr[2+ecx]
 jl LAltFuzzp2
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[2+ecx],bp
 mov al,byte ptr[12345678h+eax]
LAltFuzzPatch7:
 mov ah,byte ptr[1+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[1+edi],al
LAltFuzzp2:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LAltFuzzDraw6:
 cmp bp,word ptr[4+ecx]
 jl LAltFuzzp3
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[4+ecx],bp
 mov al,byte ptr[12345678h+eax]
LAltFuzzPatch6:
 mov ah,byte ptr[2+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[2+edi],al
LAltFuzzp3:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LAltFuzzDraw5:
 cmp bp,word ptr[6+ecx]
 jl LAltFuzzp4
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[6+ecx],bp
 mov al,byte ptr[12345678h+eax]
LAltFuzzPatch5:
 mov ah,byte ptr[3+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[3+edi],al
LAltFuzzp4:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LAltFuzzDraw4:
 cmp bp,word ptr[8+ecx]
 jl LAltFuzzp5
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[8+ecx],bp
 mov al,byte ptr[12345678h+eax]
LAltFuzzPatch4:
 mov ah,byte ptr[4+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[4+edi],al
LAltFuzzp5:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LAltFuzzDraw3:
 cmp bp,word ptr[10+ecx]
 jl LAltFuzzp6
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[10+ecx],bp
 mov al,byte ptr[12345678h+eax]
LAltFuzzPatch3:
 mov ah,byte ptr[5+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[5+edi],al
LAltFuzzp6:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LAltFuzzDraw2:
 cmp bp,word ptr[12+ecx]
 jl LAltFuzzp7
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[12+ecx],bp
 mov al,byte ptr[12345678h+eax]
LAltFuzzPatch2:
 mov ah,byte ptr[6+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[6+edi],al
LAltFuzzp7:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LAltFuzzDraw1:
 cmp bp,word ptr[14+ecx]
 jl LAltFuzzp8
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[14+ecx],bp
 mov al,byte ptr[12345678h+eax]
LAltFuzzPatch1:
 mov ah,byte ptr[7+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[7+edi],al
LAltFuzzp8:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
 add edi,8
 add ecx,16
 dec bx
 jnz LAltFuzzDrawLoop
 pop esi
LAltFuzzNextSpan:
 add esi,32
LAltFuzzNextSpanESISet:
 movsx edx,word ptr[24+esi]
 cmp edx,offset -9999
 jnz LAltFuzzSpanLoop
 pop edi
 pop ebp
 pop ebx
 pop esi
 ret
LAltFuzzExactlyOneLong:
 mov ecx,dword ptr[4+esi]
 mov ebp,dword ptr[20+esi]
 ror ebp,16
 mov ebx,dword ptr[8+esi]
 cmp bp,word ptr[ecx]
 jl LAltFuzzNextSpan
 xor eax,eax
 mov edi,dword ptr[0+esi]
 mov ah,byte ptr[26+1+esi]
 add esi,32
 mov al,byte ptr[ebx]
 mov word ptr[ecx],bp
 mov al,byte ptr[12345678h+eax]
LAltFuzzPatch9:
 mov ah,byte ptr[edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[edi],al
 jmp LAltFuzzNextSpanESISet
 public _D_PolysetDrawSpansRGB_8
_D_PolysetDrawSpansRGB_8:
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
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch8:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch8:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTablePatch8:
 mov byte ptr[edi],al
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
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch7:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch7:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTablePatch7:
 mov byte ptr[1+edi],al
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
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch6:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch6:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTablePatch6:
 mov byte ptr[2+edi],al
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
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch5:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch5:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTablePatch5:
 mov byte ptr[3+edi],al
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
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch4:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch4:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTablePatch4:
 mov byte ptr[4+edi],al
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
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch3:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch3:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTablePatch3:
 mov byte ptr[5+edi],al
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
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch2:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch2:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTablePatch2:
 mov byte ptr[6+edi],al
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
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGPatch1:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBPatch1:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTablePatch1:
 mov byte ptr[7+edi],al
LRGBp8:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
 add edi,8
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
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[28+1+esi]
 mov al,byte ptr[ebx]
 mov ax,word ptr[12345678h+eax*2]
LGPatch9:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[30+1+esi]
 add esi,32
 mov al,byte ptr[ebx]
 mov ax,word ptr[12345678h+eax*2]
LBPatch9:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTablePatch9:
 mov byte ptr[edi],al
 jmp LRGBNextSpanESISet
 public _D_PolysetDrawSpansRGBFuzz_8
_D_PolysetDrawSpansRGBFuzz_8:
 push esi
 push ebx
 mov esi,dword ptr[4+8+esp]
 mov ecx,dword ptr[_r_zistepx]
 push ebp
 push edi
 ror ecx,16
 movsx edx,word ptr[24+esi]
 mov dword ptr[lzistepx],ecx
LRGBFuzzSpanLoop:
 mov eax,dword ptr[_d_aspancount]
 sub eax,edx
 mov edx,dword ptr[_erroradjustup]
 mov ebx,dword ptr[_errorterm]
 add ebx,edx
 js LRGBFuzzNoTurnover
 mov edx,dword ptr[_erroradjustdown]
 mov edi,dword ptr[_d_countextrastep]
 sub ebx,edx
 mov ebp,dword ptr[_d_aspancount]
 mov dword ptr[_errorterm],ebx
 add ebp,edi
 mov dword ptr[_d_aspancount],ebp
 jmp LRGBFuzzRightEdgeStepped
LRGBFuzzNoTurnover:
 mov edi,dword ptr[_d_aspancount]
 mov edx,dword ptr[_ubasestep]
 mov dword ptr[_errorterm],ebx
 add edi,edx
 mov dword ptr[_d_aspancount],edi
LRGBFuzzRightEdgeStepped:
 cmp eax,1
 jl LRGBFuzzNextSpan
 jz LRGBFuzzExactlyOneLong
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
 jmp  dword ptr[LRGBFuzzentryvec_table+eax*4]
LRGBFuzzentryvec_table:
 dd LRGBFuzzDraw8, LRGBFuzzDraw7, LRGBFuzzDraw6, LRGBFuzzDraw5
 dd LRGBFuzzDraw4, LRGBFuzzDraw3, LRGBFuzzDraw2, LRGBFuzzDraw1
LRGBFuzzDrawLoop:
LRGBFuzzDraw8:
 cmp bp,word ptr[ecx]
 jl LRGBFuzzp1
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRFuzzPatch8:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGFuzzPatch8:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBFuzzPatch8:
 or ax,word ptr[Ltemp]
 mov ah,byte ptr[12345678h+eax*2]
LTableFuzzPatch8:
 mov al,byte ptr[edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[edi],al
LRGBFuzzp1:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBFuzzDraw7:
 cmp bp,word ptr[2+ecx]
 jl LRGBFuzzp2
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[2+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRFuzzPatch7:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGFuzzPatch7:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBFuzzPatch7:
 or ax,word ptr[Ltemp]
 mov ah,byte ptr[12345678h+eax*2]
LTableFuzzPatch7:
 mov al,byte ptr[1+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[1+edi],al
LRGBFuzzp2:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBFuzzDraw6:
 cmp bp,word ptr[4+ecx]
 jl LRGBFuzzp3
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[4+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRFuzzPatch6:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGFuzzPatch6:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBFuzzPatch6:
 or ax,word ptr[Ltemp]
 mov ah,byte ptr[12345678h+eax*2]
LTableFuzzPatch6:
 mov al,byte ptr[2+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[2+edi],al
LRGBFuzzp3:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBFuzzDraw5:
 cmp bp,word ptr[6+ecx]
 jl LRGBFuzzp4
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[6+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRFuzzPatch5:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGFuzzPatch5:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBFuzzPatch5:
 or ax,word ptr[Ltemp]
 mov ah,byte ptr[12345678h+eax*2]
LTableFuzzPatch5:
 mov al,byte ptr[3+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[3+edi],al
LRGBFuzzp4:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBFuzzDraw4:
 cmp bp,word ptr[8+ecx]
 jl LRGBFuzzp5
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[8+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRFuzzPatch4:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGFuzzPatch4:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBFuzzPatch4:
 or ax,word ptr[Ltemp]
 mov ah,byte ptr[12345678h+eax*2]
LTableFuzzPatch4:
 mov al,byte ptr[4+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[4+edi],al
LRGBFuzzp5:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBFuzzDraw3:
 cmp bp,word ptr[10+ecx]
 jl LRGBFuzzp6
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[10+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRFuzzPatch3:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGFuzzPatch3:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBFuzzPatch3:
 or ax,word ptr[Ltemp]
 mov ah,byte ptr[12345678h+eax*2]
LTableFuzzPatch3:
 mov al,byte ptr[5+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[5+edi],al
LRGBFuzzp6:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBFuzzDraw2:
 cmp bp,word ptr[12+ecx]
 jl LRGBFuzzp7
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[12+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRFuzzPatch2:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGFuzzPatch2:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBFuzzPatch2:
 or ax,word ptr[Ltemp]
 mov ah,byte ptr[12345678h+eax*2]
LTableFuzzPatch2:
 mov al,byte ptr[6+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[6+edi],al
LRGBFuzzp7:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBFuzzDraw1:
 cmp bp,word ptr[14+ecx]
 jl LRGBFuzzp8
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[14+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRFuzzPatch1:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGFuzzPatch1:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBFuzzPatch1:
 or ax,word ptr[Ltemp]
 mov ah,byte ptr[12345678h+eax*2]
LTableFuzzPatch1:
 mov al,byte ptr[7+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[7+edi],al
LRGBFuzzp8:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
 add edi,8
 add ecx,16
 dec bx
 jnz LRGBFuzzDrawLoop
 pop esi
LRGBFuzzNextSpan:
 add esi,32
LRGBFuzzNextSpanESISet:
 movsx edx,word ptr[24+esi]
 cmp edx,offset -9999
 jnz LRGBFuzzSpanLoop
 pop edi
 pop ebp
 pop ebx
 pop esi
 ret
LRGBFuzzExactlyOneLong:
 mov ecx,dword ptr[4+esi]
 mov ebp,dword ptr[20+esi]
 ror ebp,16
 mov ebx,dword ptr[8+esi]
 cmp bp,word ptr[ecx]
 jl LRGBFuzzNextSpan
 xor eax,eax
 mov edi,dword ptr[0+esi]
 mov ah,byte ptr[26+1+esi]
 mov al,byte ptr[ebx]
 mov word ptr[ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRFuzzPatch9:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[28+1+esi]
 mov al,byte ptr[ebx]
 mov ax,word ptr[12345678h+eax*2]
LGFuzzPatch9:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[30+1+esi]
 add esi,32
 mov al,byte ptr[ebx]
 mov ax,word ptr[12345678h+eax*2]
LBFuzzPatch9:
 or ax,word ptr[Ltemp]
 mov ah,byte ptr[12345678h+eax*2]
LTableFuzzPatch9:
 mov al,byte ptr[edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[edi],al
 jmp LRGBFuzzNextSpanESISet
 public _D_PolysetDrawSpansRGBAltFuzz_8
_D_PolysetDrawSpansRGBAltFuzz_8:
 push esi
 push ebx
 mov esi,dword ptr[4+8+esp]
 mov ecx,dword ptr[_r_zistepx]
 push ebp
 push edi
 ror ecx,16
 movsx edx,word ptr[24+esi]
 mov dword ptr[lzistepx],ecx
LRGBAltFuzzSpanLoop:
 mov eax,dword ptr[_d_aspancount]
 sub eax,edx
 mov edx,dword ptr[_erroradjustup]
 mov ebx,dword ptr[_errorterm]
 add ebx,edx
 js LRGBAltFuzzNoTurnover
 mov edx,dword ptr[_erroradjustdown]
 mov edi,dword ptr[_d_countextrastep]
 sub ebx,edx
 mov ebp,dword ptr[_d_aspancount]
 mov dword ptr[_errorterm],ebx
 add ebp,edi
 mov dword ptr[_d_aspancount],ebp
 jmp LRGBAltFuzzRightEdgeStepped
LRGBAltFuzzNoTurnover:
 mov edi,dword ptr[_d_aspancount]
 mov edx,dword ptr[_ubasestep]
 mov dword ptr[_errorterm],ebx
 add edi,edx
 mov dword ptr[_d_aspancount],edi
LRGBAltFuzzRightEdgeStepped:
 cmp eax,1
 jl LRGBAltFuzzNextSpan
 jz LRGBAltFuzzExactlyOneLong
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
 jmp  dword ptr[LRGBAltFuzzentryvec_table+eax*4]
LRGBAltFuzzentryvec_table:
 dd LRGBAltFuzzDraw8, LRGBAltFuzzDraw7, LRGBAltFuzzDraw6, LRGBAltFuzzDraw5
 dd LRGBAltFuzzDraw4, LRGBAltFuzzDraw3, LRGBAltFuzzDraw2, LRGBAltFuzzDraw1
LRGBAltFuzzDrawLoop:
LRGBAltFuzzDraw8:
 cmp bp,word ptr[ecx]
 jl LRGBAltFuzzp1
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRAltFuzzPatch8:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGAltFuzzPatch8:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBAltFuzzPatch8:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTableAltFuzzPatch8:
 mov ah,byte ptr[edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[edi],al
LRGBAltFuzzp1:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBAltFuzzDraw7:
 cmp bp,word ptr[2+ecx]
 jl LRGBAltFuzzp2
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[2+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRAltFuzzPatch7:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGAltFuzzPatch7:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBAltFuzzPatch7:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTableAltFuzzPatch7:
 mov ah,byte ptr[1+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[1+edi],al
LRGBAltFuzzp2:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBAltFuzzDraw6:
 cmp bp,word ptr[4+ecx]
 jl LRGBAltFuzzp3
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[4+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRAltFuzzPatch6:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGAltFuzzPatch6:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBAltFuzzPatch6:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTableAltFuzzPatch6:
 mov ah,byte ptr[2+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[2+edi],al
LRGBAltFuzzp3:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBAltFuzzDraw5:
 cmp bp,word ptr[6+ecx]
 jl LRGBAltFuzzp4
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[6+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRAltFuzzPatch5:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGAltFuzzPatch5:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBAltFuzzPatch5:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTableAltFuzzPatch5:
 mov ah,byte ptr[3+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[3+edi],al
LRGBAltFuzzp4:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBAltFuzzDraw4:
 cmp bp,word ptr[8+ecx]
 jl LRGBAltFuzzp5
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[8+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRAltFuzzPatch4:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGAltFuzzPatch4:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBAltFuzzPatch4:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTableAltFuzzPatch4:
 mov ah,byte ptr[4+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[4+edi],al
LRGBAltFuzzp5:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBAltFuzzDraw3:
 cmp bp,word ptr[10+ecx]
 jl LRGBAltFuzzp6
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[10+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRAltFuzzPatch3:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGAltFuzzPatch3:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBAltFuzzPatch3:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTableAltFuzzPatch3:
 mov ah,byte ptr[5+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[5+edi],al
LRGBAltFuzzp6:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBAltFuzzDraw2:
 cmp bp,word ptr[12+ecx]
 jl LRGBAltFuzzp7
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[12+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRAltFuzzPatch2:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGAltFuzzPatch2:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBAltFuzzPatch2:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTableAltFuzzPatch2:
 mov ah,byte ptr[6+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[6+edi],al
LRGBAltFuzzp7:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
LRGBAltFuzzDraw1:
 cmp bp,word ptr[14+ecx]
 jl LRGBAltFuzzp8
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[14+ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRAltFuzzPatch1:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LGAltFuzzPatch1:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov ax,word ptr[12345678h+eax*2]
LBAltFuzzPatch1:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTableAltFuzzPatch1:
 mov ah,byte ptr[7+edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[7+edi],al
LRGBAltFuzzp8:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
 add edi,8
 add ecx,16
 dec bx
 jnz LRGBAltFuzzDrawLoop
 pop esi
LRGBAltFuzzNextSpan:
 add esi,32
LRGBAltFuzzNextSpanESISet:
 movsx edx,word ptr[24+esi]
 cmp edx,offset -9999
 jnz LRGBAltFuzzSpanLoop
 pop edi
 pop ebp
 pop ebx
 pop esi
 ret
LRGBAltFuzzExactlyOneLong:
 mov ecx,dword ptr[4+esi]
 mov ebp,dword ptr[20+esi]
 ror ebp,16
 mov ebx,dword ptr[8+esi]
 cmp bp,word ptr[ecx]
 jl LRGBAltFuzzNextSpan
 xor eax,eax
 mov edi,dword ptr[0+esi]
 mov ah,byte ptr[26+1+esi]
 mov al,byte ptr[ebx]
 mov word ptr[ecx],bp
 mov ax,word ptr[12345678h+eax*2]
LRAltFuzzPatch9:
 mov word ptr[Ltemp],ax
 mov ah,byte ptr[28+1+esi]
 mov al,byte ptr[ebx]
 mov ax,word ptr[12345678h+eax*2]
LGAltFuzzPatch9:
 or word ptr[Ltemp],ax
 mov ah,byte ptr[30+1+esi]
 add esi,32
 mov al,byte ptr[ebx]
 mov ax,word ptr[12345678h+eax*2]
LBAltFuzzPatch9:
 or ax,word ptr[Ltemp]
 mov al,byte ptr[12345678h+eax*2]
LTableAltFuzzPatch9:
 mov ah,byte ptr[edi]
 add eax,dword ptr[_d_transluc]
 mov al,byte ptr[eax]
 mov byte ptr[edi],al
 jmp LRGBAltFuzzNextSpanESISet
 public _D_PolysetAff8End
_D_PolysetAff8End:
 public _D_Aff8Patch
_D_Aff8Patch:
 mov eax,dword ptr[_fadetable]
 mov dword ptr[LPatch1-4],eax
 mov dword ptr[LPatch2-4],eax
 mov dword ptr[LPatch3-4],eax
 mov dword ptr[LPatch4-4],eax
 mov dword ptr[LPatch5-4],eax
 mov dword ptr[LPatch6-4],eax
 mov dword ptr[LPatch7-4],eax
 mov dword ptr[LPatch8-4],eax
 mov dword ptr[LPatch9-4],eax
 mov dword ptr[LFuzzPatch1-4],eax
 mov dword ptr[LFuzzPatch2-4],eax
 mov dword ptr[LFuzzPatch3-4],eax
 mov dword ptr[LFuzzPatch4-4],eax
 mov dword ptr[LFuzzPatch5-4],eax
 mov dword ptr[LFuzzPatch6-4],eax
 mov dword ptr[LFuzzPatch7-4],eax
 mov dword ptr[LFuzzPatch8-4],eax
 mov dword ptr[LFuzzPatch9-4],eax
 mov dword ptr[LAltFuzzPatch1-4],eax
 mov dword ptr[LAltFuzzPatch2-4],eax
 mov dword ptr[LAltFuzzPatch3-4],eax
 mov dword ptr[LAltFuzzPatch4-4],eax
 mov dword ptr[LAltFuzzPatch5-4],eax
 mov dword ptr[LAltFuzzPatch6-4],eax
 mov dword ptr[LAltFuzzPatch7-4],eax
 mov dword ptr[LAltFuzzPatch8-4],eax
 mov dword ptr[LAltFuzzPatch9-4],eax
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
 mov dword ptr[LRFuzzPatch1-4],eax
 mov dword ptr[LRFuzzPatch2-4],eax
 mov dword ptr[LRFuzzPatch3-4],eax
 mov dword ptr[LRFuzzPatch4-4],eax
 mov dword ptr[LRFuzzPatch5-4],eax
 mov dword ptr[LRFuzzPatch6-4],eax
 mov dword ptr[LRFuzzPatch7-4],eax
 mov dword ptr[LRFuzzPatch8-4],eax
 mov dword ptr[LRFuzzPatch9-4],eax
 mov dword ptr[LRAltFuzzPatch1-4],eax
 mov dword ptr[LRAltFuzzPatch2-4],eax
 mov dword ptr[LRAltFuzzPatch3-4],eax
 mov dword ptr[LRAltFuzzPatch4-4],eax
 mov dword ptr[LRAltFuzzPatch5-4],eax
 mov dword ptr[LRAltFuzzPatch6-4],eax
 mov dword ptr[LRAltFuzzPatch7-4],eax
 mov dword ptr[LRAltFuzzPatch8-4],eax
 mov dword ptr[LRAltFuzzPatch9-4],eax
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
 mov dword ptr[LGFuzzPatch1-4],eax
 mov dword ptr[LGFuzzPatch2-4],eax
 mov dword ptr[LGFuzzPatch3-4],eax
 mov dword ptr[LGFuzzPatch4-4],eax
 mov dword ptr[LGFuzzPatch5-4],eax
 mov dword ptr[LGFuzzPatch6-4],eax
 mov dword ptr[LGFuzzPatch7-4],eax
 mov dword ptr[LGFuzzPatch8-4],eax
 mov dword ptr[LGFuzzPatch9-4],eax
 mov dword ptr[LGAltFuzzPatch1-4],eax
 mov dword ptr[LGAltFuzzPatch2-4],eax
 mov dword ptr[LGAltFuzzPatch3-4],eax
 mov dword ptr[LGAltFuzzPatch4-4],eax
 mov dword ptr[LGAltFuzzPatch5-4],eax
 mov dword ptr[LGAltFuzzPatch6-4],eax
 mov dword ptr[LGAltFuzzPatch7-4],eax
 mov dword ptr[LGAltFuzzPatch8-4],eax
 mov dword ptr[LGAltFuzzPatch9-4],eax
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
 mov dword ptr[LBFuzzPatch1-4],eax
 mov dword ptr[LBFuzzPatch2-4],eax
 mov dword ptr[LBFuzzPatch3-4],eax
 mov dword ptr[LBFuzzPatch4-4],eax
 mov dword ptr[LBFuzzPatch5-4],eax
 mov dword ptr[LBFuzzPatch6-4],eax
 mov dword ptr[LBFuzzPatch7-4],eax
 mov dword ptr[LBFuzzPatch8-4],eax
 mov dword ptr[LBFuzzPatch9-4],eax
 mov dword ptr[LBAltFuzzPatch1-4],eax
 mov dword ptr[LBAltFuzzPatch2-4],eax
 mov dword ptr[LBAltFuzzPatch3-4],eax
 mov dword ptr[LBAltFuzzPatch4-4],eax
 mov dword ptr[LBAltFuzzPatch5-4],eax
 mov dword ptr[LBAltFuzzPatch6-4],eax
 mov dword ptr[LBAltFuzzPatch7-4],eax
 mov dword ptr[LBAltFuzzPatch8-4],eax
 mov dword ptr[LBAltFuzzPatch9-4],eax
 mov eax,dword ptr[_d_rgbtable]
 mov dword ptr[LTablePatch1-4],eax
 mov dword ptr[LTablePatch2-4],eax
 mov dword ptr[LTablePatch3-4],eax
 mov dword ptr[LTablePatch4-4],eax
 mov dword ptr[LTablePatch5-4],eax
 mov dword ptr[LTablePatch6-4],eax
 mov dword ptr[LTablePatch7-4],eax
 mov dword ptr[LTablePatch8-4],eax
 mov dword ptr[LTablePatch9-4],eax
 mov dword ptr[LTableFuzzPatch1-4],eax
 mov dword ptr[LTableFuzzPatch2-4],eax
 mov dword ptr[LTableFuzzPatch3-4],eax
 mov dword ptr[LTableFuzzPatch4-4],eax
 mov dword ptr[LTableFuzzPatch5-4],eax
 mov dword ptr[LTableFuzzPatch6-4],eax
 mov dword ptr[LTableFuzzPatch7-4],eax
 mov dword ptr[LTableFuzzPatch8-4],eax
 mov dword ptr[LTableFuzzPatch9-4],eax
 mov dword ptr[LTableAltFuzzPatch1-4],eax
 mov dword ptr[LTableAltFuzzPatch2-4],eax
 mov dword ptr[LTableAltFuzzPatch3-4],eax
 mov dword ptr[LTableAltFuzzPatch4-4],eax
 mov dword ptr[LTableAltFuzzPatch5-4],eax
 mov dword ptr[LTableAltFuzzPatch6-4],eax
 mov dword ptr[LTableAltFuzzPatch7-4],eax
 mov dword ptr[LTableAltFuzzPatch8-4],eax
 mov dword ptr[LTableAltFuzzPatch9-4],eax
 ret
_TEXT ENDS
 END
