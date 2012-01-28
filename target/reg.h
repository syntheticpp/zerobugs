#ifndef REG_H__0665F4FD_2BB0_4EC8_87DF_114A66A67A0C
#define REG_H__0665F4FD_2BB0_4EC8_87DF_114A66A67A0C
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
//
#include <sstream>
#include "zdk/register.h"
#include "zdk/weak_ptr.h"
#include "zdk/zero.h"
#include "target/target.h"
#include "target/variant_lite.h"


enum RegType
{
    REG_USER, // general-purpose
    REG_FPUX, // floating-point / extended
};


template<typename T = reg_t, RegType R = REG_USER>
class Reg : public ZObjectImpl<Register>
{
    typedef T value_type;

    Reg(const Reg&);            // non-copyable
    Reg& operator=(const Reg&); // non-assignable

public:
    DECLARE_UUID("a9b6f4d8-d964-454a-99a8-73aa12fc2709")

BEGIN_INTERFACE_MAP(Reg)
    INTERFACE_ENTRY(Reg)
    INTERFACE_ENTRY(Register)
END_INTERFACE_MAP()

    virtual ~Reg() throw() { }

    Reg(const char* name, const Thread& t, size_t off, T val)
      : name_(name)
      , thread_(&t)
      , offset_(off)
      , value_(val)
    { }

// <Register implementation>
    const char* name() const { return name_.c_str(); }

    size_t size() const { return sizeof(value_type); }

    Variant* value() const
    {
        if (!var_)
        {
            var_ = new VariantLite<value_type>(value_);
            assert(var_->size() == this->size());
        }
        return var_.get();
    }

    bool set_value(const char* value, const char*)
    {
        bool result = false;
        if (value)
        {
            std::istringstream is(value);
            is.unsetf(std::ios_base::basefield);
            T newVal;
            is >> newVal;
            result = commit(newVal);
        }
        return result;
    }

    size_t
    enum_fields(EnumCallback3<const char*, reg_t, reg_t>*) const
    {
        return 0;
    }
// </Register implementation>

    Variant::TypeTag type_tag() const
    {
        return ::type_tag<value_type>::tag;
    }

    virtual RegType type() const { return R; }

    RefPtr<Thread> thread() const { return thread_.lock(); }

    /**
     * used when writing register values
     */
    size_t offset() const { return offset_; }

protected:
    bool commit(T newVal)
    {
        bool result = false;
        RefPtr<Variant> var(new VariantLite<T>(newVal));
        if (RefPtr<Thread> thread = thread_.ref_ptr())
        {
            Runnable& task = interface_cast<Runnable&>(*thread);
            result = task.write_register(this, var.get());
            if (result)
            {
                var_ = var;
                value_ = newVal;
            }
        }
        return result;
    }

protected:
    std::string     name_;
    WeakPtr<Thread> thread_; //owner
    size_t          offset_;
    value_type      value_;
    mutable RefPtr<Variant> var_;
};

#endif // REG_H__0665F4FD_2BB0_4EC8_87DF_114A66A67A0C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
