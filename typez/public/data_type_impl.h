#ifndef DATA_TYPE_IMPL_H__72652349_DF83_41B5_92F9_76BAA39A9737
#define DATA_TYPE_IMPL_H__72652349_DF83_41B5_92F9_76BAA39A9737
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

#include <assert.h>
#include <boost/format.hpp>
#include <boost/utility.hpp>
#include "dharma/system_error.h"
#include "generic/auto_file.h"
#include "zdk/check_ptr.h"
#include "zdk/data_type.h"
#include "zdk/buffer_impl.h"
#include "zdk/observer_impl.h"
#include "zdk/shared_string_impl.h"
#include "zdk/types.h"
#include "zdk/weak_ref_impl.h"
#include "zdk/zero.h"
#include "type_names.h"

#undef make_pointer_name


using Platform::word_t;



/**
 * Default base for DataTypeImpl<>
 */
CLASS NamedTypeImpl
{
protected:
    virtual void set_name(SharedString& name) { name_ = &name; }

    void set_name(const std::string& name)
    { this->set_name(*shared_string(name)); }

    explicit NamedTypeImpl(SharedString* name)
    {
        if (name)
        {
            set_name(*name);
        }
        else
        {
            set_name(*unnamed_type());
        }
    }
    explicit NamedTypeImpl(const RefPtr<SharedString>& name)
    {
        if (name)
        {
            set_name(*name);
        }
        else
        {
            set_name(*unnamed_type());
        }
    }

    virtual ~NamedTypeImpl() throw() { }

    SharedString* get_name() const
    {
        assert(this->name_.get());
        return this->name_.get();
    }

    /**
     * implicit interface, expected by DataTypeImpl
     */
    void attach_impl(Observer*) { }

private:
    RefPtr<SharedString> name_;
};


CLASS ObservableNamedType : public NamedTypeImpl, boost::noncopyable
{
public:
    void set_name(SharedString& name)
    {
        NamedTypeImpl::set_name(name);
        if (subject_)
        {
            subject_->notify_state_change();
        }
    }

    void set_name(const std::string& name)
    { this->set_name(*shared_string(name)); }

protected:
    ObservableNamedType(SharedString* name = 0)
        : NamedTypeImpl(name)
    { }

    virtual ~ObservableNamedType() throw () { }

    /**
     * implicit interface, expected by DataTypeImpl
     */
    void attach_impl(Observer* observer)
    {
        if (observer)
        {
            if (!subject_)
            {
                RefPtr<Subject> subj(new SubjectImpl<>);
                subject_ = subj;
            }
            CHKPTR(subject_)->attach(observer);
        }
    }

private:
    RefPtr<Subject> subject_;
};


/**
 * Template implementation for the DataType interface,
 * or for an interface derived from DataType.
 * @note: not copyable, because RefCountedImpl has private
 * copy ctor and private assignment operator.
 */
template<typename T = DataType, typename U = NamedTypeImpl>
CLASS DataTypeImpl : public RefCountedImpl<T>, public U
{
    static uuidref_t _uuid();

public:
    BEGIN_INTERFACE_MAP(DataTypeImpl)
        INTERFACE_ENTRY_INHERIT(T)
        INTERFACE_ENTRY(T)
    END_INTERFACE_MAP()

protected:
    virtual ~DataTypeImpl() throw()
    { }

    /**
     * @param name the name of this type (eg: `int')
     * @param bitSize the size of an object of this type,
     * in bits (so that it can be used with struct bitfields).
     */
    explicit DataTypeImpl(SharedString* name,
                          bitsize_t bitSize = 0)
        : U(name), bitSize_(bitSize)
    { }
    explicit DataTypeImpl(const RefPtr<SharedString>& name,
                          bitsize_t bitSize = 0)
        : U(name), bitSize_(bitSize)
    { }

    template<typename V>
    explicit DataTypeImpl(const V& type)
        : U(type.name()), bitSize_(type.bit_size())
    { }

public:
    ////////////////////////////////////////////////////////////
    virtual SharedString* name() const
    {
        return U::get_name();
    }

    ////////////////////////////////////////////////////////////
    SharedString* make_pointer_name(const char* p, RefTracker* tracker) const
    {
        SharedString* result =
            p ? this->name()->append(p) : this->name();

        if (tracker)
        {
            tracker->register_object(result);
        }
        return result;
    }

    ////////////////////////////////////////////////////////////
    // generic comparison
    int compare(const char* lhs, const char* rhs) const
    {
        assert(lhs);
        assert(rhs);

        return strcmp(lhs, rhs);
    }

    ////////////////////////////////////////////////////////////
    virtual size_t bit_size() const { return bitSize_; }

    ////////////////////////////////////////////////////////////
    virtual size_t size() const
    {
        using Platform::byte_size;
        return (bit_size() + byte_size - 1) / byte_size;
    }

    ////////////////////////////////////////////////////////////
    /// generic write
    /// @note: does not work for return values,
    /// derived classes should handle the case
    void write(DebugSymbol* sym, const Buffer* buf) const
    {
        // pre-conditions
        assert(sym);
        assert(buf);
        assert(!sym->is_return_value());
        assert(buf->size() == this->size());

        RefPtr<Thread> thread = sym->thread();
        assert(thread);

        size_t words = buf->size() / sizeof(word_t);

        const word_t* data =
            reinterpret_cast<const word_t*>(buf->data());
        RefPtr<Buffer> copybuf;

        // write_data expects a bufferful of machine words
        // (i.e. longs on the IA32), because this is how ptrace works;
        // if the buffer is not machine word-aligned, we need to do
        // a read first, so that we don't trash any adjacent memory
        if (buf->size() % sizeof(word_t))
        {
            size_t bytes = sizeof(word_t);
            words = ((buf->size() + bytes - 1) / bytes);

            std::vector<word_t> tmp(words);

            thread->read_data(sym->addr(), &tmp[0], words);
            memcpy(&tmp[0], buf->data(), buf->size());

            // n was the size in longs, convert it to bytes
            bytes *= words;

            copybuf = new BufferImpl(bytes);
            copybuf->put(&tmp[0], bytes, 0);

            data = reinterpret_cast<const word_t*>(copybuf->data());
        }
        thread->write_data(sym->addr(), data, words);
    }

protected:
    ////////////////////////////////////////////////////////////
    void attach_to_observer(Observer* observer)
    {
        U::attach_impl(observer);
    }

    ////////////////////////////////////////////////////////////
    void describe(int fd) const
    {
        std::string s = this->description();
        if (::write(fd, s.c_str(), s.length()))
            ;
    }

    virtual std::string description() const;

private:
    DataTypeImpl(const DataTypeImpl&);
    DataTypeImpl& operator=(const DataTypeImpl&);

    const size_t bitSize_;
};


template<typename T, typename B>
inline std::string ZDK_LOCAL DataTypeImpl<T, B>::description() const
{
    return (boost::format("DataType: %1% (%2%)\nsize=%3% byte(s), %4% bits\n")
        % this->name()->c_str()
        % this->_name()
        % this->size()
        % this->bit_size()).str();
}

#endif // DATA_TYPE_IMPL_H__72652349_DF83_41B5_92F9_76BAA39A9737
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
