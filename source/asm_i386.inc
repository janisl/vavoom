	.486P
	.MMX
	.model FLAT

;	Global rasterizer variables
externdef	_d_sdivzstepu:dword
externdef	_d_tdivzstepu:dword
externdef	_d_zistepu:dword
externdef	_d_sdivzstepv:dword
externdef	_d_tdivzstepv:dword
externdef	_d_zistepv:dword
externdef	_d_sdivzorigin:dword
externdef	_d_tdivzorigin:dword
externdef	_d_ziorigin:dword

externdef	_sadjust:dword
externdef	_tadjust:dword
externdef	_bbextents:dword
externdef	_bbextentt:dword

externdef	_cacheblock:dword
externdef	_cachewidth:dword
externdef	_ds_transluc:dword
externdef	_ds_transluc16:dword

externdef	_r_lightptr:dword
externdef	_r_lightptrr:dword
externdef	_r_lightptrg:dword
externdef	_r_lightptrb:dword
externdef	_r_lightwidth:dword
externdef	_r_numvblocks:dword
externdef	_r_sourcemax:dword
externdef	_r_stepback:dword
externdef	_prowdestbase:dword
externdef	_pbasesource:dword
externdef	_sourcetstep:dword
externdef	_surfrowbytes:dword
externdef	_lightright:dword
externdef	_lightrightstep:dword
externdef	_lightdeltastep:dword
externdef	_lightdelta:dword
externdef	_lightrleft:dword
externdef	_lightrright:dword
externdef	_lightrleftstep:dword
externdef	_lightrrightstep:dword
externdef	_lightgleft:dword
externdef	_lightgright:dword
externdef	_lightgleftstep:dword
externdef	_lightgrightstep:dword
externdef	_lightbleft:dword
externdef	_lightbright:dword
externdef	_lightbleftstep:dword
externdef	_lightbrightstep:dword

;	Asm only variables
externdef	sdivz8stepu:dword
externdef	tdivz8stepu:dword
externdef	zi8stepu:dword
externdef	sdivz16stepu:dword
externdef	tdivz16stepu:dword
externdef	zi16stepu:dword

externdef	s:dword
externdef	t:dword
externdef	snext:dword
externdef	tnext:dword
externdef	sstep:dword
externdef	tstep:dword
externdef	sfracf:dword
externdef	tfracf:dword
externdef	spancountminus1:dword

externdef	izi:dword
externdef	izistep:dword

externdef	advancetable:dword

externdef	pbase:dword
externdef	pz:dword

externdef	reciprocal_table:dword

externdef	pspantemp:dword
externdef	counttemp:dword
externdef	jumptemp:dword

externdef	mmbuf:dword

externdef	fp_64k:dword
externdef	fp_8:dword
externdef	fp_16:dword
externdef	Float2ToThe31nd:dword
externdef	FloatMinus2ToThe31nd:dword
externdef	fp_64kx64k:dword

externdef	float_1:dword
externdef	float_particle_z_clip:dword
externdef	float_point5:dword
externdef	DP_u:dword
externdef	DP_v:dword
externdef	DP_32768:dword
externdef	DP_Color:dword
externdef	DP_Pix:dword

;	External rasterizer variables
externdef	_ylookup:dword
externdef	_zbuffer:dword
externdef	_scrn:dword
externdef	_scrn16:dword
externdef	_pal8_to16:dword
externdef	_mmx_mask4:dword
externdef	_mmx_mask8:dword
externdef	_mmx_mask16:dword
externdef	_d_rowbytes:dword
externdef	_d_zrowbytes:dword
externdef	_vieworg:dword
externdef	_r_ppn:dword
externdef	_r_pup:dword
externdef	_r_pright:dword
externdef	_centerxfrac:dword
externdef	_centeryfrac:dword
externdef	_d_particle_right:dword
externdef	_d_particle_top:dword
externdef	_d_pix_min:dword
externdef	_d_pix_max:dword
externdef	_d_pix_shift:dword
externdef	_d_y_aspect_shift:dword
externdef	_d_rgbtable:dword
externdef	_rshift:dword
externdef	_gshift:dword
externdef	_bshift:dword
externdef	_fadetable:dword
externdef	_fadetable16:dword
externdef	_fadetable16r:dword
externdef	_fadetable16g:dword
externdef	_fadetable16b:dword
externdef	_fadetable32:dword

;	External variables for progs
externdef	_pr_globals:dword
externdef	_pr_stackPtr:dword
externdef	_pr_statements:dword
externdef	_pr_functions:dword
externdef	_pr_globaldefs:dword
externdef	_pr_builtins:dword
externdef	_current_func:dword

externdef	_D_DrawZSpan:near
externdef	_PR_RFInvalidOpcode:near

