//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id$
//**
//**	Copyright (C) 1999-2001 JÆnis Legzdi·ý
//**
//**	This program is free software; you can redistribute it and/or
//**  modify it under the terms of the GNU General Public License
//**  as published by the Free Software Foundation; either version 2
//**  of the License, or (at your option) any later version.
//**
//**	This program is distributed in the hope that it will be useful,
//**  but WITHOUT ANY WARRANTY; without even the implied warranty of
//**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//**  GNU General Public License for more details.
//**
//**	$Log$
//**	Revision 1.2  2001/07/27 14:27:56  dj_jl
//**	Update with Id-s and Log-s, some fixes
//**
//**************************************************************************
#include "../../makeinfo.h"

int num_sfx = 135;

sfxinfo_t sfx[] =
{
	{ "", "", 0, 2, 1 },
	{ "Sound1", "dsswish", 64, 2, 1 },
	{ "Sound2", "dsmeatht", 64, 2, 1 },
	{ "Sound3", "dsmtalht", 64, 2, 1 },
	{ "Sound4", "dswpnup", 78, 2, 1 },
	{ "Sound5", "dsrifle", 64, 2, 1 },
	{ "Sound6", "dsmislht", 64, 2, 1 },
	{ "Sound7", "dsflburn", 64, 2, 1 },
	{ "Sound8", "dsflidl", 118, 2, 1 },
	{ "Sound9", "dsagrsee", 98, 2, 1 },
	{ "Sound10", "dsplpain", 96, 2, 1 },
	{ "Sound11", "dspcrush", 96, 2, 1 },
	{ "Sound12", "dspespna", 98, 2, 1 },
	{ "Sound13", "dspespnb", 98, 2, 1 },
	{ "Sound14", "dspespnc", 98, 2, 1 },
	{ "Sound15", "dspespnd", 98, 2, 1 },
	{ "Sound16", "dsagrdpn", 98, 2, 1 },
	{ "Sound17", "dspldeth", 32, 2, 1 },
	{ "Sound18", "dsplxdth", 32, 2, 1 },
	{ "Sound19", "dsslop", 78, 2, 1 },
	{ "Sound20", "dsrebdth", 98, 2, 1 },
	{ "Sound21", "dsagrdth", 98, 2, 1 },
	{ "Sound22", "dslgfire", 211, 2, 1 },
	{ "Sound23", "dssmfire", 211, 2, 1 },
	{ "Sound24", "dsalarm", 210, 2, 1 },
	{ "Sound25", "dsdrlmto", 98, 2, 1 },
	{ "Sound26", "dsdrlmtc", 98, 2, 1 },
	{ "Sound27", "dsdrsmto", 98, 2, 1 },
	{ "Sound28", "dsdrsmtc", 98, 2, 1 },
	{ "Sound29", "dsdrlwud", 98, 2, 1 },
	{ "Sound30", "dsdrswud", 98, 2, 1 },
	{ "Sound31", "dsdrston", 98, 2, 1 },
	{ "Sound32", "dsbdopn", 98, 2, 1 },
	{ "Sound33", "dsbdcls", 98, 2, 1 },
	{ "Switch", "dsswtchn", 78, 2, 1 },
	{ "SwitchBolt", "dsswbolt", 98, 2, 1 },
	{ "SwitchScanner", "dsswscan", 98, 2, 1 },
	{ "Sound37", "dsyeah", 10, 2, 1 },
	{ "Sound38", "dsmask", 210, 2, 1 },
	{ "Sound39", "dspstart", 100, 2, 1 },
	{ "Sound40", "dspstop", 100, 2, 1 },
	{ "Sound41", "dsitemup", 78, 2, 1 },
	{ "BreakGlass", "dsbglass", 200, 2, 1 },
	{ "Sound43", "dswriver", 201, 2, 1 },
	{ "Sound44", "dswfall", 201, 2, 1 },
	{ "Sound45", "dswdrip", 201, 2, 1 },
	{ "Sound46", "dswsplsh", 201, 2, 1 },
	{ "Sound47", "dsrebact", 98, 2, 1 },
	{ "Sound48", "dsagrac1", 98, 2, 1 },
	{ "Sound49", "dsagrac2", 98, 2, 1 },
	{ "Sound50", "dsagrac3", 98, 2, 1 },
	{ "Sound51", "dsagrac4", 98, 2, 1 },
	{ "Sound52", "dsambppl", 218, 2, 1 },
	{ "Sound53", "dsambbar", 218, 2, 1 },
	{ "Sound54", "dstelept", 32, 2, 1 },
	{ "Sound55", "dsratact", 99, 2, 1 },
	{ "Sound56", "dsitmbk", 100, 2, 1 },
	{ "Sound57", "dsxbow", 99, 2, 1 },
	{ "Sound58", "dsburnme", 99, 2, 1 },
	{ "Sound59", "dsoof", 96, 2, 1 },
	{ "Sound60", "dswbrldt", 98, 2, 1 },
	{ "Sound61", "dspsdtha", 109, 2, 1 },
	{ "Sound62", "dspsdthb", 109, 2, 1 },
	{ "Sound63", "dspsdthc", 109, 2, 1 },
	{ "Sound64", "dsrb2pn", 96, 2, 1 },
	{ "Sound65", "dsrb2dth", 32, 2, 1 },
	{ "Sound66", "dsrb2see", 98, 2, 1 },
	{ "Sound67", "dsrb2act", 98, 2, 1 },
	{ "Sound68", "dsfirxpl", 70, 2, 1 },
	{ "Sound69", "dsstnmov", 100, 2, 1 },
	{ "Sound70", "dsnoway", 78, 2, 1 },
	{ "Sound71", "dsrlaunc", 64, 2, 1 },
	{ "Sound72", "dsrflite", 65, 2, 1 },
	{ "Sound73", "dsradio", 60, 2, 1 },
	{ "SwitchPullChain", "dspulchn", 98, 2, 1 },
	{ "SwitchKnob", "dsswknob", 98, 2, 1 },
	{ "SwitchKeyCard", "dskeycrd", 98, 2, 1 },
	{ "SwitchStone", "dsswston", 98, 2, 1 },
	{ "Sound78", "dssntsee", 98, 2, 1 },
	{ "Sound79", "dssntdth", 98, 2, 1 },
	{ "Sound80", "dssntact", 98, 2, 1 },
	{ "Sound81", "dspgrdat", 64, 2, 1 },
	{ "Sound82", "dspgrsee", 90, 2, 1 },
	{ "Sound83", "dspgrdpn", 96, 2, 1 },
	{ "Sound84", "dspgrdth", 32, 2, 1 },
	{ "Sound85", "dspgract", 120, 2, 1 },
	{ "Sound86", "dsproton", 64, 2, 1 },
	{ "Sound87", "dsprotfl", 64, 2, 1 },
	{ "Sound88", "dsplasma", 64, 2, 1 },
	{ "Sound89", "dsreavat", 64, 2, 1 },
	{ "Sound90", "dsrevbld", 64, 2, 1 },
	{ "Sound91", "dsrevsee", 90, 2, 1 },
	{ "Sound92", "dsreavpn", 96, 2, 1 },
	{ "Sound93", "dsrevdth", 32, 2, 1 },
	{ "Sound94", "dsrevact", 120, 2, 1 },
	{ "Sound95", "dsspisit", 90, 2, 1 },
	{ "Sound96", "dsspdwlk", 65, 2, 1 },
	{ "Sound97", "dsspidth", 32, 2, 1 },
	{ "Sound98", "dsspdatk", 32, 2, 1 },
	{ "Sound99", "dschant", 218, 2, 1 },
	{ "Sound100", "dsstatic", 32, 2, 1 },
	{ "Sound101", "dschain", 70, 2, 1 },
	{ "Sound102", "dstend", 100, 2, 1 },
	{ "Sound103", "dsphoot", 32, 2, 1 },
	{ "Sound104", "dsexplod", 32, 2, 1 },
	{ "Sound105", "dsbarexp", 32, 2, 1 },
	{ "Sound106", "dssigil", 32, 2, 1 },
	{ "Sound107", "dssglhit", 32, 2, 1 },
	{ "Sound108", "dssiglup", 32, 2, 1 },
	{ "Sound109", "dsprgpn", 96, 2, 1 },
	{ "Sound110", "dsprogac", 120, 2, 1 },
	{ "Sound111", "dslorpn", 96, 2, 1 },
	{ "Sound112", "dslorsee", 90, 2, 1 },
	{ "SwitchFool", "dsdifool", 32, 2, 1 },
	{ "Sound114", "dsinqdth", 32, 2, 1 },
	{ "Sound115", "dsinqact", 98, 2, 1 },
	{ "Sound116", "dsinqsee", 90, 2, 1 },
	{ "Sound117", "dsinqjmp", 65, 2, 1 },
	{ "Sound118", "dsamaln1", 99, 2, 1 },
	{ "Sound119", "dsamaln2", 99, 2, 1 },
	{ "Sound120", "dsamaln3", 99, 2, 1 },
	{ "Sound121", "dsamaln4", 99, 2, 1 },
	{ "Sound122", "dsamaln5", 99, 2, 1 },
	{ "Sound123", "dsamaln6", 99, 2, 1 },
	{ "Sound124", "dsmnalse", 64, 2, 1 },
	{ "Sound125", "dsalnsee", 64, 2, 1 },
	{ "Sound126", "dsalnpn", 96, 2, 1 },
	{ "Sound127", "dsalnact", 120, 2, 1 },
	{ "Sound128", "dsalndth", 32, 2, 1 },
	{ "Sound129", "dsmnaldt", 32, 2, 1 },
	{ "Sound130", "dsreactr", 31, 2, 1 },
	{ "Sound131", "dsairlck", 98, 2, 1 },
	{ "Sound132", "dsdrchno", 98, 2, 1 },
	{ "Sound133", "dsdrchnc", 98, 2, 1 },
	{ "SwitchValve", "dsvalve", 98, 2, 1 },
};