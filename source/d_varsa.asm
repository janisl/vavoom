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
 align 4
 public _d_sdivzstepu
 public _d_tdivzstepu
 public _d_zistepu
 public _d_sdivzstepv
 public _d_tdivzstepv
 public _d_zistepv
 public _d_sdivzorigin
 public _d_tdivzorigin
 public _d_ziorigin
_d_sdivzstepu dd 0
_d_tdivzstepu dd 0
_d_zistepu dd 0
_d_sdivzstepv dd 0
_d_tdivzstepv dd 0
_d_zistepv dd 0
_d_sdivzorigin dd 0
_d_tdivzorigin dd 0
_d_ziorigin dd 0
 public _sadjust
 public _tadjust
 public _bbextents
 public _bbextentt
_sadjust dd 0
_tadjust dd 0
_bbextents dd 0
_bbextentt dd 0
 public _cacheblock
 public _cachewidth
 public _ds_transluc
 public _ds_transluc16
_cacheblock dd 0
_cachewidth dd 0
_ds_transluc dd 0
_ds_transluc16 dd 0
 public _d_rowbytes
 public _d_zrowbytes
_d_rowbytes dd 0
_d_zrowbytes dd 0
 public sdivz8stepu
 public tdivz8stepu
 public zi8stepu
 public sdivz16stepu
 public tdivz16stepu
 public zi16stepu
sdivz8stepu dd 0
tdivz8stepu dd 0
zi8stepu dd 0
sdivz16stepu dd 0
tdivz16stepu dd 0
zi16stepu dd 0
 public s
 public t
 public snext
 public tnext
 public sstep
 public tstep
 public sfracf
 public tfracf
 public spancountminus1
s dd 0
t dd 0
snext dd 0
tnext dd 0
sstep dd 0
tstep dd 0
sfracf dd 0
tfracf dd 0
spancountminus1 dd 0
 public izi
 public izistep
izi dd 0
izistep dd 0
 public advancetable
advancetable dd 0, 0, 2, 0
 public pbase
 public pz
pbase dd 0
pz dd 0
 public reciprocal_table
reciprocal_table dd 040000000h, 02aaaaaaah, 020000000h
 dd 019999999h, 015555555h, 012492492h
 dd 010000000h, 0e38e38eh, 0ccccccch, 0ba2e8bah
 dd 0aaaaaaah, 09d89d89h, 09249249h, 08888888h
 public pspantemp
 public counttemp
 public jumptemp
pspantemp dd 0
counttemp dd 0
jumptemp dd 0
 public mmbuf
 align 4
mmbuf dd 0
 dd 0
 public fp_8
 public fp_16
 public fp_64k
 public Float2ToThe31nd
 public FloatMinus2ToThe31nd
 public fp_64kx64k
fp_8 dd 8.0
fp_16 dd 16.0
fp_64k dd 047800000h
Float2ToThe31nd dd 04f000000h
FloatMinus2ToThe31nd dd 0cf000000h
fp_64kx64k dd 04f000000h
 public float_1
 public float_particle_z_clip
 public float_point5
float_1 dd 1.0
float_particle_z_clip dd 8.0
float_point5 dd 0.5
 public DP_u
 public DP_v
 public DP_32768
 public DP_Color
 public DP_Pix
DP_u dd 0
DP_v dd 0
DP_32768 dd 32768.0
DP_Color dd 0
DP_Pix dd 0
_DATA ENDS
 END
