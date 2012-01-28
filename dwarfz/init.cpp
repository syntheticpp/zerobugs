//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "init.h"
#include "array_type.h"
#include "base_type.h"
#include "class_type.h"
#include "const_type.h"
#include "delegate.h"
#include "enum_type.h"
#include "function.h"
#include "imported_decl.h"
#include "inlined_instance.h"
#include "namespace.h"
#include "parameter.h"
#include "pointer_type.h"
#include "subroutine_type.h"
#include "typedef.h"
#include "producer.h"
#include "ptr_to_member_type.h"
#include "variable.h"
#include "volatile_type.h"


/**
 * Register classes with the factory
 */
void Dwarf::init()
{
    Producer<ArrayType>::register_with_factory();
    Producer<AssocArrayType>::register_with_factory();
    Producer<BaseType>::register_with_factory();
    Producer<ConstType>::register_with_factory();
    Producer<Delegate>::register_with_factory();
    Producer<DynArrayType>::register_with_factory();
    Producer<EnumType>::register_with_factory();
    Producer<Function>::register_with_factory();
    Producer<InlinedInstance>::register_with_factory();
    Producer<ImportedDecl>::register_with_factory();
    Producer<KlassType, DW_TAG_class_type>::register_with_factory();
    Producer<KlassType>::register_with_factory();
    Producer<Namespace>::register_with_factory();
    Producer<Parameter>::register_with_factory();
    Producer<PointerType, DW_TAG_pointer_type>::register_with_factory();
    Producer<PointerType, DW_TAG_reference_type>::register_with_factory();
    Producer<PtrToMemberType>::register_with_factory();
    Producer<SubroutineType>::register_with_factory();

    Producer<Typedef>::register_with_factory();
    Producer<UnionType>::register_with_factory();
    Producer<Variable>::register_with_factory();
    Producer<VolatileType>::register_with_factory();
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
