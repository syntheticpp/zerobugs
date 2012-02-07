#ifndef DYNAMIC_LIB_H__92A66132_1FF8_44F3_8E1A_55BF6E14C33C
#define DYNAMIC_LIB_H__92A66132_1FF8_44F3_8E1A_55BF6E14C33C
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

#include "zdk/config.h"
#include "zdk/export.h"
#include "zdk/stdexcept.h"
#if __USE_GNU
 #include <dlfcn.h>
#endif

#include <memory>
#include <string>
#include <boost/type_traits/function_traits.hpp>
#include <boost/type_traits.hpp>
#include "generic/ref_counted_base_handle.h"
#include "generic/type_select.h"
#include "zdk/interface_cast.h"


// Forward declaration of a base interface for
// components that can be created on demand by
// factory functions within modules.
struct Plugin;

// Forward decl of smart pointer around
// entities that live inside modules.
template<typename T> class ImportPtr;


class bad_downcast : public std::bad_cast
{
    std::string what_;

public:
    bad_downcast(const std::type_info & fromType,
                 const std::type_info & toType);

    ~bad_downcast() throw ()
    {
    }

    virtual const char* what() const throw()
    {
        return what_.c_str();
    }
};


struct DynamicLibHandleTraits
{
    static void* null_value() { return 0; }
    static void dispose(void*&) throw();
};


typedef auto_handle<
    void*,
    DynamicLibHandleTraits,
    ref_counted_base_handle<void*, DynamicLibHandleTraits>
> DynamicLibHandle;


/**
 * A class for abstracting out the OS-specific
 * mechanisms for loading dynamic libraries
 */
class DynamicLib
{
    DynamicLib(const DynamicLib&);
    DynamicLib& operator=(const DynamicLib&);

public:
    /**
     * Ctor, takes the path to the module file
     */
    explicit DynamicLib(const char* path = 0);
    ~DynamicLib() throw();

    /**
     * Obtains the address where a module-lived
     * symbol is loaded, and wraps it into a
     * ImportPtr<T>
     * @return true if successful, or false if no
     * symbol with the given name is found.
     */
    template<typename T>
    bool import(const char* symbolName, ImportPtr<T>& t)
    {
        bool result = false;
        if (void* ptr = address_of(symbolName))
        {
            /* ISO C++ forbids casting from pointer-to-object
               to pointer-to-function */
            // t.reset(reinterpret_cast<T*>(ptr), *this);
            t.reset((T*)ptr, *this);

            result = true;
        }
        return result;
    }

    /**
     * Returns the canonicalized name of the
     * file corresponding to this module. The returned
     * path is absolute and contains no dots or
     * symbolic links.
     */
    const std::string& filename() const
    {
        return filename_;
    }

    /**
     * Return the reference count.
     */
    int count() const { return handle_.count(); }

    bool is_self() const { return isSelf_; }

    const DynamicLibHandle& handle() const { return handle_; }

    /**
     * A class for counting references to
     * entities that live inside modules
     */
    class Counter
    {
    public:
        explicit Counter(int = 0, DynamicLib* = 0);

        Counter(const Counter&);

        ~Counter() throw();

        Counter& operator++();
        Counter& operator--();

        operator int() const { return count_; }

    private:
        Counter& operator++(int);
        Counter& operator--(int);

        std::ostream& print(std::ostream&, const char*) const;

        int count_;
        DynamicLibHandle handle_;
    };

protected:
    /**
     * Loads the module. Does nothing if already loaded.
     * Throws std::runtime_error on failure.
     */
    void load();

    /**
     * Unloads the dynamic library (or shared object) from memory.
     */
    void unload() throw();

private:
    /**
     * Internal implementation for getting the
     * address for a given symbol.
     */
    void* address_of(const char* symbolName);

    DynamicLibHandle handle_;
    std::string filename_;

    // this flags indicates whether the filename()
    // is the same as the caller's image file
    bool isSelf_;
};


typedef ::std::shared_ptr<DynamicLib> DynamicLibPtr;


namespace Impl
{
/**
 * When ImportPtr wraps pointers to functions that return
 * pointers to plugins, we want to turn the function into
 * a functor.
 *
 * The wrapper functor is the FactoryFunction class.
 *
 * The result_type of the functor is ImportPtr<T>
 * where T* is the original result type of the function.
 * This way, we disallow callers to assign the result of
 * a plugin-creator function to a naked pointer.
 *
 * The wrapping occurs in the dereferencing operator of
 * ImportPtr.
 */
template<typename F, typename R>
class FactoryFunction
{
public:
    typedef ImportPtr<R> result_type;

    FactoryFunction(F& fp, DynamicLib& module)
        : fp_(&fp), module_(module)
    { }

    /**
     * Support for funcs with no parameters
     */
    result_type operator()() const
    {
    #if __USE_GNU
        _dl_mcount_wrapper_check(fp_);
    #endif
        return result_type((*fp_)(), module_);
    }

    /**
     * Support for functions with one argument
     */
    template<typename U>
    result_type operator()(U u) const
    {
    #if __USE_GNU
        _dl_mcount_wrapper_check((void*)fp_);
    #endif
        return result_type((*fp_)(u), module_);
    }

    // Support for functions with several args
    // can be added here as needed
private:
    F* fp_;
    DynamicLib& module_;
};



template<typename T>
class PluginWrap : public T
{
protected:
    virtual ~PluginWrap() throw();

private:
    void release();
};


////////////////////////////////////////////////////////////////
// <type-traits>
// This helper template determines when the
// result type of the dereferencing of a ImportPtr
// needs to be a FactoryFunction<T>
//
template<bool isFunction, typename T>
struct TypeTraitsHelper
{
    enum { isPlugin =
        ::boost::is_same<Plugin, T>::value ||
        ::boost::is_base_and_derived<Plugin, T>::value
    };

    typedef const T* const_ptr_type;

    // Use the "overlay" technique -- when T* is
    // pointer to plugin, we'll cast it to PluginImpl<T>*
    // and thus discourage direct calls into release()
    //
    typedef typename TypeSelect<
        isPlugin, PluginWrap<T>*, T*>::type ptr_type;

    typedef typename TypeSelect<
        isPlugin, PluginWrap<T>&, T&>::type ref_type;
};


/**
 * Partial specialization for functions
 */
template<typename F>
struct TypeTraitsHelper<true, F>
{
    typedef typename boost::function_traits<
        F>::result_type result_type;
    typedef typename boost::remove_pointer<
        result_type>::type result_base_type;

    // When returnsPointerToPlugin evaluates to
    // true, we need to wrap the pointer to
    // function into a FactoryFunction.
    enum { returnsPointerToPlugin =
        ::boost::is_pointer<result_type>::value &&
        (::boost::is_base_and_derived< Plugin, result_base_type>::value
            || ::boost::is_same< Plugin, result_base_type>::value)
    };

    typedef typename TypeSelect<
        returnsPointerToPlugin,
        FactoryFunction<F, result_base_type>,
        F&
    >::type ref_type;

    typedef F* ptr_type;

    // ptr to function cannot be cv-qualified
    typedef F* const_ptr_type;
};


template<typename T>
class TypeTraits
{
public:
    enum { isFunction = ::boost::is_function<T>::value };

    typedef TypeTraitsHelper<isFunction, T> helper;
    typedef typename helper::ref_type ref_type;
    typedef typename helper::ptr_type ptr_type;
    typedef typename helper::const_ptr_type const_ptr_type;
};

// When the reference type is not T&, we need
// to pass the pointer to module to the
// FactoryFunction<T> constructor
template<typename T, typename R>
struct Dereferencer
{
    static R get(T* p, DynamicLib* mod)
    {
        if (!mod)
            throw std::invalid_argument("null DynamicLib");
        return R(*p, *mod);
    }
};

// When R is T&, dereference normally and ignore DynamicLib*
template<typename T>
struct Dereferencer<T, T&>
{
    static T& get(T* p, DynamicLib*)
    { return *p; };
};
} // namespace Impl

// </type-traits>
////////////////////////////////////////////////////////////////


template<typename T>
struct ImportPtrTraits
{
    static T null_value() { return T(); }
    static void dispose(T) {} // do nothing
};

/**
 * A class for determining the base class for
 * ImportPtr<T> (hence the name "meta-base")
 */
template<typename T>
struct ImportPtrMetaBase
{
    typedef ImportPtrTraits<T*> traits;

    // combine a ref_counted-based auto_handle with
    // the traits for T*
    typedef auto_handle<
        T*,
        traits,
        ref_counted_base_handle<T*, traits, DynamicLib::Counter>
    > base_type;
};


/**
 * Reference-counted smart pointer to module objects.
 */
template<typename T>
class ImportPtr : public ImportPtrMetaBase<T>::base_type
{
public:
    // short-hand for base class type
    typedef typename ImportPtrMetaBase<T>::base_type base_type;

    // type returned by the dereference operator
    typedef typename Impl::TypeTraits<T>::ref_type ref_type;
    typedef typename Impl::TypeTraits<T>::ptr_type ptr_type;
    typedef typename Impl::TypeTraits<T>::const_ptr_type const_ptr_type;

    typedef T* (ImportPtr::*unspecified_bool_type)() const;

    ~ImportPtr() throw() { }

    ImportPtr(T* p, DynamicLib& module)
        : base_type(p, &module)
        , module_(&module)
    {}

    ImportPtr() : base_type(0, (DynamicLib*)0), module_(0)
    {}
    template<typename U>
    explicit ImportPtr(const U& u)
        : base_type(u.construct(), (DynamicLib*)0)
        , module_(0)
    {}
    ImportPtr(const ImportPtr& other)
        : base_type(const_cast<ImportPtr&>(other))
        , module_(other.module_)
    {}
    /**
     * template copy constructor;
     * supports implicit conversions (such as
     * from pointer-to-derived to ptr-to-base)
     */
    template<typename U>
    ImportPtr(const ImportPtr<U>& other)
        : base_type(const_cast<ImportPtr<U>&>(other))
        , module_(other.module_)
    {}
    ImportPtr& operator=(const ImportPtr& other)
    {
        ImportPtr tmp(other);
        this->swap(tmp);
        return *this;
    }
    void reset(T* p, DynamicLib& module)
    {
        ImportPtr tmp(p, module);
        tmp.swap(*this);
        assert(module_ == &module);
    }
    void reset()
    {
        ImportPtr tmp;
        tmp.swap(*this);
        assert(module_ == 0);
    }
    void swap(ImportPtr& other) throw()
    {
        base_type::swap(other);
        std::swap(module_, other.module_);
    }
    operator unspecified_bool_type() const
    {
        return base_type::get() == 0 ? 0 : &ImportPtr::get;
    }

    ptr_type operator->() const { return static_cast<ptr_type>(get()); }

    /**
     * For T=function that returns a plugin-based
     * object, ref_type is a functor that returns
     * a smart ptr to the plugin.
     */
    ref_type operator *()
    {
        assert(get());
        return Impl::Dereferencer<T, ref_type>::get(get(), module_);
    }
    template<typename U>
    bool down_cast(ImportPtr<U>& ptr) const
    {
        if (U* up = interface_cast<U*>(this->get()))
        {
            ptr = reinterpret_cast<const ImportPtr<U>&>(*this);
            ptr.set(up);
            return true;
        }
        return false;
    }

    bool is_null() const { return get() == 0; }

    T* get() const { return base_type::get(); }

    const DynamicLib* module() const { return module_; }

private:
    template<typename U> friend class ImportPtr;

    DynamicLib* module_;
};


template<typename T, typename U>
ImportPtr<T> inline import_dynamic_cast(const ImportPtr<U>& source)
{
    ImportPtr<T> target;

    if (!source.down_cast(target))
    {
        throw bad_downcast(typeid(U), typeid(T));
    }
    return target;
}


template<typename T, typename U>
bool inline
operator <(const ImportPtr<T>& lhs, const ImportPtr<U>& rhs)
{
    return lhs.get() < rhs.get();
}


template<typename T, typename U>
bool inline
operator ==(const ImportPtr<T>& lhs, const ImportPtr<U>& rhs)
{
    return lhs.get() == rhs.get();
}


template<typename T, typename U>
T inline interface_cast(const ImportPtr<U>& u)
{
    typedef typename boost::remove_pointer<T>::type V;
    T ptr = 0;
    if (u.get())
    {
        if (u->query_interface(V::_uuid(), (void**)&ptr))
        {
            assert(ptr);
        }
    }
    return ptr;
}

#endif // DYNAMIC_LIB_H__92A66132_1FF8_44F3_8E1A_55BF6E14C33C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
