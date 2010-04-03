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

#ifndef __VCC_H__
#define __VCC_H__

// HEADER FILES ------------------------------------------------------------

#define OPCODE_STATS

#include "../../libs/core/core.h"
#include "../../source/common.h"
#include "../../source/vector.h"
#include "../../source/vc_location.h"
#include "../../source/vc_type.h"
#include "../../source/vc_member.h"
#include "../../source/progdefs.h"
#include "../../source/vc_field.h"
#include "../../source/vc_property.h"
#include "../../source/vc_method.h"
#include "../../source/vc_constant.h"
#include "../../source/vc_struct.h"
#include "../../source/vc_state.h"
#include "../../source/vc_class.h"
#include "../../source/vc_package.h"
#include "../../source/vc_emit_context.h"
#include "../../source/vc_expr_base.h"
#include "../../source/vc_expr_literal.h"
#include "../../source/vc_expr_unary_binary.h"
#include "../../source/vc_expr_cast.h"
#include "../../source/vc_expr_type.h"
#include "../../source/vc_expr_field.h"
#include "../../source/vc_expr_array.h"
#include "../../source/vc_expr_invoke.h"
#include "../../source/vc_expr_assign.h"
#include "../../source/vc_expr_local.h"
#include "../../source/vc_expr_misc.h"
#include "../../source/vc_statement.h"
#include "../../source/vc_error.h"
#include "../../source/vc_lexer.h"
#include "../../source/vc_modifiers.h"
#include "../../source/vc_parser.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

int dprintf(const char *text, ...);

char *va(const char *text, ...) __attribute__ ((format(printf, 1, 2)));

VStream* OpenFile(const VStr& Name);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

extern bool						GBigEndian;

#endif
