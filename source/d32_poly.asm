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
Ltemp dd 0
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _D_PolysetAff32Start
_D_PolysetAff32Start:
 public _D_PolysetDrawSpans_32
_D_PolysetDrawSpans_32:
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
 sub edi,eax
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
 mov eax,dword ptr[12345678h+eax*4]
LPatch8:
 mov dword ptr[edi],eax
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
 mov eax,dword ptr[12345678h+eax*4]
LPatch7:
 mov dword ptr[4+edi],eax
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
 mov eax,dword ptr[12345678h+eax*4]
LPatch6:
 mov dword ptr[8+edi],eax
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
 mov eax,dword ptr[12345678h+eax*4]
LPatch5:
 mov dword ptr[12+edi],eax
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
 mov eax,dword ptr[12345678h+eax*4]
LPatch4:
 mov dword ptr[16+edi],eax
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
 mov eax,dword ptr[12345678h+eax*4]
LPatch3:
 mov dword ptr[20+edi],eax
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
 mov eax,dword ptr[12345678h+eax*4]
LPatch2:
 mov dword ptr[24+edi],eax
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
 mov eax,dword ptr[12345678h+eax*4]
LPatch1:
 mov dword ptr[28+edi],eax
Lp8:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
 add edi,32
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
 mov eax,dword ptr[12345678h+eax*4]
LPatch9:
 mov dword ptr[edi],eax
 jmp LNextSpanESISet
 public _D_PolysetDrawSpansFuzz_32
_D_PolysetDrawSpansFuzz_32:
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
 sub edi,eax
 sub edi,eax
 sub edi,eax
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
 mov eax,dword ptr[12345678h+eax*4]
LFuzzPatch8:
 push ebx
 push ecx
 push edx
 push esi
 push eax
 mov esi,dword ptr[_d_srctranstab]
 mov edx,dword ptr[_d_dsttranstab]
 and eax,0ffh
 xor ecx,ecx
 mov cl,byte ptr[edi]
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[1+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[1+edi]
 mov byte ptr[edi],bh
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[2+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[2+edi]
 mov byte ptr[1+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 pop eax
 pop esi
 add ebx,dword ptr[edx+ecx*2]
 pop edx
 pop ecx
 mov byte ptr[2+edi],bh
 pop ebx
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
 mov eax,dword ptr[12345678h+eax*4]
LFuzzPatch7:
 push ebx
 push ecx
 push edx
 push esi
 push eax
 mov esi,dword ptr[_d_srctranstab]
 mov edx,dword ptr[_d_dsttranstab]
 and eax,0ffh
 xor ecx,ecx
 mov cl,byte ptr[4+edi]
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[1+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[5+edi]
 mov byte ptr[4+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[2+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[6+edi]
 mov byte ptr[5+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 pop eax
 pop esi
 add ebx,dword ptr[edx+ecx*2]
 pop edx
 pop ecx
 mov byte ptr[6+edi],bh
 pop ebx
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
 mov eax,dword ptr[12345678h+eax*4]
LFuzzPatch6:
 push ebx
 push ecx
 push edx
 push esi
 push eax
 mov esi,dword ptr[_d_srctranstab]
 mov edx,dword ptr[_d_dsttranstab]
 and eax,0ffh
 xor ecx,ecx
 mov cl,byte ptr[8+edi]
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[1+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[9+edi]
 mov byte ptr[8+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[2+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[10+edi]
 mov byte ptr[9+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 pop eax
 pop esi
 add ebx,dword ptr[edx+ecx*2]
 pop edx
 pop ecx
 mov byte ptr[10+edi],bh
 pop ebx
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
 mov eax,dword ptr[12345678h+eax*4]
LFuzzPatch5:
 push ebx
 push ecx
 push edx
 push esi
 push eax
 mov esi,dword ptr[_d_srctranstab]
 mov edx,dword ptr[_d_dsttranstab]
 and eax,0ffh
 xor ecx,ecx
 mov cl,byte ptr[12+edi]
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[1+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[13+edi]
 mov byte ptr[12+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[2+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[14+edi]
 mov byte ptr[13+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 pop eax
 pop esi
 add ebx,dword ptr[edx+ecx*2]
 pop edx
 pop ecx
 mov byte ptr[14+edi],bh
 pop ebx
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
 mov eax,dword ptr[12345678h+eax*4]
LFuzzPatch4:
 push ebx
 push ecx
 push edx
 push esi
 push eax
 mov esi,dword ptr[_d_srctranstab]
 mov edx,dword ptr[_d_dsttranstab]
 and eax,0ffh
 xor ecx,ecx
 mov cl,byte ptr[16+edi]
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[1+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[17+edi]
 mov byte ptr[16+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[2+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[18+edi]
 mov byte ptr[17+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 pop eax
 pop esi
 add ebx,dword ptr[edx+ecx*2]
 pop edx
 pop ecx
 mov byte ptr[18+edi],bh
 pop ebx
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
 mov eax,dword ptr[12345678h+eax*4]
LFuzzPatch3:
 push ebx
 push ecx
 push edx
 push esi
 push eax
 mov esi,dword ptr[_d_srctranstab]
 mov edx,dword ptr[_d_dsttranstab]
 and eax,0ffh
 xor ecx,ecx
 mov cl,byte ptr[20+edi]
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[1+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[21+edi]
 mov byte ptr[20+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[2+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[22+edi]
 mov byte ptr[21+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 pop eax
 pop esi
 add ebx,dword ptr[edx+ecx*2]
 pop edx
 pop ecx
 mov byte ptr[22+edi],bh
 pop ebx
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
 mov eax,dword ptr[12345678h+eax*4]
LFuzzPatch2:
 push ebx
 push ecx
 push edx
 push esi
 push eax
 mov esi,dword ptr[_d_srctranstab]
 mov edx,dword ptr[_d_dsttranstab]
 and eax,0ffh
 xor ecx,ecx
 mov cl,byte ptr[24+edi]
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[1+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[25+edi]
 mov byte ptr[24+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[2+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[26+edi]
 mov byte ptr[25+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 pop eax
 pop esi
 add ebx,dword ptr[edx+ecx*2]
 pop edx
 pop ecx
 mov byte ptr[26+edi],bh
 pop ebx
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
 mov eax,dword ptr[12345678h+eax*4]
LFuzzPatch1:
 push ebx
 push ecx
 push edx
 push esi
 push eax
 mov esi,dword ptr[_d_srctranstab]
 mov edx,dword ptr[_d_dsttranstab]
 and eax,0ffh
 xor ecx,ecx
 mov cl,byte ptr[28+edi]
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[1+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[29+edi]
 mov byte ptr[28+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[2+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[30+edi]
 mov byte ptr[29+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 pop eax
 pop esi
 add ebx,dword ptr[edx+ecx*2]
 pop edx
 pop ecx
 mov byte ptr[30+edi],bh
 pop ebx
LFuzzp8:
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
 add edi,32
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
 mov eax,dword ptr[12345678h+eax*4]
LFuzzPatch9:
 push ebx
 push ecx
 push edx
 push esi
 push eax
 mov esi,dword ptr[_d_srctranstab]
 mov edx,dword ptr[_d_dsttranstab]
 and eax,0ffh
 xor ecx,ecx
 mov cl,byte ptr[edi]
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[1+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[1+edi]
 mov byte ptr[edi],bh
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[2+esp]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[2+edi]
 mov byte ptr[1+edi],bh
 mov ebx,dword ptr[esi+eax*2]
 pop eax
 pop esi
 add ebx,dword ptr[edx+ecx*2]
 pop edx
 pop ecx
 mov byte ptr[2+edi],bh
 pop ebx
 jmp LFuzzNextSpanESISet
 public _D_PolysetDrawSpansRGB_32
_D_PolysetDrawSpansRGB_32:
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
 sub edi,eax
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
 mov al,byte ptr[12345678h+eax]
LRPatch8:
 mov byte ptr[31+edi],al
LROffsPatch8:
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LGPatch8:
 mov byte ptr[31+edi],al
LGOffsPatch8:
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LBPatch8:
 mov byte ptr[31+edi],al
LBOffsPatch8:
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
 mov al,byte ptr[12345678h+eax]
LRPatch7:
 mov byte ptr[31+edi],al
LROffsPatch7:
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LGPatch7:
 mov byte ptr[31+edi],al
LGOffsPatch7:
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LBPatch7:
 mov byte ptr[31+edi],al
LBOffsPatch7:
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
 mov al,byte ptr[12345678h+eax]
LRPatch6:
 mov byte ptr[31+edi],al
LROffsPatch6:
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LGPatch6:
 mov byte ptr[31+edi],al
LGOffsPatch6:
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LBPatch6:
 mov byte ptr[31+edi],al
LBOffsPatch6:
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
 mov al,byte ptr[12345678h+eax]
LRPatch5:
 mov byte ptr[31+edi],al
LROffsPatch5:
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LGPatch5:
 mov byte ptr[31+edi],al
LGOffsPatch5:
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LBPatch5:
 mov byte ptr[31+edi],al
LBOffsPatch5:
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
 mov al,byte ptr[12345678h+eax]
LRPatch4:
 mov byte ptr[31+edi],al
LROffsPatch4:
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LGPatch4:
 mov byte ptr[31+edi],al
LGOffsPatch4:
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LBPatch4:
 mov byte ptr[31+edi],al
LBOffsPatch4:
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
 mov al,byte ptr[12345678h+eax]
LRPatch3:
 mov byte ptr[31+edi],al
LROffsPatch3:
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LGPatch3:
 mov byte ptr[31+edi],al
LGOffsPatch3:
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LBPatch3:
 mov byte ptr[31+edi],al
LBOffsPatch3:
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
 mov al,byte ptr[12345678h+eax]
LRPatch2:
 mov byte ptr[31+edi],al
LROffsPatch2:
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LGPatch2:
 mov byte ptr[31+edi],al
LGOffsPatch2:
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LBPatch2:
 mov byte ptr[31+edi],al
LBOffsPatch2:
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
 mov al,byte ptr[12345678h+eax]
LRPatch1:
 mov byte ptr[31+edi],al
LROffsPatch1:
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LGPatch1:
 mov byte ptr[31+edi],al
LGOffsPatch1:
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LBPatch1:
 mov byte ptr[31+edi],al
LBOffsPatch1:
LRGBp8:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
 add edi,32
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
 mov al,byte ptr[12345678h+eax]
LRPatch9:
 mov byte ptr[31+edi],al
LROffsPatch9:
 mov ah,byte ptr[28+1+esi]
 mov al,byte ptr[ebx]
 mov al,byte ptr[12345678h+eax]
LGPatch9:
 mov byte ptr[31+edi],al
LGOffsPatch9:
 mov ah,byte ptr[30+1+esi]
 mov al,byte ptr[ebx]
 add esi,32 
 mov al,byte ptr[12345678h+eax]
LBPatch9:
 mov byte ptr[31+edi],al
LBOffsPatch9:
 jmp LRGBNextSpanESISet
 public _D_PolysetDrawSpansRGBFuzz_32
_D_PolysetDrawSpansRGBFuzz_32:
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
 mov ebx,dword ptr[12+esi]
 mov bx,dx
 mov ecx,dword ptr[4+esi]
 mov edi,dword ptr[0+esi]
 mov edx,dword ptr[28+esi]
 mov dword ptr[gb],edx
 mov edx,dword ptr[_r_bstepx]
 shr edx,16
 mov dx,word ptr[_r_gstepx]
 mov dword ptr[gbstep],edx
 mov edx,dword ptr[16+esi]
 mov dx,word ptr[26+esi]
 mov ebp,dword ptr[20+esi]
 ror ebp,16
 push esi
 mov esi,dword ptr[8+esi]
LRGBFuzzDrawLoop:
LRGBFuzzDraw1:
 cmp bp,word ptr[14+ecx]
 jl LRGBFuzzp1
 xor eax,eax
 mov ah,dh
 mov al,byte ptr[esi]
 mov word ptr[14+ecx],bp
 mov al,byte ptr[12345678h+eax]
LRFuzzPatch1:
 mov byte ptr[Ltemp],al
 mov ah,byte ptr[gb+1]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LGFuzzPatch1:
 mov byte ptr[Ltemp+1],al
 mov ah,byte ptr[gb+3]
 mov al,byte ptr[esi]
 mov al,byte ptr[12345678h+eax]
LBFuzzPatch1:
 mov byte ptr[Ltemp+2],al
 xor eax,eax
 push ebx
 push ecx
 push edx
 push esi
 mov esi,dword ptr[_d_srctranstab]
 mov edx,dword ptr[_d_dsttranstab]
 mov al,byte ptr[Ltemp]
 xor ecx,ecx
 mov cl,byte ptr[31+edi]
LRFuzzOffsPatch1:
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[Ltemp+1]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[31+edi]
LGFuzzOffsPatch1:
 mov byte ptr[31+edi],bh
LRFuzzOffsPatch2:
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[Ltemp+2]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[31+edi]
LBFuzzOffsPatch1:
 mov byte ptr[31+edi],bh
LGFuzzOffsPatch2:
 mov ebx,dword ptr[esi+eax*2]
 pop esi
 add ebx,dword ptr[edx+ecx*2]
 pop edx
 pop ecx
 mov byte ptr[31+edi],bh
LBFuzzOffsPatch2:
 pop ebx
LRGBFuzzp1:
 mov eax,dword ptr[gbstep]
 add dword ptr[gb],eax
 add edx,dword ptr[tstep]
 sbb eax,eax
 add ebp,dword ptr[lzistepx]
 adc ebp,0
 add ebx,dword ptr[_a_sstepxfrac]
 adc esi,dword ptr[advancetable+4+eax*4]
 add edi,4
 add ecx,2
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
 mov al,byte ptr[12345678h+eax]
LRFuzzPatch2:
 mov byte ptr[Ltemp],al
 mov ah,byte ptr[28+1+esi]
 mov al,byte ptr[ebx]
 mov al,byte ptr[12345678h+eax]
LGFuzzPatch2:
 mov byte ptr[Ltemp+1],al
 mov ah,byte ptr[30+1+esi]
 mov al,byte ptr[ebx]
 add esi,32 
 mov al,byte ptr[12345678h+eax]
LBFuzzPatch2:
 mov byte ptr[Ltemp+2],al
 xor eax,eax
 push ebx
 push ecx
 push edx
 push esi
 mov esi,dword ptr[_d_srctranstab]
 mov edx,dword ptr[_d_dsttranstab]
 mov al,byte ptr[Ltemp]
 xor ecx,ecx
 mov cl,byte ptr[31+edi]
LRFuzzOffsPatch3:
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[Ltemp+1]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[31+edi]
LGFuzzOffsPatch3:
 mov byte ptr[31+edi],bh
LRFuzzOffsPatch4:
 mov ebx,dword ptr[esi+eax*2]
 mov al,byte ptr[Ltemp+2]
 add ebx,dword ptr[edx+ecx*2]
 mov cl,byte ptr[31+edi]
LBFuzzOffsPatch3:
 mov byte ptr[31+edi],bh
LGFuzzOffsPatch4:
 mov ebx,dword ptr[esi+eax*2]
 pop esi
 add ebx,dword ptr[edx+ecx*2]
 pop edx
 pop ecx
 mov byte ptr[31+edi],bh
LBFuzzOffsPatch4:
 pop ebx
 jmp LRGBFuzzNextSpanESISet
 public _D_PolysetAff32End
_D_PolysetAff32End:
LROffsPatchTable:
 dd LROffsPatch1-1, 28
 dd LROffsPatch2-1, 24
 dd LROffsPatch3-1, 20
 dd LROffsPatch4-1, 16
 dd LROffsPatch5-1, 12
 dd LROffsPatch6-1, 8
 dd LROffsPatch7-1, 4
 dd LROffsPatch8-1, 0
 dd LROffsPatch9-1, 0
 dd LRFuzzOffsPatch1-1, 0
 dd LRFuzzOffsPatch2-1, 0
 dd LRFuzzOffsPatch3-1, 0
 dd LRFuzzOffsPatch4-1, 0
LGOffsPatchTable:
 dd LGOffsPatch1-1, 28
 dd LGOffsPatch2-1, 24
 dd LGOffsPatch3-1, 20
 dd LGOffsPatch4-1, 16
 dd LGOffsPatch5-1, 12
 dd LGOffsPatch6-1, 8
 dd LGOffsPatch7-1, 4
 dd LGOffsPatch8-1, 0
 dd LGOffsPatch9-1, 0
 dd LGFuzzOffsPatch1-1, 0
 dd LGFuzzOffsPatch2-1, 0
 dd LGFuzzOffsPatch3-1, 0
 dd LGFuzzOffsPatch4-1, 0
LBOffsPatchTable:
 dd LBOffsPatch1-1, 28
 dd LBOffsPatch2-1, 24
 dd LBOffsPatch3-1, 20
 dd LBOffsPatch4-1, 16
 dd LBOffsPatch5-1, 12
 dd LBOffsPatch6-1, 8
 dd LBOffsPatch7-1, 4
 dd LBOffsPatch8-1, 0
 dd LBOffsPatch9-1, 0
 dd LBFuzzOffsPatch1-1, 0
 dd LBFuzzOffsPatch2-1, 0
 dd LBFuzzOffsPatch3-1, 0
 dd LBFuzzOffsPatch4-1, 0
 public _D_Aff32Patch
_D_Aff32Patch:
 mov eax,dword ptr[_fadetable32]
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
 mov eax,dword ptr[_fadetable32r]
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
 mov eax,dword ptr[_fadetable32g]
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
 mov eax,dword ptr[_fadetable32b]
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
 push ebx
 mov ebx,offset LROffsPatchTable
 mov ecx,13
LROffsPatchLoop:
 mov edx,dword ptr[ebx]
 mov eax,dword ptr[_roffs]
 add eax,dword ptr[4+ebx]
 add ebx,8
 mov byte ptr[edx],al
 dec ecx
 jnz LROffsPatchLoop
 mov ebx,offset LGOffsPatchTable
 mov ecx,13
LGOffsPatchLoop:
 mov edx,dword ptr[ebx]
 mov eax,dword ptr[_goffs]
 add eax,dword ptr[4+ebx]
 add ebx,8
 mov byte ptr[edx],al
 dec ecx
 jnz LGOffsPatchLoop
 mov ebx,offset LBOffsPatchTable
 mov ecx,13
LBOffsPatchLoop:
 mov edx,dword ptr[ebx]
 mov eax,dword ptr[_boffs]
 add eax,dword ptr[4+ebx]
 add ebx,8
 mov byte ptr[edx],al
 dec ecx
 jnz LBOffsPatchLoop
 pop ebx
 ret
_TEXT ENDS
 END
