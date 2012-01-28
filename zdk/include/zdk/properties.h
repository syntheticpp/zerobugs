#ifndef PROPERTIES_H__07367374_99D3_4DC1_8CD4_FAF91176DFEE
#define PROPERTIES_H__07367374_99D3_4DC1_8CD4_FAF91176DFEE
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

#include "zdk/platform.h"
#include "zdk/zobject.h"


using Platform::word_t;


DECLARE_ZDK_INTERFACE_(Properties, ZObject)
{
    DECLARE_UUID("6e947e43-9a88-4540-b5a4-f4eb82a9bb8c")

    /**
     * Query an integer property; if not found, set
     * and return the defaultVal.
     */
    virtual word_t get_word(const char* name, word_t defaultVal = 0) = 0;

    virtual void set_word(const char* name, word_t value) = 0;

    virtual double get_double(const char* name, double defaultVal = .0) = 0;

    virtual void set_double(const char* name, double value) = 0;

    virtual const char* get_string( const char* name,
                                    const char* defaultVal = NULL) = 0;

    virtual void set_string(const char* name, const char* value) = 0;

    /**
     * Retrieve a sub-object
     */
    virtual ZObject* get_object(const char* name) const = 0;

    virtual void set_object(const char* name, ZObject*) = 0;
};

#endif // PROPERTIES_H__07367374_99D3_4DC1_8CD4_FAF91176DFEE
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
