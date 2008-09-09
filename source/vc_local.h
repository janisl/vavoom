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
//**	Copyright (C) 1999-2006 Jānis Legzdiņš
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
//**************************************************************************

#ifndef __vc_local_h__
#define __vc_local_h__

#ifdef IN_VCC
#include "../utils/vcc/vcc.h"
#else

#include "gamedefs.h"
#include "progdefs.h"

#include "vc_emit_context.h"
#include "vc_expr_base.h"
#include "vc_expr_literal.h"
#include "vc_expr_unary_binary.h"
#include "vc_expr_cast.h"
#include "vc_expr_type.h"
#include "vc_expr_field.h"
#include "vc_expr_array.h"
#include "vc_expr_invoke.h"
#include "vc_expr_assign.h"
#include "vc_expr_local.h"
#include "vc_expr_misc.h"
#include "vc_statement.h"
#include "vc_error.h"
#include "vc_lexer.h"

#endif

#endif
