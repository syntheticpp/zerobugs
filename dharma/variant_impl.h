#ifndef VARIANT_IMPL_H__B52CFC1E_3729_4DEB_9C58_15F792F04337
#define VARIANT_IMPL_H__B52CFC1E_3729_4DEB_9C58_15F792F04337

// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/buffer_impl.h"
#include "zdk/ref_ptr.h"
#include "zdk/variant.h"
#include "zdk/zobject_impl.h"


CLASS VariantImpl : public ZObjectImpl<Variant>
{
public:
    DECLARE_UUID("5a842fff-c069-4bf1-bf07-445ce097a146")
BEGIN_INTERFACE_MAP(VariantImpl)
    INTERFACE_ENTRY(VariantImpl)
    INTERFACE_ENTRY(Variant)
    INTERFACE_ENTRY_AGGREGATE(buf_)
END_INTERFACE_MAP()

    VariantImpl();

    explicit VariantImpl(DebugSymbol&);

    virtual ~VariantImpl() throw();

    virtual void set_type_tag(TypeTag);

    virtual TypeTag type_tag() const;

    /**
     * @note the current implementation returns
     * zero sizes for VT_OBJECT type tags.
     */
    virtual size_t size() const;

    virtual uint64_t uint64() const;

    virtual int64_t int64() const;

    virtual long double long_double() const;

    virtual Platform::addr_t pointer() const;

    virtual uint64_t bits() const;

    virtual const void* data() const;

    virtual void copy(const Variant*, bool lvalue);

    virtual DebugSymbol* debug_symbol() const;

    void set_debug_symbol(DebugSymbol*);

    void resize(size_t size) { buf_->resize(size); }

    virtual Encoding encoding() const { return enc_; }

    void set_encoding(Encoding enc) { enc_ = enc; }

private:
    TypeTag             tag_;
    Encoding            enc_;
    RefPtr<BufferImpl>  buf_;
    RefPtr<DebugSymbol> sym_;
};

#endif // VARIANT_IMPL_H__B52CFC1E_3729_4DEB_9C58_15F792F04337
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
