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
 align 4
fpenv:
 dd 0, 0, 0, 0, 0, 0, 0, 0
_DATA ENDS
_TEXT SEGMENT
 public _MaskExceptions
_MaskExceptions:
 fnstenv ds:dword ptr[fpenv]
 or ds:dword ptr[fpenv],03Fh
 fldenv ds:dword ptr[fpenv]
 ret
_TEXT ENDS
_DATA SEGMENT
 align 4
 public ceil_cw
 public single_cw
ceil_cw dd 0
single_cw dd 0
full_cw dd 0
cw dd 0
pushed_cw dd 0
_DATA ENDS
_TEXT SEGMENT
 align 4
 public _Sys_SetFPCW
_Sys_SetFPCW:
 fnstcw ds:word ptr[cw]
 mov eax,ds:dword ptr[cw]
 and ah,0F0h
 or ah,003h
 mov ds:dword ptr[full_cw],eax
 and ah,0F0h
 or ah,00Ch
 mov ds:dword ptr[single_cw],eax
 and ah,0F0h
 or ah,008h
 mov ds:dword ptr[ceil_cw],eax
 ret
 align 4
 public _Sys_LowFPPrecision
_Sys_LowFPPrecision:
 fldcw ds:word ptr[single_cw]
 ret
 align 4
 public _Sys_HighFPPrecision
_Sys_HighFPPrecision:
 fldcw ds:word ptr[full_cw]
 ret
 align 4
 public _Sys_PushFPCW_SetHigh
_Sys_PushFPCW_SetHigh:
 fnstcw ds:word ptr[pushed_cw]
 fldcw ds:word ptr[full_cw]
 ret
 align 4
 public _Sys_PopFPCW
_Sys_PopFPCW:
 fldcw ds:word ptr[pushed_cw]
 ret
_TEXT ENDS
 END
