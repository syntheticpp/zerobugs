#ifndef TYPE_SPEC_H__D9482620_EB68_4A13_B898_1D48781824FA
#define TYPE_SPEC_H__D9482620_EB68_4A13_B898_1D48781824FA
//
// $Id: type_spec.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <string>
#include <vector>
#include "expr.h"



/**
 * Represents type specifiers and qualifiers in the parse tree
 */
CLASS TypeSpec : public Expr
{
public:
    DECLARE_UUID("9f9c74c6-ceec-435e-ae39-e5d7c2512605")

BEGIN_INTERFACE_MAP(TypeSpec)
    INTERFACE_ENTRY(TypeSpec)
    INTERFACE_ENTRY_INHERIT(Expr)
END_INTERFACE_MAP()

    enum Mask
    {
        TM_INT      = 0x1000,
        TM_FLOAT    = 0x2000,
//      TM_VOID     = 0x4000,
//      TM_TYPENAME = 0x8000,
        TM_TYPE     = 0xF000,
        TM_NON_INT  = 0xE000,
        TM_SIZE     = 0x00FF, // 255 is enough for builtins
    };

    enum Flags
    {
        // 0x1000 == int mask
        TF_INT      = 0x00001000 | sizeof(int),
        TF_CHAR     = 0x00001000 | sizeof(char),
        TF_SHORT    = 0x00001000 | sizeof(short),
        TF_SIGNED   = 0x00000100,
        TF_UNSIGNED = 0x00000200,
        // note: long can apply to either an int or a double
        TF_LONG     = 0x00000400,
        TF_LONGLONG = 0x00000800,
        // 0x2000 == float mask
        TF_FLOAT    = 0x00002000 | sizeof(float),
        TF_DOUBLE   = 0x00002000 | sizeof(double),
        TF_VOID     = 0x00004000,
        TF_TYPENAME = 0x00008000,
        TF_CONST    = 0x00010000,
        TF_VOLATILE = 0x00020000,
    };

    virtual ~TypeSpec() throw();

    TypeSpec(Interp*, Flags, const char* name = NULL);

    Flags flags() const { return flags_; }

    const std::string& name() const { return name_; }

protected:
    RefPtr<Variant> eval_impl(Context&);

    RefPtr<Expr> clone(Interp* interp) const
    {
        return new TypeSpec(interp, flags_, name_.c_str());
    }

private:
    TypeSpec(const TypeSpec&);
    TypeSpec& operator=(const TypeSpec&);

    Flags flags_;
    std::string name_;
};


CLASS TypeName : public TypeSpec
{
BEGIN_INTERFACE_MAP(TypeName)
    INTERFACE_ENTRY(TypeName)
    INTERFACE_ENTRY_INHERIT(TypeSpec)
END_INTERFACE_MAP()

public:
    DECLARE_UUID("e845e72e-1f1d-4b17-a82e-58051887ae93")

    TypeName(Interp*, RefPtr<DataType>);
};


/**
 * A list of type specifiers and/or c-v qualifiers.
 */
CLASS TypeSpecList : public Expr
{
public:
    DECLARE_UUID("5a1f65b8-d6f6-4912-92c3-e32bea81c5e9")

BEGIN_INTERFACE_MAP(TypeSpecList)
    INTERFACE_ENTRY(TypeSpecList)
    INTERFACE_ENTRY_INHERIT(Expr)
END_INTERFACE_MAP()

    explicit TypeSpecList(Interp*);

    ~TypeSpecList() throw();

    void push_back(RefPtr<Expr>);

    const std::string& name() const { return name_; }

protected:
    RefPtr<Variant> eval_impl(Context&);

    void validate_and_set(TypeSpec::Flags);

    TypeSpecList(Interp* interp,
        unsigned long flags,
        const std::string& name
    ) : Expr(interp), flags_(flags), name_(name)
    { }

    RefPtr<Expr> clone(Interp* interp) const
    {
        return new TypeSpecList(interp, flags_, name_);
    }

private:
    unsigned long flags_;
    std::string name_;
};


ZDK_LOCAL RefPtr<Expr> verify_class_or_struct(const RefPtr<Expr>&);

ZDK_LOCAL RefPtr<Expr> verify_union(const RefPtr<Expr>&);

ZDK_LOCAL RefPtr<Expr> verify_enum(const RefPtr<Expr>&);

#endif // TYPE_SPEC_H__D9482620_EB68_4A13_B898_1D48781824FA
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
