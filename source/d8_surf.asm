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
 externdef _ds_transluc
 externdef _ds_transluc16
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
 externdef ceil_cw
 externdef single_cw
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
 externdef _fadetable
 externdef _fadetable16
 externdef _fadetable16r
 externdef _fadetable16g
 externdef _fadetable16b
 externdef _fadetable32
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
 public _D_Surf8Start
_D_Surf8Start:
 align 4
 public _D_DrawSurfaceBlock8_mip0
_D_DrawSurfaceBlock8_mip0:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx,ds:dword ptr[_r_lightptr]
 mov eax,ds:dword ptr[_r_numvblocks]
 mov ds:dword ptr[sb_v],eax
 mov edi,ds:dword ptr[_prowdestbase]
 mov esi,ds:dword ptr[_pbasesource]
Lv_loop_mip0:
 mov eax,ds:dword ptr[ebx]
 mov edx,ds:dword ptr[4+ebx]
 mov ebp,eax
 mov ecx,ds:dword ptr[_r_lightwidth]
 mov ds:dword ptr[_lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx,ds:dword ptr[ebx+ecx*4]
 mov ds:dword ptr[_r_lightptr],ebx
 mov ecx,ds:dword ptr[4+ebx]
 mov ebx,ds:dword ptr[ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,4
 or ebp,0F0000000h
 sar ebx,4
 mov ds:dword ptr[_lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 or ebx,0F0000000h
 sub ecx,ecx
 mov ds:dword ptr[_lightdeltastep],ebx
 sub ebx,ebx
Lblockloop8_mip0:
 mov ds:dword ptr[_lightdelta],ebp
 mov cl,ds:byte ptr[14+esi]
 sar ebp,4
 mov bh,dh
 mov bl,ds:byte ptr[15+esi]
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LPatch0:
 mov bl,ds:byte ptr[13+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LPatch1:
 mov cl,ds:byte ptr[12+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LPatch2:
 mov bl,ds:byte ptr[11+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LPatch3:
 mov cl,ds:byte ptr[10+esi]
 mov ds:dword ptr[12+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LPatch4:
 mov bl,ds:byte ptr[9+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LPatch5:
 mov cl,ds:byte ptr[8+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LPatch6:
 mov bl,ds:byte ptr[7+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LPatch7:
 mov cl,ds:byte ptr[6+esi]
 mov ds:dword ptr[8+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LPatch8:
 mov bl,ds:byte ptr[5+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LPatch9:
 mov cl,ds:byte ptr[4+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LPatch10:
 mov bl,ds:byte ptr[3+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LPatch11:
 mov cl,ds:byte ptr[2+esi]
 mov ds:dword ptr[4+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LPatch12:
 mov bl,ds:byte ptr[1+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LPatch13:
 mov cl,ds:byte ptr[esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 mov ah,ds:byte ptr[12345678h+ebx]
LPatch14:
 mov edx,ds:dword ptr[_lightright]
 mov al,ds:byte ptr[12345678h+ecx]
LPatch15:
 mov ebp,ds:dword ptr[_lightdelta]
 mov ds:dword ptr[edi],eax
 add esi,ds:dword ptr[_sourcetstep]
 add edi,ds:dword ptr[_surfrowbytes]
 add edx,ds:dword ptr[_lightrightstep]
 add ebp,ds:dword ptr[_lightdeltastep]
 mov ds:dword ptr[_lightright],edx
 jc Lblockloop8_mip0
 cmp esi,ds:dword ptr[_r_sourcemax]
 jb LSkip_mip0
 sub esi,ds:dword ptr[_r_stepback]
LSkip_mip0:
 mov ebx,ds:dword ptr[_r_lightptr]
 dec ds:dword ptr[sb_v]
 jnz Lv_loop_mip0
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 align 4
 public _D_DrawSurfaceBlock8_mip1
_D_DrawSurfaceBlock8_mip1:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx,ds:dword ptr[_r_lightptr]
 mov eax,ds:dword ptr[_r_numvblocks]
 mov ds:dword ptr[sb_v],eax
 mov edi,ds:dword ptr[_prowdestbase]
 mov esi,ds:dword ptr[_pbasesource]
Lv_loop_mip1:
 mov eax,ds:dword ptr[ebx]
 mov edx,ds:dword ptr[4+ebx]
 mov ebp,eax
 mov ecx,ds:dword ptr[_r_lightwidth]
 mov ds:dword ptr[_lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx,ds:dword ptr[ebx+ecx*4]
 mov ds:dword ptr[_r_lightptr],ebx
 mov ecx,ds:dword ptr[4+ebx]
 mov ebx,ds:dword ptr[ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,3
 or ebp,070000000h
 sar ebx,3
 mov ds:dword ptr[_lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 or ebx,0F0000000h
 sub ecx,ecx
 mov ds:dword ptr[_lightdeltastep],ebx
 sub ebx,ebx
Lblockloop8_mip1:
 mov ds:dword ptr[_lightdelta],ebp
 mov cl,ds:byte ptr[6+esi]
 sar ebp,3
 mov bh,dh
 mov bl,ds:byte ptr[7+esi]
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LPatch22:
 mov bl,ds:byte ptr[5+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LPatch23:
 mov cl,ds:byte ptr[4+esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LPatch24:
 mov bl,ds:byte ptr[3+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LPatch25:
 mov cl,ds:byte ptr[2+esi]
 mov ds:dword ptr[4+edi],eax
 mov bh,dh
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LPatch26:
 mov bl,ds:byte ptr[1+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LPatch27:
 mov cl,ds:byte ptr[esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 mov ah,ds:byte ptr[12345678h+ebx]
LPatch28:
 mov edx,ds:dword ptr[_lightright]
 mov al,ds:byte ptr[12345678h+ecx]
LPatch29:
 mov ebp,ds:dword ptr[_lightdelta]
 mov ds:dword ptr[edi],eax
 mov eax,ds:dword ptr[_sourcetstep]
 add esi,eax
 mov eax,ds:dword ptr[_surfrowbytes]
 add edi,eax
 mov eax,ds:dword ptr[_lightrightstep]
 add edx,eax
 mov eax,ds:dword ptr[_lightdeltastep]
 add ebp,eax
 mov ds:dword ptr[_lightright],edx
 jc Lblockloop8_mip1
 cmp esi,ds:dword ptr[_r_sourcemax]
 jb LSkip_mip1
 sub esi,ds:dword ptr[_r_stepback]
LSkip_mip1:
 mov ebx,ds:dword ptr[_r_lightptr]
 dec ds:dword ptr[sb_v]
 jnz Lv_loop_mip1
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 align 4
 public _D_DrawSurfaceBlock8_mip2
_D_DrawSurfaceBlock8_mip2:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx,ds:dword ptr[_r_lightptr]
 mov eax,ds:dword ptr[_r_numvblocks]
 mov ds:dword ptr[sb_v],eax
 mov edi,ds:dword ptr[_prowdestbase]
 mov esi,ds:dword ptr[_pbasesource]
Lv_loop_mip2:
 mov eax,ds:dword ptr[ebx]
 mov edx,ds:dword ptr[4+ebx]
 mov ebp,eax
 mov ecx,ds:dword ptr[_r_lightwidth]
 mov ds:dword ptr[_lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx,ds:dword ptr[ebx+ecx*4]
 mov ds:dword ptr[_r_lightptr],ebx
 mov ecx,ds:dword ptr[4+ebx]
 mov ebx,ds:dword ptr[ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,2
 or ebp,030000000h
 sar ebx,2
 mov ds:dword ptr[_lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 or ebx,0F0000000h
 sub ecx,ecx
 mov ds:dword ptr[_lightdeltastep],ebx
 sub ebx,ebx
Lblockloop8_mip2:
 mov ds:dword ptr[_lightdelta],ebp
 mov cl,ds:byte ptr[2+esi]
 sar ebp,2
 mov bh,dh
 mov bl,ds:byte ptr[3+esi]
 add edx,ebp
 mov ch,dh
 add edx,ebp
 mov ah,ds:byte ptr[12345678h+ebx]
LPatch18:
 mov bl,ds:byte ptr[1+esi]
 mov al,ds:byte ptr[12345678h+ecx]
LPatch19:
 mov cl,ds:byte ptr[esi]
 mov bh,dh
 add edx,ebp
 ror eax,16
 mov ch,dh
 mov ah,ds:byte ptr[12345678h+ebx]
LPatch20:
 mov edx,ds:dword ptr[_lightright]
 mov al,ds:byte ptr[12345678h+ecx]
LPatch21:
 mov ebp,ds:dword ptr[_lightdelta]
 mov ds:dword ptr[edi],eax
 mov eax,ds:dword ptr[_sourcetstep]
 add esi,eax
 mov eax,ds:dword ptr[_surfrowbytes]
 add edi,eax
 mov eax,ds:dword ptr[_lightrightstep]
 add edx,eax
 mov eax,ds:dword ptr[_lightdeltastep]
 add ebp,eax
 mov ds:dword ptr[_lightright],edx
 jc Lblockloop8_mip2
 cmp esi,ds:dword ptr[_r_sourcemax]
 jb LSkip_mip2
 sub esi,ds:dword ptr[_r_stepback]
LSkip_mip2:
 mov ebx,ds:dword ptr[_r_lightptr]
 dec ds:dword ptr[sb_v]
 jnz Lv_loop_mip2
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 align 4
 public _D_DrawSurfaceBlock8_mip3
_D_DrawSurfaceBlock8_mip3:
 push ebp
 push edi
 push esi
 push ebx
 mov ebx,ds:dword ptr[_r_lightptr]
 mov eax,ds:dword ptr[_r_numvblocks]
 mov ds:dword ptr[sb_v],eax
 mov edi,ds:dword ptr[_prowdestbase]
 mov esi,ds:dword ptr[_pbasesource]
Lv_loop_mip3:
 mov eax,ds:dword ptr[ebx]
 mov edx,ds:dword ptr[4+ebx]
 mov ebp,eax
 mov ecx,ds:dword ptr[_r_lightwidth]
 mov ds:dword ptr[_lightright],edx
 sub ebp,edx
 and ebp,0FFFFFh
 lea ebx,ds:dword ptr[ebx+ecx*4]
 mov ds:dword ptr[_lightdelta],ebp
 mov ds:dword ptr[_r_lightptr],ebx
 mov ecx,ds:dword ptr[4+ebx]
 mov ebx,ds:dword ptr[ebx]
 sub ebx,eax
 sub ecx,edx
 sar ecx,1
 sar ebx,1
 mov ds:dword ptr[_lightrightstep],ecx
 sub ebx,ecx
 and ebx,0FFFFFh
 sar ebp,1
 or ebx,0F0000000h
 mov ds:dword ptr[_lightdeltastep],ebx
 sub ebx,ebx
 mov bl,ds:byte ptr[1+esi]
 sub ecx,ecx
 mov bh,dh
 mov cl,ds:byte ptr[esi]
 add edx,ebp
 mov ch,dh
 mov al,ds:byte ptr[12345678h+ebx]
LPatch16:
 mov edx,ds:dword ptr[_lightright]
 mov ds:byte ptr[1+edi],al
 mov al,ds:byte ptr[12345678h+ecx]
LPatch17:
 mov ds:byte ptr[edi],al
 mov eax,ds:dword ptr[_sourcetstep]
 add esi,eax
 mov eax,ds:dword ptr[_surfrowbytes]
 add edi,eax
 mov eax,ds:dword ptr[_lightdeltastep]
 mov ebp,ds:dword ptr[_lightdelta]
 mov cl,ds:byte ptr[esi]
 add ebp,eax
 mov eax,ds:dword ptr[_lightrightstep]
 sar ebp,1
 add edx,eax
 mov bh,dh
 mov bl,ds:byte ptr[1+esi]
 add edx,ebp
 mov ch,dh
 mov al,ds:byte ptr[12345678h+ebx]
LPatch30:
 mov edx,ds:dword ptr[_sourcetstep]
 mov ds:byte ptr[1+edi],al
 mov al,ds:byte ptr[12345678h+ecx]
LPatch31:
 mov ds:byte ptr[edi],al
 mov ebp,ds:dword ptr[_surfrowbytes]
 add esi,edx
 add edi,ebp
 cmp esi,ds:dword ptr[_r_sourcemax]
 jb LSkip_mip3
 sub esi,ds:dword ptr[_r_stepback]
LSkip_mip3:
 mov ebx,ds:dword ptr[_r_lightptr]
 dec ds:dword ptr[sb_v]
 jnz Lv_loop_mip3
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
 align 4
 public _D_DrawSurfaceBlock8RGB_mip0
_D_DrawSurfaceBlock8RGB_mip0:
 mov eax,ds:dword ptr[_pbasesource]
 sub esp,60
 mov edx,ds:dword ptr[_prowdestbase]
 push ebp
 push edi
 push esi
 push ebx
 xor esi,esi
 mov ds:dword ptr[36+esp],eax
 mov ds:dword ptr[32+esp],edx
 cmp esi,ds:dword ptr[_r_numvblocks]
 jge L1013
L1000:
 mov edx,ds:dword ptr[_r_lightptrr]
 mov ebx,ds:dword ptr[_r_lightwidth]
 sal ebx,2
 inc esi
 mov eax,ds:dword ptr[edx]
 mov ecx,ds:dword ptr[4+edx]
 mov ds:dword ptr[_lightrleft],eax
 mov ds:dword ptr[_lightrright],ecx
 add edx,ebx
 mov ds:dword ptr[_r_lightptrr],edx
 mov edi,ds:dword ptr[edx]
 sub edi,eax
 mov eax,edi
 shr eax,4
 mov ds:dword ptr[_lightrleftstep],eax
 mov eax,ds:dword ptr[4+edx]
 mov edx,ds:dword ptr[_r_lightptrg]
 sub eax,ecx
 shr eax,4
 mov ds:dword ptr[_lightrrightstep],eax
 mov eax,ds:dword ptr[edx]
 mov ecx,ds:dword ptr[4+edx]
 mov ds:dword ptr[_lightgleft],eax
 mov ds:dword ptr[_lightgright],ecx
 add edx,ebx
 mov ds:dword ptr[_r_lightptrg],edx
 mov edi,ds:dword ptr[edx]
 sub edi,eax
 mov eax,edi
 shr eax,4
 mov ds:dword ptr[_lightgleftstep],eax
 mov eax,ds:dword ptr[4+edx]
 mov edx,ds:dword ptr[_r_lightptrb]
 sub eax,ecx
 shr eax,4
 mov ds:dword ptr[_lightgrightstep],eax
 mov eax,ds:dword ptr[edx]
 mov ecx,ds:dword ptr[4+edx]
 mov ds:dword ptr[_lightbleft],eax
 mov ds:dword ptr[_lightbright],ecx
 add edx,ebx
 mov ds:dword ptr[_r_lightptrb],edx
 mov ebx,ds:dword ptr[edx]
 sub ebx,eax
 mov eax,ebx
 shr eax,4
 mov ds:dword ptr[_lightbleftstep],eax
 mov eax,ds:dword ptr[4+edx]
 sub eax,ecx
 shr eax,4
 mov ds:dword ptr[_lightbrightstep],eax
 xor ebx,ebx
 mov ds:dword ptr[28+esp],esi
L1004:
 mov ecx,ds:dword ptr[_lightrright]
 mov esi,ds:dword ptr[_lightrleft]
 mov edx,ds:dword ptr[_lightgright]
 mov edi,ds:dword ptr[_lightgleft]
 mov eax,ds:dword ptr[_lightbright]
 sub esi,ecx
 sar esi,4
 mov ds:dword ptr[56+esp],esi
 mov esi,ds:dword ptr[_lightbleft]
 sub edi,edx
 sar edi,4
 mov ds:dword ptr[48+esp],edi
 sub esi,eax
 sar esi,4
 mov ds:dword ptr[40+esp],esi
 mov ds:dword ptr[52+esp],ecx
 mov ds:dword ptr[44+esp],edx
 mov ebp,eax
 mov ds:dword ptr[60+esp],15
 inc ebx
 mov ds:dword ptr[24+esp],ebx
L1008:
 mov ecx,ds:dword ptr[52+esp]
 mov edi,ds:dword ptr[36+esp]
 mov eax,ds:dword ptr[60+esp]
 and ecx,65280
 xor ebx,ebx
 mov bl,ds:byte ptr[eax+edi]
 mov eax,ds:dword ptr[44+esp]
 add ecx,ebx
 and eax,65280
 add eax,ebx
 mov cx,ds:word ptr[12345678h+ecx*2]
LRPatch0:
 or cx,ds:word ptr[12345678h+eax*2]
LGPatch0:
 mov eax,ebp
 and eax,65280
 add eax,ebx
 or cx,ds:word ptr[12345678h+eax*2]
LBPatch0:
 and ecx,65535
 mov edx,ds:dword ptr[32+esp]
 mov al,ds:byte ptr[12345678h+ecx]
LTPatch0:
 mov ecx,ds:dword ptr[60+esp]
 mov ds:byte ptr[ecx+edx],al
 mov ebx,ds:dword ptr[56+esp]
 mov esi,ds:dword ptr[48+esp]
 add ds:dword ptr[52+esp],ebx
 mov edi,ecx
 mov ecx,ds:dword ptr[52+esp]
 mov eax,ds:dword ptr[36+esp]
 add ds:dword ptr[44+esp],esi
 add ebp,ds:dword ptr[40+esp]
 dec edi
 and ecx,65280
 xor ebx,ebx
 mov bl,ds:byte ptr[edi+eax]
 mov eax,ds:dword ptr[44+esp]
 add ecx,ebx
 and eax,65280
 add eax,ebx
 mov cx,ds:word ptr[12345678h+ecx*2]
LRPatch1:
 or cx,ds:word ptr[12345678h+eax*2]
LGPatch1:
 mov eax,ebp
 and eax,65280
 add eax,ebx
 or cx,ds:word ptr[12345678h+eax*2]
LBPatch1:
 and ecx,65535
 mov edx,ds:dword ptr[32+esp]
 mov al,ds:byte ptr[12345678h+ecx]
LTPatch1:
 mov ds:byte ptr[edi+edx],al
 mov ecx,ds:dword ptr[56+esp]
 mov ebx,ds:dword ptr[48+esp]
 add ds:dword ptr[52+esp],ecx
 add ds:dword ptr[44+esp],ebx
 add ebp,ds:dword ptr[40+esp]
 add ds:dword ptr[60+esp],offset -2
 jns L1008
 mov esi,ds:dword ptr[_sourcetstep]
 mov eax,ds:dword ptr[_lightrrightstep]
 add ds:dword ptr[_lightrright],eax
 mov eax,ds:dword ptr[_lightrleftstep]
 add ds:dword ptr[_lightrleft],eax
 mov eax,ds:dword ptr[_lightgrightstep]
 add ds:dword ptr[_lightgright],eax
 mov eax,ds:dword ptr[_lightgleftstep]
 add ds:dword ptr[_lightgleft],eax
 mov eax,ds:dword ptr[_lightbrightstep]
 add ds:dword ptr[_lightbright],eax
 mov eax,ds:dword ptr[_lightbleftstep]
 mov edi,ds:dword ptr[_surfrowbytes]
 mov ebx,ds:dword ptr[24+esp]
 add ds:dword ptr[36+esp],esi
 add ds:dword ptr[_lightbleft],eax
 add edx,edi
 mov ds:dword ptr[32+esp],edx
 cmp ebx,15
 jle L1004
 mov eax,ds:dword ptr[36+esp]
 cmp eax,ds:dword ptr[_r_sourcemax]
 jb L999
 mov edx,ds:dword ptr[_r_stepback]
 sub eax,edx
 mov ds:dword ptr[36+esp],eax
L999:
 mov esi,ds:dword ptr[28+esp]
 cmp esi,ds:dword ptr[_r_numvblocks]
 jl L1000
L1013:
 pop ebx
 pop esi
 pop edi
 pop ebp
 add esp,60
 ret
 align 4
 public _D_DrawSurfaceBlock8RGB_mip1
_D_DrawSurfaceBlock8RGB_mip1:
 mov eax,ds:dword ptr[_pbasesource]
 sub esp,60
 mov edx,ds:dword ptr[_prowdestbase]
 push ebp
 push edi
 push esi
 push ebx
 xor esi,esi
 mov ds:dword ptr[36+esp],eax
 mov ds:dword ptr[32+esp],edx
 cmp esi,ds:dword ptr[_r_numvblocks]
 jge L1035
L1022:
 mov edx,ds:dword ptr[_r_lightptrr]
 mov ebx,ds:dword ptr[_r_lightwidth]
 sal ebx,2
 inc esi
 mov eax,ds:dword ptr[edx]
 mov ecx,ds:dword ptr[4+edx]
 mov ds:dword ptr[_lightrleft],eax
 mov ds:dword ptr[_lightrright],ecx
 add edx,ebx
 mov ds:dword ptr[_r_lightptrr],edx
 mov edi,ds:dword ptr[edx]
 sub edi,eax
 mov eax,edi
 shr eax,3
 mov ds:dword ptr[_lightrleftstep],eax
 mov eax,ds:dword ptr[4+edx]
 mov edx,ds:dword ptr[_r_lightptrg]
 sub eax,ecx
 shr eax,3
 mov ds:dword ptr[_lightrrightstep],eax
 mov eax,ds:dword ptr[edx]
 mov ecx,ds:dword ptr[4+edx]
 mov ds:dword ptr[_lightgleft],eax
 mov ds:dword ptr[_lightgright],ecx
 add edx,ebx
 mov ds:dword ptr[_r_lightptrg],edx
 mov edi,ds:dword ptr[edx]
 sub edi,eax
 mov eax,edi
 shr eax,3
 mov ds:dword ptr[_lightgleftstep],eax
 mov eax,ds:dword ptr[4+edx]
 mov edx,ds:dword ptr[_r_lightptrb]
 sub eax,ecx
 shr eax,3
 mov ds:dword ptr[_lightgrightstep],eax
 mov eax,ds:dword ptr[edx]
 mov ecx,ds:dword ptr[4+edx]
 mov ds:dword ptr[_lightbleft],eax
 mov ds:dword ptr[_lightbright],ecx
 add edx,ebx
 mov ds:dword ptr[_r_lightptrb],edx
 mov ebx,ds:dword ptr[edx]
 sub ebx,eax
 mov eax,ebx
 shr eax,3
 mov ds:dword ptr[_lightbleftstep],eax
 mov eax,ds:dword ptr[4+edx]
 sub eax,ecx
 shr eax,3
 mov ds:dword ptr[_lightbrightstep],eax
 xor ebx,ebx
 mov ds:dword ptr[28+esp],esi
L1026:
 mov ecx,ds:dword ptr[_lightrright]
 mov esi,ds:dword ptr[_lightrleft]
 mov edx,ds:dword ptr[_lightgright]
 mov edi,ds:dword ptr[_lightgleft]
 mov eax,ds:dword ptr[_lightbright]
 sub esi,ecx
 sar esi,3
 mov ds:dword ptr[56+esp],esi
 mov esi,ds:dword ptr[_lightbleft]
 sub edi,edx
 sar edi,3
 mov ds:dword ptr[48+esp],edi
 sub esi,eax
 sar esi,3
 mov ds:dword ptr[40+esp],esi
 mov ds:dword ptr[52+esp],ecx
 mov ds:dword ptr[44+esp],edx
 mov ebp,eax
 mov ds:dword ptr[60+esp],7
 inc ebx
 mov ds:dword ptr[24+esp],ebx
L1030:
 mov ecx,ds:dword ptr[52+esp]
 mov edi,ds:dword ptr[36+esp]
 mov eax,ds:dword ptr[60+esp]
 and ecx,65280
 xor ebx,ebx
 mov bl,ds:byte ptr[eax+edi]
 mov eax,ds:dword ptr[44+esp]
 add ecx,ebx
 and eax,65280
 add eax,ebx
 mov cx,ds:word ptr[12345678h+ecx*2]
LRPatch2:
 or cx,ds:word ptr[12345678h+eax*2]
LGPatch2:
 mov eax,ebp
 and eax,65280
 add eax,ebx
 or cx,ds:word ptr[12345678h+eax*2]
LBPatch2:
 and ecx,65535
 mov edx,ds:dword ptr[32+esp]
 mov al,ds:byte ptr[12345678h+ecx]
LTPatch2:
 mov ecx,ds:dword ptr[60+esp]
 mov ds:byte ptr[ecx+edx],al
 mov ebx,ds:dword ptr[56+esp]
 mov esi,ds:dword ptr[48+esp]
 add ds:dword ptr[52+esp],ebx
 mov edi,ecx
 mov ecx,ds:dword ptr[52+esp]
 mov eax,ds:dword ptr[36+esp]
 add ds:dword ptr[44+esp],esi
 add ebp,ds:dword ptr[40+esp]
 dec edi
 and ecx,65280
 xor ebx,ebx
 mov bl,ds:byte ptr[edi+eax]
 mov eax,ds:dword ptr[44+esp]
 add ecx,ebx
 and eax,65280
 add eax,ebx
 mov cx,ds:word ptr[12345678h+ecx*2]
LRPatch3:
 or cx,ds:word ptr[12345678h+eax*2]
LGPatch3:
 mov eax,ebp
 and eax,65280
 add eax,ebx
 or cx,ds:word ptr[+eax*2]
LBPatch3:
 and ecx,65535
 mov edx,ds:dword ptr[32+esp]
 mov al,ds:byte ptr[12345678h+ecx]
LTPatch3:
 mov ds:byte ptr[edi+edx],al
 mov ecx,ds:dword ptr[56+esp]
 mov ebx,ds:dword ptr[48+esp]
 add ds:dword ptr[52+esp],ecx
 add ds:dword ptr[44+esp],ebx
 add ebp,ds:dword ptr[40+esp]
 add ds:dword ptr[60+esp],offset -2
 jns L1030
 mov esi,ds:dword ptr[_sourcetstep]
 mov eax,ds:dword ptr[_lightrrightstep]
 add ds:dword ptr[_lightrright],eax
 mov eax,ds:dword ptr[_lightrleftstep]
 add ds:dword ptr[_lightrleft],eax
 mov eax,ds:dword ptr[_lightgrightstep]
 add ds:dword ptr[_lightgright],eax
 mov eax,ds:dword ptr[_lightgleftstep]
 add ds:dword ptr[_lightgleft],eax
 mov eax,ds:dword ptr[_lightbrightstep]
 add ds:dword ptr[_lightbright],eax
 mov eax,ds:dword ptr[_lightbleftstep]
 mov edi,ds:dword ptr[_surfrowbytes]
 mov ebx,ds:dword ptr[24+esp]
 add ds:dword ptr[36+esp],esi
 add ds:dword ptr[_lightbleft],eax
 add edx,edi
 mov ds:dword ptr[32+esp],edx
 cmp ebx,7
 jle L1026
 mov eax,ds:dword ptr[36+esp]
 cmp eax,ds:dword ptr[_r_sourcemax]
 jb L1021
 mov edx,ds:dword ptr[_r_stepback]
 sub eax,edx
 mov ds:dword ptr[36+esp],eax
L1021:
 mov esi,ds:dword ptr[28+esp]
 cmp esi,ds:dword ptr[_r_numvblocks]
 jl L1022
L1035:
 pop ebx
 pop esi
 pop edi
 pop ebp
 add esp,60
 ret
 align 4
 public _D_DrawSurfaceBlock8RGB_mip2
_D_DrawSurfaceBlock8RGB_mip2:
 mov eax,ds:dword ptr[_pbasesource]
 sub esp,60
 mov edx,ds:dword ptr[_prowdestbase]
 push ebp
 push edi
 push esi
 push ebx
 xor esi,esi
 mov ds:dword ptr[36+esp],eax
 mov ds:dword ptr[32+esp],edx
 cmp esi,ds:dword ptr[_r_numvblocks]
 jge L1057
L1044:
 mov edx,ds:dword ptr[_r_lightptrr]
 mov ebx,ds:dword ptr[_r_lightwidth]
 sal ebx,2
 inc esi
 mov eax,ds:dword ptr[edx]
 mov ecx,ds:dword ptr[4+edx]
 mov ds:dword ptr[_lightrleft],eax
 mov ds:dword ptr[_lightrright],ecx
 add edx,ebx
 mov ds:dword ptr[_r_lightptrr],edx
 mov edi,ds:dword ptr[edx]
 sub edi,eax
 mov eax,edi
 shr eax,2
 mov ds:dword ptr[_lightrleftstep],eax
 mov eax,ds:dword ptr[4+edx]
 mov edx,ds:dword ptr[_r_lightptrg]
 sub eax,ecx
 shr eax,2
 mov ds:dword ptr[_lightrrightstep],eax
 mov eax,ds:dword ptr[edx]
 mov ecx,ds:dword ptr[4+edx]
 mov ds:dword ptr[_lightgleft],eax
 mov ds:dword ptr[_lightgright],ecx
 add edx,ebx
 mov ds:dword ptr[_r_lightptrg],edx
 mov edi,ds:dword ptr[edx]
 sub edi,eax
 mov eax,edi
 shr eax,2
 mov ds:dword ptr[_lightgleftstep],eax
 mov eax,ds:dword ptr[4+edx]
 mov edx,ds:dword ptr[_r_lightptrb]
 sub eax,ecx
 shr eax,2
 mov ds:dword ptr[_lightgrightstep],eax
 mov eax,ds:dword ptr[edx]
 mov ecx,ds:dword ptr[4+edx]
 mov ds:dword ptr[_lightbleft],eax
 mov ds:dword ptr[_lightbright],ecx
 add edx,ebx
 mov ds:dword ptr[_r_lightptrb],edx
 mov ebx,ds:dword ptr[edx]
 sub ebx,eax
 mov eax,ebx
 shr eax,2
 mov ds:dword ptr[_lightbleftstep],eax
 mov eax,ds:dword ptr[4+edx]
 sub eax,ecx
 shr eax,2
 mov ds:dword ptr[_lightbrightstep],eax
 xor ebx,ebx
 mov ds:dword ptr[28+esp],esi
L1048:
 mov ecx,ds:dword ptr[_lightrright]
 mov esi,ds:dword ptr[_lightrleft]
 mov edx,ds:dword ptr[_lightgright]
 mov edi,ds:dword ptr[_lightgleft]
 mov eax,ds:dword ptr[_lightbright]
 sub esi,ecx
 sar esi,2
 mov ds:dword ptr[56+esp],esi
 mov esi,ds:dword ptr[_lightbleft]
 sub edi,edx
 sar edi,2
 mov ds:dword ptr[48+esp],edi
 sub esi,eax
 sar esi,2
 mov ds:dword ptr[40+esp],esi
 mov ds:dword ptr[52+esp],ecx
 mov ds:dword ptr[44+esp],edx
 mov ebp,eax
 mov ds:dword ptr[60+esp],3
 inc ebx
 mov ds:dword ptr[24+esp],ebx
L1052:
 mov ecx,ds:dword ptr[52+esp]
 mov edi,ds:dword ptr[36+esp]
 mov eax,ds:dword ptr[60+esp]
 and ecx,65280
 xor ebx,ebx
 mov bl,ds:byte ptr[eax+edi]
 mov eax,ds:dword ptr[44+esp]
 add ecx,ebx
 and eax,65280
 add eax,ebx
 mov cx,ds:word ptr[12345678h+ecx*2]
LRPatch4:
 or cx,ds:word ptr[12345678h+eax*2]
LGPatch4:
 mov eax,ebp
 and eax,65280
 add eax,ebx
 or cx,ds:word ptr[+eax*2]
LBPatch4:
 and ecx,65535
 mov edx,ds:dword ptr[32+esp]
 mov al,ds:byte ptr[12345678h+ecx]
LTPatch4:
 mov ecx,ds:dword ptr[60+esp]
 mov ds:byte ptr[ecx+edx],al
 mov ebx,ds:dword ptr[56+esp]
 mov esi,ds:dword ptr[48+esp]
 add ds:dword ptr[52+esp],ebx
 mov edi,ecx
 mov ecx,ds:dword ptr[52+esp]
 mov eax,ds:dword ptr[36+esp]
 add ds:dword ptr[44+esp],esi
 add ebp,ds:dword ptr[40+esp]
 dec edi
 and ecx,65280
 xor ebx,ebx
 mov bl,ds:byte ptr[edi+eax]
 mov eax,ds:dword ptr[44+esp]
 add ecx,ebx
 and eax,65280
 add eax,ebx
 mov cx,ds:word ptr[12345678h+ecx*2]
LRPatch5:
 or cx,ds:word ptr[12345678h+eax*2]
LGPatch5:
 mov eax,ebp
 and eax,65280
 add eax,ebx
 or cx,ds:word ptr[12345678h+eax*2]
LBPatch5:
 and ecx,65535
 mov edx,ds:dword ptr[32+esp]
 mov al,ds:byte ptr[12345678h+ecx]
LTPatch5:
 mov ds:byte ptr[edi+edx],al
 mov ecx,ds:dword ptr[56+esp]
 mov ebx,ds:dword ptr[48+esp]
 add ds:dword ptr[52+esp],ecx
 add ds:dword ptr[44+esp],ebx
 add ebp,ds:dword ptr[40+esp]
 add ds:dword ptr[60+esp],offset -2
 jns L1052
 mov esi,ds:dword ptr[_sourcetstep]
 mov eax,ds:dword ptr[_lightrrightstep]
 add ds:dword ptr[_lightrright],eax
 mov eax,ds:dword ptr[_lightrleftstep]
 add ds:dword ptr[_lightrleft],eax
 mov eax,ds:dword ptr[_lightgrightstep]
 add ds:dword ptr[_lightgright],eax
 mov eax,ds:dword ptr[_lightgleftstep]
 add ds:dword ptr[_lightgleft],eax
 mov eax,ds:dword ptr[_lightbrightstep]
 add ds:dword ptr[_lightbright],eax
 mov eax,ds:dword ptr[_lightbleftstep]
 mov edi,ds:dword ptr[_surfrowbytes]
 mov ebx,ds:dword ptr[24+esp]
 add ds:dword ptr[36+esp],esi
 add ds:dword ptr[_lightbleft],eax
 add edx,edi
 mov ds:dword ptr[32+esp],edx
 cmp ebx,3
 jle L1048
 mov eax,ds:dword ptr[36+esp]
 cmp eax,ds:dword ptr[_r_sourcemax]
 jb L1043
 mov edx,ds:dword ptr[_r_stepback]
 sub eax,edx
 mov ds:dword ptr[36+esp],eax
L1043:
 mov esi,ds:dword ptr[28+esp]
 cmp esi,ds:dword ptr[_r_numvblocks]
 jl L1044
L1057:
 pop ebx
 pop esi
 pop edi
 pop ebp
 add esp,60
 ret
 align 4
 public _D_DrawSurfaceBlock8RGB_mip3
_D_DrawSurfaceBlock8RGB_mip3:
 mov eax,ds:dword ptr[_pbasesource]
 sub esp,44
 mov edx,ds:dword ptr[_prowdestbase]
 push ebp
 push edi
 push esi
 push ebx
 xor esi,esi
 mov ds:dword ptr[28+esp],eax
 mov ds:dword ptr[24+esp],edx
 cmp esi,ds:dword ptr[_r_numvblocks]
 jge L1079
L1066:
 mov edx,ds:dword ptr[_r_lightptrr]
 mov ebx,ds:dword ptr[_r_lightwidth]
 sal ebx,2
 inc esi
 mov eax,ds:dword ptr[edx]
 mov ecx,ds:dword ptr[4+edx]
 mov ds:dword ptr[_lightrleft],eax
 mov ds:dword ptr[_lightrright],ecx
 add edx,ebx
 mov ds:dword ptr[_r_lightptrr],edx
 mov edi,ds:dword ptr[edx]
 sub edi,eax
 mov eax,edi
 shr eax,1
 mov ds:dword ptr[_lightrleftstep],eax
 mov eax,ds:dword ptr[4+edx]
 mov edx,ds:dword ptr[_r_lightptrg]
 sub eax,ecx
 shr eax,1
 mov ds:dword ptr[_lightrrightstep],eax
 mov eax,ds:dword ptr[edx]
 mov ecx,ds:dword ptr[4+edx]
 mov ds:dword ptr[_lightgleft],eax
 mov ds:dword ptr[_lightgright],ecx
 add edx,ebx
 mov ds:dword ptr[_r_lightptrg],edx
 mov edi,ds:dword ptr[edx]
 sub edi,eax
 mov eax,edi
 shr eax,1
 mov ds:dword ptr[_lightgleftstep],eax
 mov eax,ds:dword ptr[4+edx]
 mov edx,ds:dword ptr[_r_lightptrb]
 sub eax,ecx
 shr eax,1
 mov ds:dword ptr[_lightgrightstep],eax
 mov eax,ds:dword ptr[edx]
 mov ecx,ds:dword ptr[4+edx]
 mov ds:dword ptr[_lightbleft],eax
 mov ds:dword ptr[_lightbright],ecx
 add edx,ebx
 mov ds:dword ptr[_r_lightptrb],edx
 mov ebx,ds:dword ptr[edx]
 sub ebx,eax
 mov eax,ebx
 shr eax,1
 mov ds:dword ptr[_lightbleftstep],eax
 mov eax,ds:dword ptr[4+edx]
 sub eax,ecx
 shr eax,1
 mov ds:dword ptr[_lightbrightstep],eax
 mov ds:dword ptr[44+esp],0
 mov ds:dword ptr[16+esp],esi
L1070:
 mov edi,ds:dword ptr[_lightrright]
 mov eax,ds:dword ptr[_lightrleft]
 mov esi,ds:dword ptr[_lightgright]
 mov edx,ds:dword ptr[_lightgleft]
 mov ds:dword ptr[20+esp],edi
 sub eax,edi
 mov edi,ds:dword ptr[_lightbright]
 mov ebx,ds:dword ptr[_lightbleft]
 sar eax,1
 mov ds:dword ptr[40+esp],eax
 sub edx,esi
 sar edx,1
 mov ds:dword ptr[36+esp],edx
 sub ebx,edi
 sar ebx,1
 mov ds:dword ptr[32+esp],ebx
 mov ecx,ds:dword ptr[20+esp]
 mov eax,ds:dword ptr[28+esp]
 and ecx,65280
 xor ebx,ebx
 mov bl,ds:byte ptr[1+eax]
 add ecx,ebx
 mov eax,esi
 and eax,65280
 add eax,ebx
 mov cx,ds:word ptr[12345678h+ecx*2]
LRPatch6:
 or cx,ds:word ptr[12345678h+eax*2]
LGPatch6:
 mov eax,edi
 and eax,65280
 add eax,ebx
 or cx,ds:word ptr[12345678h+eax*2]
LBPatch6:
 and ecx,65535
 mov edx,ds:dword ptr[24+esp]
 mov al,ds:byte ptr[12345678h+ecx]
LTPatch6:
 mov ds:byte ptr[1+edx],al
 mov eax,ds:dword ptr[20+esp]
 mov ebx,ds:dword ptr[28+esp]
 add eax,ds:dword ptr[40+esp]
 add esi,ds:dword ptr[36+esp]
 add edi,ds:dword ptr[32+esp]
 and eax,65280
 xor ecx,ecx
 and esi,65280
 and edi,65280
 mov cl,ds:byte ptr[ebx]
 lea ebx,ds:dword ptr[ecx+eax]
 add esi,ecx
 add edi,ecx
 mov dx,ds:word ptr[12345678h+ebx*2]
LRPatch7:
 or dx,ds:word ptr[12345678h+esi*2]
LGPatch7:
 or dx,ds:word ptr[12345678h+edi*2]
LBPatch7:
 and edx,65535
 mov edi,ds:dword ptr[24+esp]
 mov al,ds:byte ptr[12345678h+edx]
LTPatch7:
 mov ds:byte ptr[edi],al
 mov eax,ds:dword ptr[_sourcetstep]
 add ds:dword ptr[28+esp],eax
 mov eax,ds:dword ptr[_lightrrightstep]
 add ds:dword ptr[_lightrright],eax
 mov eax,ds:dword ptr[_lightrleftstep]
 add ds:dword ptr[_lightrleft],eax
 mov eax,ds:dword ptr[_lightgrightstep]
 add ds:dword ptr[_lightgright],eax
 mov eax,ds:dword ptr[_lightgleftstep]
 add ds:dword ptr[_lightgleft],eax
 mov eax,ds:dword ptr[_lightbrightstep]
 add ds:dword ptr[_lightbright],eax
 mov eax,ds:dword ptr[_lightbleftstep]
 mov edx,ds:dword ptr[_surfrowbytes]
 add ds:dword ptr[_lightbleft],eax
 add edi,edx
 mov ds:dword ptr[24+esp],edi
 inc ds:dword ptr[44+esp]
 cmp ds:dword ptr[44+esp],1
 jle L1070
 mov ebx,ds:dword ptr[28+esp]
 cmp ebx,ds:dword ptr[_r_sourcemax]
 jb L1065
 mov edi,ds:dword ptr[_r_stepback]
 sub ebx,edi
 mov ds:dword ptr[28+esp],ebx
L1065:
 mov esi,ds:dword ptr[16+esp]
 cmp esi,ds:dword ptr[_r_numvblocks]
 jl L1066
L1079:
 pop ebx
 pop esi
 pop edi
 pop ebp
 add esp,44
 ret
 public _D_Surf8End
_D_Surf8End:
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
 align 4
LTPatchTable:
 dd LTPatch0-4
 dd LTPatch1-4
 dd LTPatch2-4
 dd LTPatch3-4
 dd LTPatch4-4
 dd LTPatch5-4
 dd LTPatch6-4
 dd LTPatch7-4
 align 4
LRPatchTable:
 dd LRPatch0-4
 dd LRPatch1-4
 dd LRPatch2-4
 dd LRPatch3-4
 dd LRPatch4-4
 dd LRPatch5-4
 dd LRPatch6-4
 dd LRPatch7-4
 align 4
LGPatchTable:
 dd LGPatch0-4
 dd LGPatch1-4
 dd LGPatch2-4
 dd LGPatch3-4
 dd LGPatch4-4
 dd LGPatch5-4
 dd LGPatch6-4
 dd LGPatch7-4
 align 4
LBPatchTable:
 dd LBPatch0-4
 dd LBPatch1-4
 dd LBPatch2-4
 dd LBPatch3-4
 dd LBPatch4-4
 dd LBPatch5-4
 dd LBPatch6-4
 dd LBPatch7-4
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _D_Surf8Patch
_D_Surf8Patch:
 push ebx
 mov eax,ds:dword ptr[_fadetable]
 mov ebx,offset LPatchTable
 mov ecx,32
LPatchLoop:
 mov edx,ds:dword ptr[ebx]
 add ebx,4
 mov ds:dword ptr[edx],eax
 dec ecx
 jnz LPatchLoop
 mov eax,ds:dword ptr[_d_rgbtable]
 mov ebx,offset LTPatchTable
 mov ecx,8
LTPatchLoop:
 mov edx,ds:dword ptr[ebx]
 add ebx,4
 mov ds:dword ptr[edx],eax
 dec ecx
 jnz LTPatchLoop
 mov eax,ds:dword ptr[_fadetable16r]
 mov ebx,offset LRPatchTable
 mov ecx,8
LRPatchLoop:
 mov edx,ds:dword ptr[ebx]
 add ebx,4
 mov ds:dword ptr[edx],eax
 dec ecx
 jnz LRPatchLoop
 mov eax,ds:dword ptr[_fadetable16g]
 mov ebx,offset LGPatchTable
 mov ecx,8
LGPatchLoop:
 mov edx,ds:dword ptr[ebx]
 add ebx,4
 mov ds:dword ptr[edx],eax
 dec ecx
 jnz LGPatchLoop
 mov eax,ds:dword ptr[_fadetable16b]
 mov ebx,offset LBPatchTable
 mov ecx,8
LBPatchLoop:
 mov edx,ds:dword ptr[ebx]
 add ebx,4
 mov ds:dword ptr[edx],eax
 dec ecx
 jnz LBPatchLoop
 pop ebx
 ret
_TEXT ENDS
 END
