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
 externdef _D_PolysetSetEdgeTable
 externdef _D_RasterizeAliasPolySmooth
_DATA SEGMENT
 align 4
p00_minus_p20 dd 0
p10_minus_p20 dd 0
p01_minus_p21 dd 0
p11_minus_p21 dd 0
Ltemp dd 0
_DATA ENDS
_TEXT SEGMENT
 public _D_DrawNonSubdiv
_D_DrawNonSubdiv:
 push ebp
 push edi
 push esi
 push ebx
 mov ebp,dword ptr[_d_affinetridesc+24]
 lea ebp,dword ptr[ebp+ebp*2]
 shl ebp,2
LTriangleLoop:
 mov edx,dword ptr[_d_affinetridesc+12]
 mov ebx,dword ptr[_d_affinetridesc+16]
 mov esi,dword ptr[_d_affinetridesc+16]
 mov edi,dword ptr[_d_affinetridesc+16]
 movsx eax,word ptr[0-12+edx+ebp]
 sal eax,5
 add ebx,eax
 movsx eax,word ptr[0+2-12+edx+ebp]
 sal eax,5
 add esi,eax
 movsx eax,word ptr[0+4-12+edx+ebp]
 sal eax,5
 add edi,eax
 mov eax,dword ptr[0+ebx]
 mov edx,dword ptr[4+ebx]
 sub eax,dword ptr[0+edi]
 sub edx,dword ptr[4+esi]
 imul edx,eax
 mov eax,dword ptr[0+ebx]
 mov ecx,dword ptr[4+ebx]
 sub eax,dword ptr[0+esi]
 sub ecx,dword ptr[4+edi]
 imul eax,ecx
 sub edx,eax
 test edx,edx
 jle LNextTriangle
 mov dword ptr[_d_denom],edx
 fild dword ptr[_d_denom]
 fld1
 fdivrp st(1),st(0)
 mov eax,dword ptr[0+ebx]
 mov edx,dword ptr[4+ebx]
 mov dword ptr[_r_p0],eax
 mov dword ptr[_r_p0+4],edx
 mov eax,dword ptr[8+ebx]
 mov edx,dword ptr[12+ebx]
 mov dword ptr[_r_p0+16],eax
 mov dword ptr[_r_p0+20],edx
 mov eax,dword ptr[16+ebx]
 mov edx,dword ptr[20+ebx]
 mov dword ptr[_r_p0+24],eax
 mov dword ptr[_r_p0+28],edx
 mov eax,dword ptr[0+esi]
 mov edx,dword ptr[4+esi]
 mov dword ptr[_r_p1],eax
 mov dword ptr[_r_p1+4],edx
 mov eax,dword ptr[8+esi]
 mov edx,dword ptr[12+esi]
 mov dword ptr[_r_p1+16],eax
 mov dword ptr[_r_p1+20],edx
 mov eax,dword ptr[16+esi]
 mov edx,dword ptr[20+esi]
 mov dword ptr[_r_p1+24],eax
 mov dword ptr[_r_p1+28],edx
 mov ecx,dword ptr[_d_affinetridesc+12]
 mov eax,dword ptr[0+edi]
 mov edx,dword ptr[4+edi]
 mov dword ptr[_r_p2],eax
 mov dword ptr[_r_p2+4],edx
 mov eax,dword ptr[8+edi]
 mov edx,dword ptr[12+edi]
 mov dword ptr[_r_p2+16],eax
 mov dword ptr[_r_p2+20],edx
 mov eax,dword ptr[16+edi]
 mov edx,dword ptr[20+edi]
 mov dword ptr[_r_p2+24],eax
 mov dword ptr[_r_p2+28],edx
 mov edx,dword ptr[_d_affinetridesc+20]
 movsx ebx,word ptr[6-12+ecx+ebp]
 movsx esi,word ptr[6+2-12+ecx+ebp]
 movsx edi,word ptr[6+4-12+ecx+ebp]
 lea ebx,dword ptr[edx+ebx*8]
 lea esi,dword ptr[edx+esi*8]
 lea edi,dword ptr[edx+edi*8]
 mov eax,dword ptr[0+ebx]
 mov edx,dword ptr[4+ebx]
 mov dword ptr[_r_p0+8],eax
 mov dword ptr[_r_p0+12],edx
 mov eax,dword ptr[0+esi]
 mov edx,dword ptr[4+esi]
 mov dword ptr[_r_p1+8],eax
 mov dword ptr[_r_p1+12],edx
 mov eax,dword ptr[0+edi]
 mov edx,dword ptr[4+edi]
 mov dword ptr[_r_p2+8],eax
 mov dword ptr[_r_p2+12],edx
 fstp dword ptr[_d_denom]
 mov eax,offset _D_PolysetSetEdgeTable
 call eax
 mov eax,offset _D_RasterizeAliasPolySmooth
 call eax
LNextTriangle:
 sub ebp,12
 jnz LTriangleLoop
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 public _D_PolysetCalcGradients
_D_PolysetCalcGradients:
 fild dword ptr[_r_p0]
 fild dword ptr[_r_p1]
 fild dword ptr[_r_p2]
 fild dword ptr[_r_p0+4]
 fild dword ptr[_r_p1+4]
 fild dword ptr[_r_p2+4]
 fxch st(5)
 fsub st(0),st(3)
 fxch st(2)
 fsub st(0),st(5)
 fxch st(4)
 fsubrp st(3),st(0)
 fsubrp st(4),st(0)
 fstp dword ptr[p00_minus_p20]
 fxch st(1)
 fstp dword ptr[p01_minus_p21]
 fstp dword ptr[p10_minus_p20]
 fstp dword ptr[p11_minus_p21]
 fld dword ptr[_d_denom]
 fild dword ptr[_r_p0+8]
 fild dword ptr[_r_p1+8]
 fild dword ptr[_r_p2+8]
 fsub st(1),st(0)
 fsubp st(2),st(0)
 fld st(0)
 fmul dword ptr[p01_minus_p21]
 fld st(2)
 fmul dword ptr[p11_minus_p21]
 fxch st(2)
 fmul dword ptr[p00_minus_p20]
 fxch st(3)
 fmul dword ptr[p10_minus_p20]
 fxch st(1)
 fsubrp st(2),st(0)
 fsubp st(2),st(0)
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(1)
 fistp dword ptr[_r_sstepx]
 fistp dword ptr[_r_sstepy]
 fild dword ptr[_r_p0+12]
 fild dword ptr[_r_p1+12]
 fild dword ptr[_r_p2+12]
 fsub st(1),st(0)
 fsubp st(2),st(0)
 fld st(0)
 fmul dword ptr[p01_minus_p21]
 fld st(2)
 fmul dword ptr[p11_minus_p21]
 fxch st(2)
 fmul dword ptr[p00_minus_p20]
 fxch st(3)
 fmul dword ptr[p10_minus_p20]
 fxch st(1)
 fsubrp st(2),st(0)
 fsubp st(2),st(0)
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(1)
 fistp dword ptr[_r_tstepx]
 fistp dword ptr[_r_tstepy]
 fild dword ptr[_r_p0+16]
 fild dword ptr[_r_p1+16]
 fild dword ptr[_r_p2+16]
 fsub st(1),st(0)
 fsubp st(2),st(0)
 fld st(0)
 fmul dword ptr[p01_minus_p21]
 fld st(2)
 fmul dword ptr[p11_minus_p21]
 fxch st(2)
 fmul dword ptr[p00_minus_p20]
 fxch st(3)
 fmul dword ptr[p10_minus_p20]
 fxch st(1)
 fsubrp st(2),st(0)
 fsubp st(2),st(0)
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(1)
 fistp dword ptr[_r_zistepx]
 fistp dword ptr[_r_zistepy]
 fild dword ptr[_r_p0+20]
 fild dword ptr[_r_p1+20]
 fild dword ptr[_r_p2+20]
 fsub st(1),st(0)
 fsubp st(2),st(0)
 fld st(0)
 fmul dword ptr[p01_minus_p21]
 fld st(2)
 fmul dword ptr[p11_minus_p21]
 fxch st(2)
 fmul dword ptr[p00_minus_p20]
 fxch st(3)
 fmul dword ptr[p10_minus_p20]
 fxch st(1)
 fsubrp st(2),st(0)
 fsubp st(2),st(0)
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(1)
 fldcw word ptr[ceil_cw]
 fistp dword ptr[_r_rstepx]
 fistp dword ptr[_r_rstepy]
 fldcw word ptr[single_cw]
 fild dword ptr[_r_p0+24]
 fild dword ptr[_r_p1+24]
 fild dword ptr[_r_p2+24]
 fsub st(1),st(0)
 fsubp st(2),st(0)
 fld st(0)
 fmul dword ptr[p01_minus_p21]
 fld st(2)
 fmul dword ptr[p11_minus_p21]
 fxch st(2)
 fmul dword ptr[p00_minus_p20]
 fxch st(3)
 fmul dword ptr[p10_minus_p20]
 fxch st(1)
 fsubrp st(2),st(0)
 fsubp st(2),st(0)
 fmul st(0),st(2)
 fxch st(1)
 fmul st(0),st(2)
 fxch st(1)
 fldcw word ptr[ceil_cw]
 fistp dword ptr[_r_gstepx]
 fistp dword ptr[_r_gstepy]
 fldcw word ptr[single_cw]
 fild dword ptr[_r_p0+28]
 fild dword ptr[_r_p1+28]
 fild dword ptr[_r_p2+28]
 fsub st(1),st(0)
 fsubp st(2),st(0)
 fld st(0)
 fmul dword ptr[p01_minus_p21]
 fld st(2)
 fmul dword ptr[p11_minus_p21]
 fxch st(2)
 fmul dword ptr[p00_minus_p20]
 fxch st(3)
 fmul dword ptr[p10_minus_p20]
 fxch st(1)
 fsubrp st(2),st(0)
 fsubp st(2),st(0)
 fmul st(0),st(2)
 fxch st(1)
 fmulp st(2),st(0)
 fldcw word ptr[ceil_cw]
 fistp dword ptr[_r_bstepx]
 fistp dword ptr[_r_bstepy]
 fldcw word ptr[single_cw]
 mov eax,dword ptr[_r_sstepx]
 mov edx,dword ptr[_r_tstepx]
 shl eax,16
 sal edx,16
 mov dword ptr[_a_sstepxfrac],eax
 mov dword ptr[_a_tstepxfrac],edx
 mov ecx,dword ptr[_r_sstepx]
 mov eax,dword ptr[_r_tstepx]
 sar ecx,16
 sar eax,16
 imul dword ptr[4+0+esp]
 add eax,ecx
 mov dword ptr[_a_ststepxwhole],eax
 ret
 public _D_PolysetSetUpForLineScan
_D_PolysetSetUpForLineScan:
 push esi
 push ebx
 mov dword ptr[_errorterm],offset -1
 mov edx,dword ptr[12+8+esp]
 mov eax,dword ptr[12+12+esp]
 mov esi,dword ptr[12+4+esp]
 sub edx,dword ptr[12+0+esp]
 sub esi,eax
 lea eax,dword ptr[15+edx]
 cmp eax,31
 ja LFloorDivMod
 lea ecx,dword ptr[15+esi]
 cmp ecx,31
 ja LFloorDivMod
 sal eax,5
 add eax,ecx
 sal eax,3
 mov dword ptr[_erroradjustdown],esi
 mov edx,dword ptr[_adivtab+eax]
 mov eax,dword ptr[_adivtab+4+eax]
 mov dword ptr[_ubasestep],edx
 mov dword ptr[_erroradjustup],eax
 jmp LSetupDone
LFloorDivMod:
 mov dword ptr[_erroradjustdown],esi
 fild dword ptr[_erroradjustdown]
 mov dword ptr[Ltemp],edx
 fild dword ptr[Ltemp]
 fldz
 fcomp st(1)
 fnstsw ax
 and ah,69
 je LFloorDivModNegative
 fld st(0)
 fdiv st(0),st(2)
 fldcw word ptr[floor_cw]
 frndint
 fist dword ptr[_ubasestep]
 fmulp st(2),st(0)
 fsubrp st(1),st(0)
 fistp dword ptr[_erroradjustup]
 fldcw word ptr[single_cw]
 jmp LSetupDone
LFloorDivModNegative:
 fchs
 fld st(0)
 fdiv st(0),st(2)
 fldcw word ptr[floor_cw]
 frndint
 fist dword ptr[_ubasestep]
 fmulp st(2),st(0)
 fsubrp st(1),st
 fistp dword ptr[_erroradjustup]
 fldcw word ptr[single_cw]
 neg dword ptr[_ubasestep]
 mov edx,dword ptr[_erroradjustup]
 test edx,edx
 je LSetupDone
 dec dword ptr[_ubasestep]
 sub esi,edx
 mov dword ptr[_erroradjustup],esi
LSetupDone:
 pop ebx
 pop esi
 ret
 public _D_PolysetScanLeftEdge
_D_PolysetScanLeftEdge:
 push ebp
 push esi
 push edi
 push ebx
 mov eax,dword ptr[4+16+esp]
 mov ecx,dword ptr[_d_sfrac]
 and eax,0FFFFh
 mov ebx,dword ptr[_d_ptex]
 or ecx,eax
 mov esi,dword ptr[_d_pedgespanpackage]
 mov edx,dword ptr[_d_tfrac]
 mov edi,dword ptr[_d_r]
 mov ebp,dword ptr[_d_zi]
LScanLoop:
 mov dword ptr[8+esi],ebx
 mov eax,dword ptr[_d_pdest]
 mov dword ptr[0+esi],eax
 mov eax,dword ptr[_d_pz]
 mov dword ptr[4+esi],eax
 mov eax,dword ptr[_d_aspancount]
 mov word ptr[24+esi],ax
 mov word ptr[26+esi],di
 mov eax,dword ptr[_d_g]
 mov word ptr[28+esi],ax
 mov eax,dword ptr[_d_b]
 mov word ptr[30+esi],ax
 mov dword ptr[20+esi],ebp
 mov dword ptr[12+esi],ecx
 mov dword ptr[16+esi],edx
 mov al,byte ptr[32+esi]
 add esi,32
 mov eax,dword ptr[_erroradjustup]
 mov dword ptr[_d_pedgespanpackage],esi
 mov esi,dword ptr[_errorterm]
 add esi,eax
 mov eax,dword ptr[_d_pdest]
 js LNoLeftEdgeTurnover
 sub esi,dword ptr[_erroradjustdown]
 add eax,dword ptr[_d_pdestextrastep]
 mov dword ptr[_errorterm],esi
 mov dword ptr[_d_pdest],eax
 mov eax,dword ptr[_d_pz]
 mov esi,dword ptr[_d_aspancount]
 add eax,dword ptr[_d_pzextrastep]
 add ecx,dword ptr[_d_sfracextrastep]
 adc ebx,dword ptr[_d_ptexextrastep]
 add esi,dword ptr[_d_countextrastep]
 mov dword ptr[_d_pz],eax
 mov eax,dword ptr[_d_tfracextrastep]
 mov dword ptr[_d_aspancount],esi
 add edx,eax
 jnc LSkip1
 add ebx,dword ptr[_d_affinetridesc+4]
LSkip1:
 add edi,dword ptr[_d_rextrastep]
 mov eax,dword ptr[_d_gextrastep]
 add dword ptr[_d_g],eax
 mov eax,dword ptr[_d_bextrastep]
 add dword ptr[_d_b],eax
 add ebp,dword ptr[_d_ziextrastep]
 mov esi,dword ptr[_d_pedgespanpackage]
 dec ecx
 test ecx,0FFFFh
 jnz LScanLoop
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret
LNoLeftEdgeTurnover:
 mov dword ptr[_errorterm],esi
 add eax,dword ptr[_d_pdestbasestep]
 mov dword ptr[_d_pdest],eax
 mov eax,dword ptr[_d_pz]
 mov esi,dword ptr[_d_aspancount]
 add eax,dword ptr[_d_pzbasestep]
 add ecx,dword ptr[_d_sfracbasestep]
 adc ebx,dword ptr[_d_ptexbasestep]
 add esi,dword ptr[_ubasestep]
 mov dword ptr[_d_pz],eax
 mov dword ptr[_d_aspancount],esi
 mov esi,dword ptr[_d_tfracbasestep]
 add edx,esi
 jnc LSkip2
 add ebx,dword ptr[_d_affinetridesc+4]
LSkip2:
 add edi,dword ptr[_d_rbasestep]
 mov eax,dword ptr[_d_gbasestep]
 add dword ptr[_d_g],eax
 mov eax,dword ptr[_d_bbasestep]
 add dword ptr[_d_b],eax
 add ebp,dword ptr[_d_zibasestep]
 mov esi,dword ptr[_d_pedgespanpackage]
 dec ecx
 test ecx,0FFFFh
 jnz LScanLoop
 pop ebx
 pop edi
 pop esi
 pop ebp
 ret
_TEXT ENDS
 END
