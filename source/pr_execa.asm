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
 externdef _D_DrawZSpan
 externdef _pr_stackPtr
 externdef _current_func
 externdef _PR_RFInvalidOpcode
 externdef _PR_DynamicCast
 externdef _TestFunction
_TEXT SEGMENT
 align 4
 public _RunFunction
_RunFunction:
 push ebp
 push edi
 push esi
 push ebx
 mov edi,dword ptr[4+16+esp]
 mov dword ptr[_current_func],edi
 test word ptr[14+edi],00001h
 jz LINTERPRET_FUNCTION
 call  dword ptr[4+edi]
 jmp LEND_RUN_FUNCTION
 align 4
LINTERPRET_FUNCTION:
 mov esi,dword ptr[_pr_stackPtr]
 movzx eax,word ptr[8+edi]
 movzx edx,word ptr[10+edi]
 sal eax,2
 sal edx,2
 sub esi,eax
 mov ebp,esi
 add esi,edx
 mov edi,dword ptr[4+edi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPCODE_TABLE:
 dd LOPC_DONE
 dd LOPC_RETURN
 dd LOPC_PUSHNUMBER
 dd LOPC_PUSHPOINTED
 dd LOPC_LOCALADDRESS
 dd LOPC_GLOBALADDRESS
 dd LOPC_ADD
 dd LOPC_SUBTRACT
 dd LOPC_MULTIPLY
 dd LOPC_DIVIDE
 dd LOPC_MODULUS
 dd LOPC_EQ
 dd LOPC_NE
 dd LOPC_LT
 dd LOPC_GT
 dd LOPC_LE
 dd LOPC_GE
 dd LOPC_ANDLOGICAL
 dd LOPC_ORLOGICAL
 dd LOPC_NEGATELOGICAL
 dd LOPC_ANDBITWISE
 dd LOPC_ORBITWISE
 dd LOPC_XORBITWISE
 dd LOPC_LSHIFT
 dd LOPC_RSHIFT
 dd LOPC_UNARYMINUS
 dd LOPC_BITINVERSE
 dd LOPC_CALL
 dd LOPC_GOTO
 dd LOPC_IFGOTO
 dd LOPC_IFNOTGOTO
 dd LOPC_CASEGOTO
 dd LOPC_DROP
 dd LOPC_ASSIGN
 dd LOPC_ADDVAR
 dd LOPC_SUBVAR
 dd LOPC_MULVAR
 dd LOPC_DIVVAR
 dd LOPC_MODVAR
 dd LOPC_ANDVAR
 dd LOPC_ORVAR
 dd LOPC_XORVAR
 dd LOPC_LSHIFTVAR
 dd LOPC_RSHIFTVAR
 dd LOPC_PREINC
 dd LOPC_PREDEC
 dd LOPC_POSTINC
 dd LOPC_POSTDEC
 dd LOPC_IFTOPGOTO
 dd LOPC_IFNOTTOPGOTO
 dd LOPC_ASSIGN_DROP
 dd LOPC_ADDVAR_DROP
 dd LOPC_SUBVAR_DROP
 dd LOPC_MULVAR_DROP
 dd LOPC_DIVVAR_DROP
 dd LOPC_MODVAR_DROP
 dd LOPC_ANDVAR_DROP
 dd LOPC_ORVAR_DROP
 dd LOPC_XORVAR_DROP
 dd LOPC_LSHIFTVAR_DROP
 dd LOPC_RSHIFTVAR_DROP
 dd LOPC_INC_DROP
 dd LOPC_DEC_DROP
 dd LOPC_FADD
 dd LOPC_FSUBTRACT
 dd LOPC_FMULTIPLY
 dd LOPC_FDIVIDE
 dd LOPC_FEQ
 dd LOPC_FNE
 dd LOPC_FLT
 dd LOPC_FGT
 dd LOPC_FLE
 dd LOPC_FGE
 dd LOPC_FUNARYMINUS
 dd LOPC_FADDVAR
 dd LOPC_FSUBVAR
 dd LOPC_FMULVAR
 dd LOPC_FDIVVAR
 dd LOPC_FADDVAR_DROP
 dd LOPC_FSUBVAR_DROP
 dd LOPC_FMULVAR_DROP
 dd LOPC_FDIVVAR_DROP
 dd LOPC_SWAP
 dd LOPC_ICALL
 dd LOPC_VPUSHPOINTED
 dd LOPC_VADD
 dd LOPC_VSUBTRACT
 dd LOPC_VPRESCALE
 dd LOPC_VPOSTSCALE
 dd LOPC_VISCALE
 dd LOPC_VEQ
 dd LOPC_VNE
 dd LOPC_VUNARYMINUS
 dd LOPC_VDROP
 dd LOPC_VASSIGN
 dd LOPC_VADDVAR
 dd LOPC_VSUBVAR
 dd LOPC_VSCALEVAR
 dd LOPC_VISCALEVAR
 dd LOPC_VASSIGN_DROP
 dd LOPC_VADDVAR_DROP
 dd LOPC_VSUBVAR_DROP
 dd LOPC_VSCALEVAR_DROP
 dd LOPC_VISCALEVAR_DROP
 dd LOPC_RETURNL
 dd LOPC_RETURNV
 dd LOPC_PUSHSTRING
 dd LOPC_COPY
 dd LOPC_SWAP3
 dd LOPC_PUSHFUNCTION
 dd LOPC_PUSHCLASSID
 dd LOPC_DYNAMIC_CAST
 dd LOPC_CASE_GOTO_CLASSID
 dd LOPC_PUSHNAME
 dd LOPC_CASE_GOTO_NAME
 dd LOPC_PUSHBOOL
 dd LOPC_ASSIGNBOOL
 align 4
LINC_STATEMENT_POINTER:
 add edi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_DONE:
 mov eax,offset _PR_RFInvalidOpcode
 call eax
 align 4
LOPC_RETURN:
 mov dword ptr[_pr_stackPtr],ebp
 jmp LEND_RUN_FUNCTION
 align 4
LOPC_PUSHNUMBER:
LOPC_GLOBALADDRESS:
LOPC_PUSHSTRING:
LOPC_PUSHFUNCTION:
LOPC_PUSHCLASSID:
LOPC_PUSHNAME:
 mov eax,dword ptr[edi]
 mov dword ptr[esi],eax
 add edi,4
 add esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_PUSHPOINTED:
 mov eax,dword ptr[-4+esi]
 mov eax,dword ptr[eax]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_LOCALADDRESS:
 mov eax,dword ptr[edi]
 lea eax,dword ptr[ebp+eax*4]
 mov dword ptr[esi],eax
 add edi,4
 add esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_ADD:
 sub esi,4
 mov eax,dword ptr[esi]
 add dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_SUBTRACT:
 sub esi,4
 mov eax,dword ptr[esi]
 sub dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_MULTIPLY:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 imul eax,dword ptr[esi]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_DIVIDE:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 cdq
 idiv dword ptr[esi]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_MODULUS:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 cdq
 idiv dword ptr[esi]
 mov dword ptr[-4+esi],edx
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_EQ:
 sub esi,4
 mov eax,dword ptr[esi]
 cmp dword ptr[-4+esi],eax
 sete al
 and eax,1
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_NE:
 sub esi,4
 mov eax,dword ptr[esi]
 cmp dword ptr[-4+esi],eax
 setne al
 and eax,1
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_LT:
 sub esi,4
 mov eax,dword ptr[esi]
 cmp dword ptr[-4+esi],eax
 setl al
 and eax,1
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_GT:
 sub esi,4
 mov eax,dword ptr[esi]
 cmp dword ptr[-4+esi],eax
 setg al
 and eax,1
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_LE:
 sub esi,4
 mov eax,dword ptr[esi]
 cmp dword ptr[-4+esi],eax
 setle al
 and eax,1
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_GE:
 sub esi,4
 mov eax,dword ptr[esi]
 cmp dword ptr[-4+esi],eax
 setge al
 and eax,1
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_ANDLOGICAL:
 sub esi,4
 cmp dword ptr[-4+esi],0
 je LAND_FALSE
 cmp dword ptr[esi],0
 je LAND_FALSE
 mov dword ptr[-4+esi],1
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
LAND_FALSE:
 mov dword ptr[-4+esi],0
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_ORLOGICAL:
 sub esi,4
 cmp dword ptr[-4+esi],0
 jne LOR_TRUE
 cmp dword ptr[esi],0
 jne LOR_TRUE
 mov dword ptr[-4+esi],0
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
LOR_TRUE:
 mov dword ptr[-4+esi],1
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_NEGATELOGICAL:
 cmp dword ptr[-4+esi],0
 sete al
 and eax,1
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_ANDBITWISE:
 sub esi,4
 mov eax,dword ptr[esi]
 and dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_ORBITWISE:
 sub esi,4
 mov eax,dword ptr[esi]
 or dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_XORBITWISE:
 sub esi,4
 mov eax,dword ptr[esi]
 xor dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_LSHIFT:
 sub esi,4
 mov ecx,dword ptr[esi]
 sal dword ptr[-4+esi],cl
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_RSHIFT:
 sub esi,4
 mov ecx,dword ptr[esi]
 sar dword ptr[-4+esi],cl
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_UNARYMINUS:
 neg dword ptr[-4+esi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_BITINVERSE:
 not dword ptr[-4+esi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_CALL:
 mov dword ptr[_pr_stackPtr],esi
 push dword ptr[edi]
 add edi,4
 call _RunFunction
 add esp,4
 mov eax,dword ptr[4+16+esp]
 mov dword ptr[_current_func],eax
 mov esi,dword ptr[_pr_stackPtr]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_GOTO:
 mov edi,dword ptr[edi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_IFGOTO:
 sub esi,4
 cmp dword ptr[esi],0
 je LINC_STATEMENT_POINTER
 mov edi,dword ptr[edi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_IFNOTGOTO:
 sub esi,4
 cmp dword ptr[esi],0
 jne LINC_STATEMENT_POINTER
 mov edi,dword ptr[edi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_CASEGOTO:
LOPC_CASE_GOTO_CLASSID:
LOPC_CASE_GOTO_NAME:
 mov eax,dword ptr[edi]
 add edi,4
 cmp eax,dword ptr[-4+esi]
 jne LINC_STATEMENT_POINTER
 mov edi,dword ptr[edi]
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_DROP:
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_ASSIGN:
 sub esi,4
 mov edx,dword ptr[-4+esi]
 mov eax,dword ptr[esi]
 mov dword ptr[edx],eax
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_ADDVAR:
 sub esi,4
 mov edx,dword ptr[-4+esi]
 mov eax,dword ptr[esi]
 add dword ptr[edx],eax
 mov eax,dword ptr[edx]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_SUBVAR:
 sub esi,4
 mov edx,dword ptr[-4+esi]
 mov eax,dword ptr[esi]
 sub dword ptr[edx],eax
 mov eax,dword ptr[edx]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_MULVAR:
 sub esi,4
 mov edx,dword ptr[-4+esi]
 mov eax,dword ptr[esi]
 imul eax,dword ptr[edx]
 mov dword ptr[edx],eax
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_DIVVAR:
 sub esi,4
 mov ecx,dword ptr[-4+esi]
 mov eax,dword ptr[ecx]
 cdq
 idiv dword ptr[esi]
 mov dword ptr[ecx],eax
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_MODVAR:
 sub esi,4
 mov ecx,dword ptr[-4+esi]
 mov eax,dword ptr[ecx]
 cdq
 idiv dword ptr[esi]
 mov dword ptr[ecx],edx
 mov dword ptr[-4+esi],edx
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_ANDVAR:
 sub esi,4
 mov edx,dword ptr[-4+esi]
 mov eax,dword ptr[esi]
 and dword ptr[edx],eax
 mov eax,dword ptr[edx]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_ORVAR:
 sub esi,4
 mov edx,dword ptr[-4+esi]
 mov eax,dword ptr[esi]
 or dword ptr[edx],eax
 mov eax,dword ptr[edx]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_XORVAR:
 sub esi,4
 mov edx,dword ptr[-4+esi]
 mov eax,dword ptr[esi]
 xor dword ptr[edx],eax
 mov eax,dword ptr[edx]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_LSHIFTVAR:
 sub esi,4
 mov edx,dword ptr[-4+esi]
 mov ecx,dword ptr[esi]
 sal dword ptr[edx],cl
 mov eax,dword ptr[edx]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_RSHIFTVAR:
 sub esi,4
 mov edx,dword ptr[-4+esi]
 mov ecx,dword ptr[esi]
 sar dword ptr[edx],cl
 mov eax,dword ptr[edx]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_PREINC:
 mov edx,dword ptr[-4+esi]
 inc dword ptr[edx]
 mov eax,dword ptr[edx]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_PREDEC:
 mov edx,dword ptr[-4+esi]
 dec dword ptr[edx]
 mov eax,dword ptr[edx]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_POSTINC:
 mov edx,dword ptr[-4+esi]
 mov eax,dword ptr[edx]
 mov dword ptr[-4+esi],eax
 inc dword ptr[edx]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_POSTDEC:
 mov edx,dword ptr[-4+esi]
 mov eax,dword ptr[edx]
 mov dword ptr[-4+esi],eax
 dec dword ptr[edx]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_IFTOPGOTO:
 cmp dword ptr[-4+esi],0
 je LINC_STATEMENT_POINTER
 mov edi,dword ptr[edi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_IFNOTTOPGOTO:
 cmp dword ptr[-4+esi],0
 jne LINC_STATEMENT_POINTER
 mov edi,dword ptr[edi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_ASSIGN_DROP:
 sub esi,4
 mov edx,dword ptr[-4+esi]
 mov eax,dword ptr[esi]
 mov dword ptr[edx],eax
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_ADDVAR_DROP:
 sub esi,4
 mov edx,dword ptr[-4+esi]
 mov eax,dword ptr[esi]
 add dword ptr[edx],eax
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_SUBVAR_DROP:
 sub esi,4
 mov edx,dword ptr[-4+esi]
 mov eax,dword ptr[esi]
 sub dword ptr[edx],eax
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_MULVAR_DROP:
 sub esi,4
 mov edx,dword ptr[-4+esi]
 mov eax,dword ptr[edx]
 imul eax,dword ptr[esi]
 mov dword ptr[edx],eax
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_DIVVAR_DROP:
 sub esi,4
 mov ecx,dword ptr[-4+esi]
 mov eax,dword ptr[ecx]
 cdq
 idiv dword ptr[esi]
 mov dword ptr[ecx],eax
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_MODVAR_DROP:
 sub esi,4
 mov ecx,dword ptr[-4+esi]
 mov eax,dword ptr[ecx]
 cdq
 idiv dword ptr[esi]
 mov dword ptr[ecx],edx
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_ANDVAR_DROP:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 mov ecx,dword ptr[esi]
 and dword ptr[eax],ecx
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_ORVAR_DROP:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 mov ecx,dword ptr[esi]
 or dword ptr[eax],ecx
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_XORVAR_DROP:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 mov ecx,dword ptr[esi]
 xor dword ptr[eax],ecx
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_LSHIFTVAR_DROP:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 mov ecx,dword ptr[esi]
 sal dword ptr[eax],cl
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_RSHIFTVAR_DROP:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 mov ecx,dword ptr[esi]
 sar dword ptr[eax],cl
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_INC_DROP:
 mov edx,dword ptr[-4+esi]
 inc dword ptr[edx]
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_DEC_DROP:
 mov edx,dword ptr[-4+esi]
 dec dword ptr[edx]
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FADD:
 sub esi,4
 fld dword ptr[-4+esi]
 fadd dword ptr[esi]
 fstp dword ptr[-4+esi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FSUBTRACT:
 sub esi,4
 fld dword ptr[-4+esi]
 fsub dword ptr[esi]
 fstp dword ptr[-4+esi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FMULTIPLY:
 sub esi,4
 fld dword ptr[-4+esi]
 fmul dword ptr[esi]
 fstp dword ptr[-4+esi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FDIVIDE:
 sub esi,4
 fld dword ptr[-4+esi]
 fdiv dword ptr[esi]
 fstp dword ptr[-4+esi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FEQ:
 sub esi,4
 fld dword ptr[-4+esi]
 fcomp dword ptr[esi]
 fnstsw ax
 sahf
 sete dl
 and edx,1
 mov dword ptr[-4+esi],edx
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FNE:
 sub esi,4
 fld dword ptr[-4+esi]
 fcomp dword ptr[esi]
 fnstsw ax
 sahf
 setne dl
 and edx,1
 mov dword ptr[-4+esi],edx
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FLT:
 sub esi,4
 fld dword ptr[-4+esi]
 fcomp dword ptr[esi]
 fnstsw ax
 sahf
 setb dl
 and edx,1
 mov dword ptr[-4+esi],edx
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FGT:
 sub esi,4
 fld dword ptr[-4+esi]
 fcomp dword ptr[esi]
 fnstsw ax
 sahf
 seta dl
 and edx,1
 mov dword ptr[-4+esi],edx
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FLE:
 sub esi,4
 fld dword ptr[-4+esi]
 fcomp dword ptr[esi]
 fnstsw ax
 sahf
 setbe dl
 and edx,1
 mov dword ptr[-4+esi],edx
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FGE:
 sub esi,4
 fld dword ptr[-4+esi]
 fcomp dword ptr[esi]
 fnstsw ax
 sahf
 setnb dl
 and edx,1
 mov dword ptr[-4+esi],edx
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FUNARYMINUS:
 fld dword ptr[-4+esi]
 fchs
 fstp dword ptr[-4+esi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FADDVAR:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 fld dword ptr[eax]
 fadd dword ptr[esi]
 fstp dword ptr[eax]
 mov eax,dword ptr[eax]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FSUBVAR:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 fld dword ptr[eax]
 fsub dword ptr[esi]
 fstp dword ptr[eax]
 mov eax,dword ptr[eax]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FMULVAR:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 fld dword ptr[eax]
 fmul dword ptr[esi]
 fstp dword ptr[eax]
 mov eax,dword ptr[eax]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FDIVVAR:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 fld dword ptr[eax]
 fdiv dword ptr[esi]
 fstp dword ptr[eax]
 mov eax,dword ptr[eax]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FADDVAR_DROP:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 fld dword ptr[eax]
 fadd dword ptr[esi]
 fstp dword ptr[eax]
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FSUBVAR_DROP:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 fld dword ptr[eax]
 fsub dword ptr[esi]
 fstp dword ptr[eax]
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FMULVAR_DROP:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 fld dword ptr[eax]
 fmul dword ptr[esi]
 fstp dword ptr[eax]
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_FDIVVAR_DROP:
 sub esi,4
 mov eax,dword ptr[-4+esi]
 fld dword ptr[eax]
 fdiv dword ptr[esi]
 fstp dword ptr[eax]
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_SWAP:
 mov eax,dword ptr[-8+esi]
 mov edx,dword ptr[-4+esi]
 mov dword ptr[-8+esi],edx
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_ICALL:
 sub esi,4
 push dword ptr[esi]
 mov dword ptr[_pr_stackPtr],esi
 call _RunFunction
 add esp,4
 mov eax,dword ptr[4+16+esp]
 mov dword ptr[_current_func],eax
 mov esi,dword ptr[_pr_stackPtr]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VPUSHPOINTED:
 add esi,8
 mov edx,dword ptr[-12+esi]
 mov eax,dword ptr[edx]
 mov dword ptr[-12+esi],eax
 mov eax,dword ptr[4+edx]
 mov dword ptr[-8+esi],eax
 mov eax,dword ptr[8+edx]
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VADD:
 fld dword ptr[-24+esi]
 fadd dword ptr[-12+esi]
 fstp dword ptr[-24+esi]
 fld dword ptr[-20+esi]
 fadd dword ptr[-8+esi]
 fstp dword ptr[-20+esi]
 fld dword ptr[-16+esi]
 fadd dword ptr[-4+esi]
 fstp dword ptr[-16+esi]
 add esi,offset -12
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VSUBTRACT:
 fld dword ptr[-24+esi]
 fsub dword ptr[-12+esi]
 fstp dword ptr[-24+esi]
 fld dword ptr[-20+esi]
 fsub dword ptr[-8+esi]
 fstp dword ptr[-20+esi]
 fld dword ptr[-16+esi]
 fsub dword ptr[-4+esi]
 fstp dword ptr[-16+esi]
 add esi,offset -12
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VPRESCALE:
 fld dword ptr[-16+esi]
 fld st(0)
 fld st(1)
 fmul dword ptr[-12+esi]
 fstp dword ptr[-16+esi]
 fmul dword ptr[-8+esi]
 fstp dword ptr[-12+esi]
 fmul dword ptr[-4+esi]
 fstp dword ptr[-8+esi]
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VPOSTSCALE:
 fld dword ptr[-4+esi]
 fld st(0)
 fld st(1)
 fmul dword ptr[-16+esi]
 fstp dword ptr[-16+esi]
 fmul dword ptr[-12+esi]
 fstp dword ptr[-12+esi]
 fmul dword ptr[-8+esi]
 fstp dword ptr[-8+esi]
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VISCALE:
 fld dword ptr[-16+esi]
 fdiv dword ptr[-4+esi]
 fstp dword ptr[-16+esi]
 fld dword ptr[-12+esi]
 fdiv dword ptr[-4+esi]
 fstp dword ptr[-12+esi]
 fld dword ptr[-8+esi]
 fdiv dword ptr[-4+esi]
 fstp dword ptr[-8+esi]
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VEQ:
 fld dword ptr[-24+esi]
 fld dword ptr[-12+esi]
fucompp
 fnstsw ax
 sahf
 jne VEQ_FALSE
 fld dword ptr[-20+esi]
 fld dword ptr[-8+esi]
fucompp
 fnstsw ax
 sahf
 jne VEQ_FALSE
 fld dword ptr[-16+esi]
 fld dword ptr[-4+esi]
fucompp
 fnstsw ax
 sahf
 jne VEQ_FALSE
 mov dword ptr[-24+esi],1
 add esi,offset -20
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
VEQ_FALSE:
 mov dword ptr[-24+esi],0
 add esi,offset -20
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VNE:
 fld dword ptr[-24+esi]
 fld dword ptr[-12+esi]
fucompp
 fnstsw ax
 sahf
 jne LVNE_TRUE
 fld dword ptr[-20+esi]
 fld dword ptr[-8+esi]
fucompp
 fnstsw ax
 sahf
 jne LVNE_TRUE
 fld dword ptr[-16+esi]
 fld dword ptr[-4+esi]
fucompp
 fnstsw ax
 sahf
 jne LVNE_TRUE
 mov dword ptr[-24+esi],0
 add esi,offset -20
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
LVNE_TRUE:
 mov dword ptr[-24+esi],1
 add esi,offset -20
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VUNARYMINUS:
 fld dword ptr[-12+esi]
 fchs
 fstp dword ptr[-12+esi]
 fld dword ptr[-8+esi]
 fchs
 fstp dword ptr[-8+esi]
 fld dword ptr[-4+esi]
 fchs
 fstp dword ptr[-4+esi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VDROP:
 add esi,offset -12
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VASSIGN:
 mov edx,dword ptr[-16+esi]
 mov eax,dword ptr[-12+esi]
 mov dword ptr[edx],eax
 mov dword ptr[-16+esi],eax
 mov eax,dword ptr[-8+esi]
 mov dword ptr[4+edx],eax
 mov dword ptr[-12+esi],eax
 mov eax,dword ptr[-4+esi]
 mov dword ptr[8+edx],eax
 mov dword ptr[-8+esi],eax
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VADDVAR:
 mov edx,dword ptr[-16+esi]
 fld dword ptr[edx]
 fadd dword ptr[-12+esi]
 fst dword ptr[edx]
 fstp dword ptr[-16+esi]
 fld dword ptr[4+edx]
 fadd dword ptr[-8+esi]
 fst dword ptr[4+edx]
 fstp dword ptr[-12+esi]
 fld dword ptr[8+edx]
 fadd dword ptr[-4+esi]
 fst dword ptr[8+edx]
 fstp dword ptr[-8+esi]
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VSUBVAR:
 mov edx,dword ptr[-16+esi]
 fld dword ptr[edx]
 fsub dword ptr[-12+esi]
 fst dword ptr[edx]
 fstp dword ptr[-16+esi]
 fld dword ptr[4+edx]
 fsub dword ptr[-8+esi]
 fst dword ptr[4+edx]
 fstp dword ptr[-12+esi]
 fld dword ptr[8+edx]
 fsub dword ptr[-4+esi]
 fst dword ptr[8+edx]
 fstp dword ptr[-8+esi]
 sub esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VSCALEVAR:
 add esi,4
 mov edx,dword ptr[-12+esi]
 fld dword ptr[-8+esi]
 fld dword ptr[edx]
 fmul st,st(1)
 fst dword ptr[edx]
 fstp dword ptr[-12+esi]
 fld dword ptr[4+edx]
 fmul st,st(1)
 fst dword ptr[4+edx]
 fstp dword ptr[-8+esi]
 fmul dword ptr[8+edx]
 fst dword ptr[8+edx]
 fstp dword ptr[-4+esi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VISCALEVAR:
 add esi,4
 mov edx,dword ptr[-12+esi]
 fld dword ptr[-8+esi]
 fld dword ptr[edx]
 fdiv st,st(1)
 fst dword ptr[edx]
 fstp dword ptr[-12+esi]
 fld dword ptr[4+edx]
 fdiv st,st(1)
 fst dword ptr[4+edx]
 fstp dword ptr[-8+esi]
 fdivr dword ptr[8+edx]
 fst dword ptr[8+edx]
 fstp dword ptr[-4+esi]
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VASSIGN_DROP:
 mov edx,dword ptr[-16+esi]
 mov eax,dword ptr[-12+esi]
 mov dword ptr[edx],eax
 mov eax,dword ptr[-8+esi]
 mov dword ptr[4+edx],eax
 mov eax,dword ptr[-4+esi]
 mov dword ptr[8+edx],eax
 add esi,offset -16
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VADDVAR_DROP:
 mov edx,dword ptr[-16+esi]
 fld dword ptr[edx]
 fadd dword ptr[-12+esi]
 fstp dword ptr[edx]
 fld dword ptr[4+edx]
 fadd dword ptr[-8+esi]
 fstp dword ptr[4+edx]
 fld dword ptr[8+edx]
 fadd dword ptr[-4+esi]
 fstp dword ptr[8+edx]
 add esi,offset -16
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VSUBVAR_DROP:
 mov edx,dword ptr[-16+esi]
 fld dword ptr[edx]
 fsub dword ptr[-12+esi]
 fstp dword ptr[edx]
 fld dword ptr[4+edx]
 fsub dword ptr[-8+esi]
 fstp dword ptr[4+edx]
 fld dword ptr[8+edx]
 fsub dword ptr[-4+esi]
 fstp dword ptr[8+edx]
 add esi,offset -16
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VSCALEVAR_DROP:
 mov edx,dword ptr[-8+esi]
 fld dword ptr[-4+esi]
 fld dword ptr[edx]
 fmul st,st(1)
 fstp dword ptr[edx]
 fld dword ptr[4+edx]
 fmul st,st(1)
 fstp dword ptr[4+edx]
 fmul dword ptr[8+edx]
 fstp dword ptr[8+edx]
 add esi,offset -8
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_VISCALEVAR_DROP:
 mov edx,dword ptr[-8+esi]
 fld dword ptr[-4+esi]
 fld dword ptr[edx]
 fdiv st,st(1)
 fstp dword ptr[edx]
 fld dword ptr[4+edx]
 fdiv st,st(1)
 fstp dword ptr[4+edx]
 fdivr dword ptr[8+edx]
 fstp dword ptr[8+edx]
 add esi,offset -8
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_RETURNL:
 mov eax,dword ptr[-4+esi]
 mov dword ptr[ebp],eax
 lea esi,dword ptr[4+ebp]
 mov dword ptr[_pr_stackPtr],esi
 jmp LEND_RUN_FUNCTION
 align 4
LOPC_RETURNV:
 mov eax,dword ptr[-12+esi]
 mov dword ptr[ebp],eax
 mov eax,dword ptr[-8+esi]
 mov dword ptr[4+ebp],eax
 mov eax,dword ptr[-4+esi]
 mov dword ptr[8+ebp],eax
 lea esi,dword ptr[12+ebp]
 mov dword ptr[_pr_stackPtr],esi
 jmp LEND_RUN_FUNCTION
 align 4
LOPC_COPY:
 mov eax,dword ptr[-4+esi]
 mov dword ptr[esi],eax
 add esi,4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_SWAP3:
 mov eax,dword ptr[-16+esi]
 mov edx,dword ptr[-12+esi]
 mov dword ptr[-16+esi],edx
 mov edx,dword ptr[-8+esi]
 mov dword ptr[-12+esi],edx
 mov edx,dword ptr[-4+esi]
 mov dword ptr[-8+esi],edx
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_DYNAMIC_CAST:
 mov eax,dword ptr[edi]
 mov edx,dword ptr[-4+esi]
 push eax
 push edx
 add edi,4
 mov eax,offset _PR_DynamicCast
 call eax
 mov dword ptr[-4+esi],eax
 add esp,8
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_PUSHBOOL:
 mov eax,dword ptr[edi]
 mov edx,dword ptr[-4+esi]
 add edi,4
 test dword ptr[edx],eax
 setne al
 and eax,1
 mov dword ptr[-4+esi],eax
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
 align 4
LOPC_ASSIGNBOOL:
 mov edx,dword ptr[edi]
 add esi,offset -4
 add edi,4
 cmp dword ptr[esi],0
 je LABOOL_FALSE
 mov eax,dword ptr[-4+esi]
 or dword ptr[eax],edx
 add esi,offset -4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
LABOOL_FALSE:
 mov eax,dword ptr[-4+esi]
 xor edx,offset -1
 and dword ptr[eax],edx
 add esi,offset -4
 mov eax,dword ptr[edi]
 add edi,4
 jmp  dword ptr[LOPCODE_TABLE+eax*4]
LEND_RUN_FUNCTION:
 pop ebx
 pop esi
 pop edi
 pop ebp
 ret
_TEXT ENDS
 END
