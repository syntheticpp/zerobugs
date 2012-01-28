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
#include "zdk/config.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <wchar.h>
#include <bitset>
#include <iostream>
#include <memory>           // auto_ptr
#include <stdexcept>
#include <sstream>
#include <sys/ptrace.h>
#include "dharma/environ.h"
#include "dharma/symbol_util.h"
#include "dharma/syscall_wrap.h"
#include "dharma/variant_impl.h"
#include "zdk/check_ptr.h"
#include "zdk/debug_sym.h"
#include "zdk/shared_string_impl.h"
#include "zdk/template_param.h"
#include "zdk/thread_util.h"
#include "zdk/type_system.h"
#include "zdk/type_system_util.h"
#include "zdk/variant_util.h"
#include "generic/lock.h"
#include "generic/temporary.h"
#include "generic/type_select.h"
#include "interp/variant_impl.h"
#include "public/adjust_base.h"
#include "public/debug_symbol.h"
#include "public/debug_symbol_array.h"
#include "public/enum_type.h"
#include "public/is_cv_qualified.h"
#include "public/remove_qual.h"
#include "public/types.h"
#include "public/util.h"
#include "public/value.h"
#include "private/debug_rtti.h"
#include "private/int_types.h"
#include "unmangle/unmangle.h"

using namespace std;
using namespace Platform;


// hack for temporarily turning off rtti-checking
#define disable_auto_rtti_in_scope(type, thread)                    \
auto_ptr<Temporary<bool> > tmp;                                     \
while (ClassTypeImpl* klass = interface_cast<ClassTypeImpl*>(type)) \
{                                                                   \
    if (RTTI* rtti = klass->rtti(thread))                           \
    {                                                               \
        tmp.reset(new Temporary<bool>(rtti->processing_, true));    \
    }                                                               \
    break;                                                          \
}

//maximum number of bits for bitfield data
static const size_t MAX_BITS = sizeof(int) * byte_size;

// separator used when printing type info (see ClassTypeImpl::description)
static const char _line[] =
"----------------------------------------------------------------------------";

static bool is_bit_field(bitsize_t bitSize, bitsize_t containerSize = 0)
{
    if (!bitSize)
    {
        return false;
    }
    if (containerSize > bitSize)
    {
        return true;
    }
    if (bitSize % byte_size)
    {
        return true;
    }
    const size_t nbytes = bitSize / byte_size;
    return ((nbytes > 1) && (nbytes % 2));
}

////////////////////////////////////////////////////////////////
static bool debug_object_layout()
{
    static bool flag = env::get_bool("ZERO_DEBUG_OBJECT_LAYOUT");
    return flag;
}


////////////////////////////////////////////////////////////////
bool debug_rtti()
{
    static bool flag = env::get_bool("ZERO_DEBUG_RTTI");
    return flag;
}


////////////////////////////////////////////////////////////////
static inline bool
use_auto_rtti(const RefPtr<TypeSystem>& typeSystem)
{
    return typeSystem ? typeSystem->use_auto_rtti() : true;
}


////////////////////////////////////////////////////////////////
static inline bool has_loop(const DebugSymbol& sym, addr_t addr)
{
    if (sym.addr() == addr)
    {
        return true;
    }
    for (DebugSymbol* p = sym.parent(); p; p = p->parent())
    {
        if (p->addr() == addr)
        {
            return true;
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////
IndirectType::IndirectType
(
    SharedString* name,
    bitsize_t size
)
  : BaseType(name, size)
{
}


////////////////////////////////////////////////////////////////
IndirectType::~IndirectType() throw()
{
}


////////////////////////////////////////////////////////////////
bool IndirectType::is_equal(const DataType* type) const
{
    assert(type);
    if (!type)
    {
        return false;
    }
    if (this == type)
    {
        return true;
    }
    if (is_cv_qualified(type))
    {
        return false;
    }
    if (const IndirectType* other = interface_cast<const IndirectType*>(type))
    {
        if (!link())
        {
            return !other->link();
        }
        return link() == other->link()
            || link()->is_equal(type)
            || link()->is_equal(other->link());
    }
    return link() ? link()->is_equal(type) : !type;
}


////////////////////////////////////////////////////////////////
VoidTypeImpl::VoidTypeImpl() : DataTypeImpl<VoidType>(void_type())
{
}


////////////////////////////////////////////////////////////////
SharedString*
VoidTypeImpl::read(DebugSymbol*, DebugSymbolEvents*) const
{
    return void_type();
}


////////////////////////////////////////////////////////////////
bool VoidTypeImpl::is_equal(const DataType* type) const
{
    return (this == type) || interface_cast<const VoidType*>(type);
}


////////////////////////////////////////////////////////////////
IntTypeImpl::IntTypeImpl
(
    SharedString*   name,
    bitsize_t       bitSize,
    bool            isSigned
)
  : BaseType(name, bitSize)
  , isSigned_(isSigned)
  , isChar_(bitSize == 8 || bitSize == 7) // todo: have the client
                                          // specify isChar?
  , bitOffs_(0)
{
    assert(bitSize);
}


////////////////////////////////////////////////////////////////
IntTypeImpl::IntTypeImpl
(
    bitsize_t       bitSize,
    bitsize_t       bitOffs,
    bool            isSigned
)
  : BaseType(NULL, bitSize)
  , isSigned_(isSigned)
  , isChar_(false)
  , bitOffs_(bitOffs)
{
    ostringstream name;
    name << bitSize << "-bit field [" << bitOffs << ']';
    set_name(name.str());
}


////////////////////////////////////////////////////////////////
bool IntTypeImpl::is_equal(const DataType* type) const
{
    if (this == type)
    {
        return true;
    }
    if (const IntType* it = interface_cast<const IntType*>(type))
    {
      /*
        clog << __func__ << ": bit_size()=" << bit_size()
                         << " is_signed()=" << is_signed()
                         << "/ bit_size()=" << it->bit_size()
                         << " is_signed()=" << it->is_signed()
                         << endl;
                         */
        return bit_size() == it->bit_size()
            && is_signed() == it->is_signed()
            && !is_cv_qualified(type);
    }
    return false;
}


////////////////////////////////////////////////////////////////
int IntTypeImpl::compare(const char* lhs, const char* rhs) const
{
    assert(lhs);
    assert(rhs);

    if (isSigned_)
    {
        return IntHelper<64, true>::compare(lhs, rhs, bit_size());
    }
    else
    {
        return IntHelper<64, false>::compare(lhs, rhs, bit_size());
    }
}


////////////////////////////////////////////////////////////////
size_t IntTypeImpl::parse(const char* val, Unknown2* unk) const
{
    assert(val);
    const char* p = 0;

    if (isSigned_)
    {
        IntHelper<64, true>::from_string(val, &p, bit_size(), unk);
    }
    else
    {
        IntHelper<64, false>::from_string(val, &p, bit_size(), unk);
    }
    return distance(val, p);
}


////////////////////////////////////////////////////////////////
//
// TODO: come up with a simpler / cleaner implementation
//
SharedString* IntTypeImpl::read(
    DebugSymbol* sym,
    DebugSymbolEvents* events
    ) const
{
    ostringstream outs;
    const int base = events ? events->numeric_base(sym) : 0;

    switch (base)
    {
    case 8:  outs << oct; break;
    case 0: // fallthru
    case 10: outs << dec; break;
    case 16: outs << hex; break;
    }
    outs.setf(ios::showbase);

    const int nbits = bit_size();
    assert(nbits <= 64);

    Thread* thread = CHKPTR(sym)->thread();
    assert(thread);

    const size_t wordSize = thread->is_32_bit() ? 4 : sizeof(word_t);
    const size_t bitOffs = bitOffs_ % (wordSize * byte_size);

    if (sym->is_constant() && sym->value())
    {
        assert(bitOffs == 0);

        RefPtr<Variant> v = new VariantImpl;
        parse(sym->value()->c_str(), v.get());

        variant_print(outs, *v);
        return shared_string(outs.str()).detach();
    }

    if (CHKPTR(sym)->is_return_value())
    {
        assert(bitOffs == 0);

        if (nbits > 32)
        {
            assert(nbits <= 64);

            union
            {
                int64_t val;
                uint64_t uval;
            } v;

            v.val = CHKPTR(sym->thread())->result64();

            if (isSigned_)
            {
                IntHelper<64, true>::print(outs, v.val, nbits, bitOffs, isChar_);
            }
            else
            {
                IntHelper<64, false>::print(outs, v.uval, nbits, bitOffs, isChar_);
            }
        }
        else
        {
            union
            {
                int32_t val;
                uint32_t uval;
            } v;

            v.val = CHKPTR(sym->thread())->result();

            if (isSigned_)
            {
                IntHelper<32, true>::print(outs, v.val, nbits, bitOffs, isChar_);
            }
            else
            {
                IntHelper<32, false>::print(outs, v.uval, nbits, bitOffs, isChar_);
            }
        }
    }
    /*
    else if (thread->is_32_bit())
    {
        if (isSigned_)
        {
            IntHelper<32, true>::read(outs, *sym, nbits, bitOffs, isChar_);
        }
        else
        {
            IntHelper<32, false>::read(outs, *sym, nbits, bitOffs, isChar_);
        }
    } */
    else if (isSigned_)
    {
        IntHelper<64, true>::read(outs, *sym, nbits, bitOffs, isChar_);
    }
    else
    {
        IntHelper<64, false>::read(outs, *sym, nbits, bitOffs, isChar_);
    }
    return shared_string(outs.str()).detach();
}


////////////////////////////////////////////////////////////////
//
// Handle two special cases: bit-fields, and return values;
// the "normal" case is handled by the base class implementation.
//
// ASSUMPTION: bit-fields are smaller than the machine-word
//
void IntTypeImpl::write(DebugSymbol* sym, const Buffer* buf) const
{
    assert(buf);

    RefPtr<Thread> thread = CHKPTR(sym)->thread();
    CHKPTR(thread);

    if (bit_size() > sizeof(word_t) * byte_size)
    {
        ostringstream err;
        err << "cannot modify large bitfield (" << bit_size() << ")";

        throw runtime_error(err.str());
    }

    if (is_bit_field(bit_size()))
    {
        // compilers should not allow a bit-field ret val
        assert(!sym->is_return_value());

        const size_t wordSize = thread->is_32_bit() ? 4 : sizeof(word_t);

        size_t bitOffs = bitOffs_ % (wordSize * byte_size);
#if (__BYTE_ORDER == __BIG_ENDIAN)
        bitOffs = wordSize * byte_size - bitOffs - bit_size();
#endif
        assert(bit_size() < wordSize * byte_size);

        word_t tmp = 0;
        thread->read_data(sym->addr(), &tmp, 1);

        const word_t mask = ((1 << bit_size()) - 1) << bitOffs;

        assert(buf->size() <= wordSize);
        word_t x = 0;

        switch (buf->size())
        {
        case 1: x = *(uint8_t*)buf->data();  break;
        case 2: x = *(uint16_t*)buf->data(); break;
        case 4: x = *(int32_t*)buf->data(); break;
        case 8: x = *(int64_t*)buf->data(); break;
        default:
            {
                ostringstream err;
                err << "unhandled buffer size: " << buf->size();

                throw logic_error(err.str());
            }
            break;
        }
        x <<= bitOffs;

        tmp &= ~mask;
        tmp |= x & mask;

        thread->write_data(sym->addr(), &tmp, 1);
    }
    else if (sym->is_return_value())
    {
        Runnable& task = interface_cast<Runnable&>(*thread);
        assert(size() <= 64);
        if (size() <= 32)
        {
            task.set_result(*(word_t*)buf->data());
        }
        else
        {
            task.set_result64(*(int64_t*)buf->data());
        }
    }
    else
    {
        assert(CHKPTR(buf)->size() == size());
        BaseType::write(sym, buf);
    }
}


////////////////////////////////////////////////////////////////
string IntTypeImpl::description() const
{
    string result = BaseType::description();
    if (bitOffs_)
    {
        const size_t bitOffs(bitOffs_);
        result += (
            boost::format("\nbit offset=%1%\n") % bitOffs).str();
    }
    result += "\nsigned=";
    result += (isSigned_ ? "TRUE" : "FALSE");
    result += "\n";
    return result;
}


////////////////////////////////////////////////////////////////
FloatTypeImpl::FloatTypeImpl(SharedString* name, size_t bitSize)
    : BaseType(name, bitSize)
{
    assert(bitSize);
}


////////////////////////////////////////////////////////////////
bool FloatTypeImpl::is_equal(const DataType* type) const
{
    if (const FloatType* ft = interface_cast<const FloatType*>(type))
    {
        return bit_size() == ft->bit_size() && !is_cv_qualified(type);
    }
    return false;
}


////////////////////////////////////////////////////////////////
SharedString* FloatTypeImpl::read(DebugSymbol* sym, DebugSymbolEvents*) const
{
    ostringstream outs;

    if (CHKPTR(sym)->is_constant() && sym->value())
    {
        RefPtr<Variant> v = new VariantImpl;
        parse(sym->value()->c_str(), v.get());

        variant_print(outs, *v);
        return shared_string(outs.str()).detach();
    }
    if (CHKPTR(sym)->is_return_value())
    {
        const size_t size = bit_size() / byte_size;
        Thread* thread = CHKPTR(sym->thread());

        long double dval = thread->result_double(size);

        outs << dval;
    }
    else
    {
        switch (bit_size())
        {
        case sizeof(float) * byte_size:
            outs << Value<float>().read(*sym);
            break;
        case sizeof(double) * byte_size:
            outs << Value<double>().read(*sym);
            break;

        case sizeof(long double) * byte_size:
            outs << Value<long double>().read(*sym);
            break;

        default:
            if (CHKPTR(sym->thread())->is_32_bit())
            {
                outs << Value<long double>().read(*sym);
            }
            else
            {
                outs << bit_size() << "-bit double: unhandled";
            }
            break;
        }
    }
    return shared_string(outs.str()).detach();
}


////////////////////////////////////////////////////////////////
// modify the value of a floating point variable in the
// memory space of the debugged program
void FloatTypeImpl::write(DebugSymbol* sym, const Buffer* buf) const
{
    assert(buf);

    if (!CHKPTR(sym)->is_return_value())
    {
        BaseType::write(sym, buf);
    }
    else
    {
        long double dval = 0;

        switch (buf->size())
        {
        case sizeof(float):
            dval = *(float*)buf->data();
            break;

        case sizeof(double):
            dval = *(double*)buf->data();
            break;

        case sizeof (long double):
            dval = *(long double*)buf->data();
            break;

        default:
            assert(false);
        }
        Runnable& task = interface_cast<Runnable&>(*CHKPTR(sym->thread()));
        task.set_result_double(dval, buf->size());
    }
}


////////////////////////////////////////////////////////////////
int FloatTypeImpl::compare(const char* lhs, const char* rhs) const
{
    assert(lhs);
    assert(rhs);

    if (strcmp(lhs, "nan")  == 0    ||
        strcmp(lhs, "-nan") == 0    ||
        strcmp(rhs, "nan")  == 0    ||
        strcmp(rhs, "-nan") == 0)
    {
        return strcmp(lhs, rhs);
    }
    const long double lval = strtold(lhs, 0);
    const long double rval = strtold(rhs, 0);

    if (fabsl(lval - rval) <= numeric_limits<double>::epsilon())
    {
        return 0;
    }
    else if (lval < rval)
    {
        return -1;
    }
    return 1;
}


////////////////////////////////////////////////////////////////
size_t FloatTypeImpl::parse(const char* value, Unknown2* unk) const
{
    char* ptr = 0;

    if (bit_size() > 64)
    {
        long double x = strtold(value, &ptr);
        put(unk, x);
    }
    else if (bit_size() > 32)
    {
        double x = strtod(value, &ptr);
        put(unk, x);
    }
    else
    {
        float x = strtod(value, &ptr);
        put(unk, x);
    }
    return distance(value, (const char*)ptr);
}


////////////////////////////////////////////////////////////////
PointerTypeImpl::PointerTypeImpl(
    TypeSystem& typeSystem,
    Kind pointerOrReference,
    DataType& type)
  : BaseType(0, type, typeSystem.word_size())
  , pointerOrReference_(pointerOrReference)
{
    assert(this != &type);
    update_name();
}


////////////////////////////////////////////////////////////////
PointerTypeImpl::~PointerTypeImpl() throw()
{
}


////////////////////////////////////////////////////////////////
bool PointerTypeImpl::is_equal(const DataType* type) const
{
    if (this == type)
    {
        return true;
    }
    bool result = false;
    const PointerType* pt = interface_cast<const PointerType*>(type);

    if (pt)
    {
        if (is_reference() == pt->is_reference() && !is_cv_qualified(type))
        {
            if (!pointed_type())
            {
                return !pt->pointed_type();
            }
            result = pointed_type()->is_equal(pt->pointed_type());

            assert(result == BaseType::is_equal(type));
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
SharedString* PointerTypeImpl::make_pointer_name (
    const char* ptr,
    RefTracker* tracker
    ) const
{
    string pref;
    pref += static_cast<char>(pointerOrReference_);
    if (ptr)
    {
        pref += ptr;
    }

    SharedString* result = type().make_pointer_name(pref.c_str());

    if (tracker)
    {
        tracker->register_object(result);
    }
    return result;
}


////////////////////////////////////////////////////////////////
size_t PointerTypeImpl::parse(const char* str, Unknown2* unk) const
{
    char* ptr = 0;
    addr_t addr = 0;

    if (strcmp(str, "NULL") == 0)
    {
        ptr = const_cast<char*>(str) + 4;
    }
    else if (is_cstring())
    {
        size_t len = strlen(str) + 1; // include the NUL

        if (Buffer* buf = interface_cast<Buffer*>(unk))
        {
            buf->resize(len);
            buf->put(str, len);
        }
        if (VariantImpl* var = interface_cast<VariantImpl*>(unk))
        {
            var->set_type_tag(Variant::VT_POINTER);
            var->set_encoding(Variant::VE_STRING);
        }
        return len;
    }
    else
    {
        addr = strtoul(str, &ptr, 0);
        assert (addr != ULONG_MAX || errno == 0);
    }
    if (bit_size() == 32)
    {
        put(unk, uint32_t(addr), Variant::VT_POINTER);
    }
    else
    {
        put(unk, addr, Variant::VT_POINTER);
    }
    if (ptr)
    {
        assert(ptr >= str);
        return ptr - str;
    }
    return 0;
}


////////////////////////////////////////////////////////////////
void PointerTypeImpl::update_name()
{
    char buf[2] = { (char)pointerOrReference_, 0 };
    set_name(*type().make_pointer_name(buf));
}


////////////////////////////////////////////////////////////////
SharedString* null_ptr()
{
    static const RefPtr<SharedString> nullPtr(shared_string("NULL"));
    return nullPtr.get();
}


////////////////////////////////////////////////////////////////
static bool is_char_type(const DataType& type)
{
    bool result = false;

    if (type.bit_size() == 8 || type.bit_size() == 7)
    {
        const DataType* tp = &type;

        while (const QualifiedType* qt = interface_cast<const QualifiedType*>(tp))
        {
            tp = qt->remove_qualifier();
        }
        if (tp)
        {
            const SharedString* typeName = CHKPTR(tp->name());
            result = typeName->is_equal("char")
                  || typeName->is_equal("signed char")
                  // gcc-4.0.0:
                  || typeName->is_equal("int7_t")
                  || typeName->is_equal("uint7_t");
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
static bool is_wide_char_type(const DataType& type)
{
    bool result = false;
    if (type.bit_size() == 32) // linux: sizeof (wchar_t) == 4
    {
        const DataType* tp = &type;

        while (const QualifiedType* qt = interface_cast<const QualifiedType*>(tp))
        {
            tp = qt->remove_qualifier();
        }
        if (tp)
        {
            const SharedString* typeName = CHKPTR(tp->name());
            result = typeName->is_equal("wchar_t");
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
static bool
read_as_string(
    const DataType&     type,
    DebugSymbolEvents*  events,
    DebugSymbol*        sym,
    addr_t              addr,
    ostream&            outs,
    size_t              maxLen = 4096)
{
    const bool isUnicode = is_wide_char_type(type);

    if (!isUnicode && !is_char_type(type))
    {
        return false;
    }
    if (events)
    {
        switch (events->numeric_base(sym))
        {
        case 0: break;
        case 10: break;
        case 8: outs << oct << showbase; return false;
        case 16: outs << hex << showbase; return false;
        default: assert(false);
        }
    }
    outs << '"';
    if (isUnicode)
    {
        wchar_t ustr[1024];
        read_string(sym->thread(), addr, ustr);

        mbstate_t msb;
        memset(&msb, 0, sizeof msb);
        vector<char> buf(2048);
        const wchar_t* source = &ustr[0];

        wcsrtombs(&buf[0], &source, buf.size(), &msb);
        outs << &buf[0];
    }
    else
    {
        char str[4096] = { 0 };
        read_string(sym->thread(), addr, str, maxLen);
        //replace unprintable chars
        for (size_t i = 0; i != maxLen && str[i]; ++i)
        {
            if (!isprint(str[i]))
            {
                str[i] = '.';
            }
        }
        outs << str;
    }
    outs << '"';
    return true;
}


////////////////////////////////////////////////////////////////
SharedString* PointerTypeImpl::read (
    DebugSymbol*        sym,
    DebugSymbolEvents*  events
    ) const
{
    assert(sym);
    assert(sym->thread());

    addr_t addr = sym->addr();

    if ((addr == 0) && sym->is_constant() && sym->value())
    {
        addr = strtoull(sym->value()->c_str(), 0, 0);
    }
    if (addr == 0)
    {
        if (sym->value() && is_cstring())
        {
            return sym->value();
        }
        return null_ptr();
    }
    if (!sym->is_constant())
    {
        if (sym->is_return_value())
        {
            addr = CHKPTR(sym->thread())->result();
        }
        else
        {
            addr = Value<addr_t>().read(*sym);
        }
    }

    ostringstream outs;

    if (addr == 0)
    {
        return null_ptr();
    }
    else if (!read_as_string(type(), events, sym, addr, outs))
    {
        outs << hex << showbase << addr;
    }
    SharedString* name = type().name();
    assert(name);

    RefPtr<DebugSymbolImpl> child =
        DebugSymbolImpl::create(sym->reader(),
                                *sym->thread(),
                                type(),
                                *name,
                                addr);

    sym->add_child(child.get());

    if (events && events->is_expanding(sym))
    {
        child->read(events);
    }
    return shared_string(outs.str()).detach();
}


////////////////////////////////////////////////////////////////
void PointerTypeImpl::write(DebugSymbol* sym, const Buffer* buf) const
{
    assert(sym);
    assert(buf);
    assert(buf->size() == sizeof(addr_t));

    word_t val = *(word_t*)buf->data();

    RefPtr<Thread> thread = sym->thread();
    assert(thread);

    if (sym->is_return_value())
    {
        interface_cast<Runnable&>(*thread).set_result(val);
    }
    else
    {
        thread->write_data(sym->addr(), &val, 1);
    }
}



////////////////////////////////////////////////////////////////
bool PointerTypeImpl::is_cstring() const
{
    bool result = false;

    if (pointerOrReference_ == POINTER)
    {
    #if 0
        if (type().bit_size() == 8 || type().bit_size() == 7)
        {
            DataType* tp = &type();

            while (QualifiedType* qt = interface_cast<QualifiedType*>(tp))
            {
                tp = qt->remove_qualifier();
            }
            if (tp)
            {
                const SharedString* typeName = CHKPTR(tp->name());
                result = typeName->is_equal("char")
                      || typeName->is_equal("signed char")
                      // gcc-4.0.0:
                      || typeName->is_equal("int7_t")
                      || typeName->is_equal("uint7_t");
            }
        }
    #else
        result = is_char_type(type()) || is_wide_char_type(type());
    #endif
    }
    return result /* || is_ustring() */;
}


////////////////////////////////////////////////////////////////
bool PointerTypeImpl::is_ustring() const
{
    bool result = false;

    if (pointerOrReference_ == POINTER)
    {
    #if 0
        if (type().bit_size() == 32) // linux: sizeof (wchar_t) == 4
        {
            DataType* tp = &type();

            while (QualifiedType* qt = interface_cast<QualifiedType*>(tp))
            {
                tp = qt->remove_qualifier();
            }
            if (tp)
            {
                const SharedString* typeName = CHKPTR(tp->name());
                result = typeName->is_equal("wchar_t");
            }
        }
    #else
        result = is_wide_char_type(type());
    #endif
    }
    return result;
}


////////////////////////////////////////////////////////////////
int PointerTypeImpl::compare(const char* lhs, const char* rhs) const
{
    assert(lhs);
    assert(rhs);

    char* p1 = 0, *p2 = 0;
    addr_t lval = strtoul(lhs, &p1, 0);
    addr_t rval = strtoul(rhs, &p2, 0);

    assert(p1 && p2);

    if (is_cstring())
    {
        if ((*p1 != 0) != (*p2 != 0))
        {
            // Comparing a string to a pointer --
            // we don't have much info, just assume
            // the values are equal.
            return 0;
        }
        else if (*p1)
        {
            return strcmp(lhs, rhs);
        }
    }
    if (lval < rval)
    {
        return -1;
    }
    return lval > rval;
}



////////////////////////////////////////////////////////////////
MacroTypeImpl::MacroTypeImpl(TypeSystem& types)
    : DataTypeImpl<MacroType>(type_name())
{
    impl_ = types.get_pointer_type(GET_INT_TYPE(types, char));
}


////////////////////////////////////////////////////////////////
SharedString* MacroTypeImpl::type_name()
{
    static const RefPtr<SharedString> name = shared_string("<MACRO>");
    return name.get();
}


////////////////////////////////////////////////////////////////
SharedString* MacroTypeImpl::read(DebugSymbol* symbol, DebugSymbolEvents*) const
{
    return symbol ? symbol->value() : NULL;
}


////////////////////////////////////////////////////////////////
void MacroTypeImpl::write(DebugSymbol*, const Buffer*) const
{
    assert(false); // pseudo-type, should never be called
}


////////////////////////////////////////////////////////////////
size_t MacroTypeImpl::parse(const char* str, Unknown2* unk) const
{
    return impl_->parse(str, unk);
}


////////////////////////////////////////////////////////////////
bool MacroTypeImpl::is_equal(const DataType* other) const
{
    return interface_cast<const MacroType*>(other) != NULL;
}


////////////////////////////////////////////////////////////////
ConstTypeImpl::ConstTypeImpl(DataType& type)
    : DecoratorType<ConstType>(type.name(), type, type.bit_size())
{
    update_name();
}


////////////////////////////////////////////////////////////////
void ConstTypeImpl::update_name()
{
    set_name(*type().name()->append(" const"));
}


////////////////////////////////////////////////////////////////
VolatileTypeImpl::VolatileTypeImpl(DataType& type)
    : DecoratorType<VolatileType>(type.name(), type, type.bit_size())
{
    update_name();
}


////////////////////////////////////////////////////////////////
void VolatileTypeImpl::update_name()
{
    set_name(*type().name()->append(" volatile"));
}


////////////////////////////////////////////////////////////////
ClassTypeImpl::ClassTypeImpl (
    TypeSystem*     types,
    SharedString*   name,
    size_t          bitSize,
    bool            isUnion
)
  : BaseType(name, bitSize)
  , typeSystem_(types)
  , bases_(NULL)
  , methods_(NULL)
  , rttiComputed_(false)
  , isUnion_(isUnion)
  , virtualBasesCount_(0)
{
    unqualifiedName_ = this->name();
    string tmp;
    if (this->name())
    {
        tmp = this->name()->c_str();
    }
    size_t n = tmp.rfind("::");
    if (n != string::npos)
    {
        unqualifiedName_ = shared_string(tmp.substr(n + 2));
    }
    if (types)
    {
        unqualifiedName_ = types->get_string(unqualifiedName_.get());
    }
}


////////////////////////////////////////////////////////////////
ClassTypeImpl::~ClassTypeImpl() throw()
{
    assert(ref_count() == 0);
}


////////////////////////////////////////////////////////////////
SharedString* ClassTypeImpl::unqualified_name() const
{
    assert(unqualifiedName_.get());
    return unqualifiedName_.get();
}


////////////////////////////////////////////////////////////////
void ClassTypeImpl::compute_full_name(const RefPtr<SharedString>& linkageName)
{
    if (linkageName && (unqualified_name() == name()))
    {
        // The class name may be not fully-qualified (because of
        // lack of support for namespaces in the STABS format,
        // DWARF-1, etc).
        //
        // The method linkage name may contain
        // namespaces, nested classes, etc.
        //
        // The linkage name is used here to determine the
        // fully-qualified name of this class.
        //
        const string fullname = demangle(linkageName->c_str(), false);
        const size_t pos = fullname.rfind("::");

        if ((pos != string::npos)
            && (pos != fullname.find("::"))
            && (fullname.find_first_of(" <") == string::npos) // no templates, pls
           )
        {
            string tmp = fullname.substr(0, pos);
            RefPtr<SharedString> name;

            if (RefPtr<TypeSystem> types = typeSystem_.ref_ptr())
            {
                name = types->get_string(tmp.c_str());
            }
            else
            {
                name = shared_string(tmp);
            }
            set_name(*name);
        }
    }
}


////////////////////////////////////////////////////////////////
RefPtr<MethodImpl>
ClassTypeImpl::add_method(RefPtr<SharedString> funName,
                          RefPtr<SharedString> linkageName,
                          FunType*        funType,
                          Access          access,
                          bool            isVirtual,
                          Qualifier       qualifier)
{
    if (funName)
    {
        if (funName->is_equal("__base_ctor") ||
            funName->is_equal("__comp_ctor"))
        {
            funName = unqualifiedName_;

            if (funName == NULL)
            {
                funName = this->name();
            }
        }
        else if (RefPtr<TypeSystem> types = typeSystem_.ref_ptr())
        {
            funName = types->get_string(funName.get());
        }
        compute_full_name(linkageName);
    }

    if (!linkageName)
    {
        linkageName = funName;
    }
    RefPtr<MethodImpl> memfun = new MethodImpl( funName.get(),
                                                linkageName,
                                                funType,
                                                access,
                                                isVirtual,
                                                qualifier);
#ifdef DEBUG
    if (funType)
    {
        const size_t count = funType->param_count();
        for (size_t i = 0; i != count; ++i)
        {
            CHKPTR(funType->param_type(i));
        }
    }
#endif
    if (!methods_.get())
    {
        methods_.reset(new MethodList);
    }
    methods_->push_back(memfun);
    return memfun;
}


////////////////////////////////////////////////////////////////
void
ClassTypeImpl::add_template_type_param(TemplateTypeParam& param)
{
    if (!templateTypeParams_.get())
    {
        templateTypeParams_.reset(new TemplateTypeList);
    }
    templateTypeParams_->push_back(&param);
}


////////////////////////////////////////////////////////////////
size_t ClassTypeImpl::enum_template_type_param(
    EnumCallback<TemplateTypeParam*>* callback) const
{
    if (templateTypeParams_.get())
    {
        if (callback)
        {
            TemplateTypeList::const_iterator i =
                templateTypeParams_->begin();
            for (; i != templateTypeParams_->end(); ++i)
            {
                callback->notify(i->get());
            }
        }
        return templateTypeParams_->size();
    }
    return 0;
}


////////////////////////////////////////////////////////////////
bool ClassTypeImpl::is_equal(const DataType* type) const
{
    if (this == type)
    {
        return true;
    }
    if (!type)
    {
        return false;
    }
    if (is_cv_qualified(type))
    {
        return false;
    }
    if (const ClassType* other = interface_cast<const ClassType*>(type))
    {
        if (bit_size() != other->bit_size() || is_cv_qualified(type))
        {
            return false;
        }
        if (!name()->is_equal2(type->name(), true)
         && !unqualified_name()->is_equal2(other->unqualified_name(), true))
        {
            return false;
        }
        return true;
    }
    return false;
}


////////////////////////////////////////////////////////////////
size_t ClassTypeImpl::parse(const char* str, Unknown2* unk) const
{
    if (Variant* var = interface_cast<Variant*>(unk))
    {
        var->set_type_tag(Variant::VT_OBJECT);
    }
    return 0;
}


////////////////////////////////////////////////////////////////
static RefPtr<DataType> lookup_type(DebugSymbol& sym, SharedString& typeName)
{
    DebugInfoReader* reader = CHKPTR(sym.reader());
    Thread* thread = CHKPTR(sym.thread());

    //note: this environment variable is also checked by
    //the DWARF reader plugin
    static const LookupScope scope =
        env::get("ZERO_EXPENSIVE_TYPE_LOOKUPS", 1) > 1
        ? LOOKUP_ALL
        : LOOKUP_MODULE;

    const char* name = typeName.c_str();
    const addr_t addr = sym.addr();

    RefPtr<DataType> type =
        reader->lookup_type(thread, name, addr, scope);

    // strip spaces and try again (for templates such as Foo<X, Y>)
    if (!type)
    {
        bool retry = false;
        string tmp;

        while (*name)
        {
            if (isspace(*name))
            {
                retry = true;
            }
            else
            {
                tmp += *name;
            }
            ++name;
        }
        if (retry)
        {
            type = reader->lookup_type(thread, tmp.c_str(), addr, scope);
        }
    }
    return type;
}


////////////////////////////////////////////////////////////////
SharedString* ClassTypeImpl::check_rtti (
    DebugSymbol&        sym,
    DebugSymbolEvents*  events
    ) const
{
    // preconditions
    assert(sym.reader());
    assert(sym.thread());
    assert(this->name());

    assert(interface_cast<ClassType*>(sym.type()));
    assert(!interface_cast<PointerType*>(sym.type()));

    SharedString* result = NULL;
    // prevent infinite recursion
    if (!rtti(sym.thread()) || rtti_->processing_)
    {
        return result;
    }
    // prevent infinite recursion
    Temporary<bool> setFlag(rtti_->processing_, true);

    RefPtr<SharedString> typeName = rtti_->type_name(sym);
    if (!typeName)
    {
        IF_DEBUG_RTTI(
            clog << __func__ << ": typename(";
            clog << sym.name() << ") is NULL\n";
        )
        return result;
    }

    RefPtr<DataType> type = lookup_type(sym, *typeName);
    IF_DEBUG_RTTI(
        clog << "lookup_type(" << typeName->c_str() << ")=";
        clog << type.get() << endl;
    )
    assert(!interface_cast<QualifiedType*>(type.get()));

    if (!type || type->is_equal(remove_qualifiers(sym.type())))
    {
        return result;
    }
    // rtc == run-time-class
    if (ClassType* rtc = interface_cast<ClassType*>(type.get()))
    {
        // if rtc points to one of our base classes, bail out;
        // base parts are handled when we read the aggregated class
        if (this->lookup_base(rtc->name(), NULL, true))
        {
            return result;
        }
        IF_DEBUG_RTTI(
            clog << __func__ << ": " << name() << " is a " << rtc->name();
            clog << " (symbol type: " << sym.type()->name() << ")\n";
        )

        RefPtr<DebugSymbol> oldSym = sym.clone();
        DebugSymbolImpl& impl = interface_cast<DebugSymbolImpl&>(sym);

        // set the symbol's type to the runtime class type
        if (impl.set_type(*type))
        {
            const addr_t addr = adjust_base_to_derived(sym, *this, *rtc);

            IF_DEBUG_RTTI(
                clog << __func__ << ": orig=0x" << hex << sym.addr();
                clog << " adjusted=0x" << addr << dec << endl;
            )
            impl.set_name(typeName.get());
            if (addr)
            {
                impl.set_addr(addr);
            }
            if (events)
            {
                events->symbol_change(&impl, oldSym.get());
            }
            sym.read(events);
        }
        result = sym.value();
    }
    else
    {
        cerr << __func__ << ": " << type->name() << ": not a class\n";
    }
    return result;
}


////////////////////////////////////////////////////////////////
SharedString* ClassTypeImpl::read (
    DebugSymbol*        sym,
    DebugSymbolEvents*  events
    ) const
{
    assert(sym);
    bool checkRTTI = false;

    // If this class is aggregated or referred to by
    // means of a pointer, then turn RTTI discovery on.
    //
    // If the class is aggregated by containment, we don't want
    // RTTI checking since a) we know the exact type anyway;
    // b) we don't want to run into infinite recursion, where Base
    // is determined to be a Derived, which has a Base, which points
    // to a Derived, which has a Base ...

    if (DebugSymbol* parent = sym->parent())
    {
        if (interface_cast<PointerType*>(parent->type()))
        {
            checkRTTI = true;
        }
    }
    addr_t addr = sym->addr();
    return read_impl(*sym, addr, events, checkRTTI);
}


////////////////////////////////////////////////////////////////
SharedString* ClassTypeImpl::read_impl (
    DebugSymbol&        sym,
    addr_t              addr,
    DebugSymbolEvents*  events,
    bool                checkRTTI
    ) const
{
    RefPtr<SharedString> result;

    if (checkRTTI
        && use_auto_rtti(typeSystem_.ref_ptr())
        && sym.reader())
    {
        // attempt to auto-magically discover the runtime type
        result = check_rtti(sym, events);
    }
    if (!result)
    {
        ostringstream outs;
        outs.setf(ios::showbase);
        outs << "{...} <" << hex << sym.addr() << '>';
        result = shared_string(outs.str());

        if (bases_.get())
        {
            BaseList::const_iterator i(bases_->begin());
            for (; i != bases_->end(); ++i)
            {
                (*i)->read(sym, addr, events);
            }
        }
        MemberList::const_iterator j(members_.begin());
        for (; j != members_.end(); ++j)
        {
            (*j)->read(sym, addr, events);
        }
    }
    return result.detach();
}


/**
 * Aggregate a base class part
 */
void ClassTypeImpl::add_base (
    DataType&   type,   // the type of the base class
    off_t       bitOffset,
    Access      access,
    bool        isVirtualBase,
    ClassType*  owner)
{
    if (!interface_cast<ClassType*>(&type)
     && !interface_cast<IndirectType*>(&type))
    {
        // todo: I think this may happen because of lack of
        // namespace and scope resolution when dealing with
        // IndirectType information in STABS -- investigate
        clog << "*** Warning: " << type.name() << " is not a class\n";
    }
    size_t vindex = 0;
    if (isVirtualBase)
    {
        vindex = ++virtualBasesCount_;
        assert(virtualBasesCount_);
    }
    if (owner)
    {
        assert(interface_cast<ClassTypeImpl*>(owner) == this);
#ifdef DEBUG
        clog << __func__ << ": " << owner->name();
        clog << " (" << owner->_name() << ") " << type.name() << endl;
#endif
    }
    else
    {
        owner = this;
    }
    RefPtr<BaseImpl> base =
        new BaseImpl(access, bitOffset, type.bit_size(), type, vindex);

    if (bases_.get() == NULL)
    {
        bases_.reset(new BaseList);
    }
    bases_->push_back(base);

#ifdef DEBUG
    if (debug_object_layout())
    {
        const SharedString* name = type.name();
        clog << __func__ << ": " << this->name() << endl;
        clog << " " << name << ": offs=" << bitOffset / byte_size << endl;
        clog << " " << name << ": base=" << base->offset() << endl;
        clog << " " << name << ": size=" << type.size() << endl;
        clog << " " << name << ": virt=" << boolalpha << isVirtualBase << endl;
        clog << " " << name << ": indx=" << base->virtual_index() << endl;
    }
#endif
}


////////////////////////////////////////////////////////////////
MemberImpl& ClassTypeImpl::add_member
(
    const RefPtr<SharedString>& name,
    RefPtr<SharedString>        linkageName,
    off_t                       offs,
    size_t                      bitSize,
    DataType&                   dataType,
    bool                        isStatic,
    DebugSymbol*                value
)
{
    RefPtr<TypeSystem> types = typeSystem_.ref_ptr();
    MemberPtr mptr =
        new MemberImpl(types.get(),
                        name.get(),
                        offs,
                        bitSize,
                        dataType,
                        isStatic,
                        value);
    members_.push_back(mptr);
    if (linkageName)
    {
        if (types)
        {
            linkageName = types->get_string(linkageName.get());
        }
        mptr->set_linkage_name(linkageName);
        compute_full_name(linkageName);
    }

    if (isStatic)
    {
        assert(bitSize == 0);
    }
/* too verbose

    if (debug_object_layout())
    {
        const SharedString* name = dataType.name();

        clog << __func__ << ": " << this->name() << endl;
        clog << " " << name << ": offs=" << offs / byte_size << endl;
        clog << " " << name << ": size=" << bitSize / byte_size << endl;
    }
 */
    return *mptr;
}


////////////////////////////////////////////////////////////////
RTTI* ClassTypeImpl::rtti(Thread* thread) const
{
    if (!rttiComputed_ && !rtti_)
    {
        MemberList::const_iterator i = members_.begin();
        for (; i != members_.end(); ++i)
        {
            MemberPtr mem = *i;

            // use the pointer to vtable to get runtime type info
            if (CHKPTR(mem->name())->is_equal(".vptr"))
            {
                rtti_ = new RTTI(mem);
                break;
            }
        }
    }
    // still null? try base parts
    if (!rttiComputed_ && !rtti_ && bases_.get())
    {
        BaseList::const_iterator i = bases_->begin();
        for (; i != bases_->end(); ++i)
        {
            const RefPtr<BaseImpl>& base = *i;
            if (base->virtual_index())
            {
                continue;
            }
            if (inherit_rtti(*base, thread))
            {
                break;
            }
        }
    }
    rttiComputed_ = true;
    return rtti_.get();
}


////////////////////////////////////////////////////////////////
bool ClassTypeImpl::inherit_rtti(BaseClass& mem, Thread* thread) const
{
    assert(!rtti_);
    bool result = false;

    if (ClassType* klass = interface_cast<ClassType*>(mem.type()))
    {
        if (klass->has_vtable(thread))
        {
            rtti_ = new RTTI(*klass->rtti(thread));
            rtti_->add_vptr_bit_offset(mem.bit_offset());
            result = true;
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
bool ClassTypeImpl::has_vtable(Thread* thread) const
{
    return rtti(thread);
}


////////////////////////////////////////////////////////////////
const BaseClass* ClassTypeImpl::lookup_base (
    const SharedString* name,
    off_t*              offset,
    bool                recursive
    ) const
{
    assert(name);
    const BaseClass* result = 0;

    if (debug_object_layout())
    {
        clog << __func__ << ": " << this->name() << ": (";
        clog << _name()  << ") " << base_count() << " base(s)\n";
    }
    if (bases_.get())
    {
        BaseList::const_iterator i = bases_->begin();
        for (; i != bases_->end(); ++i)
        {
            DataType* type = (*i)->type();
            assert(type);

            if (name->is_equal2(type->name()))
            {
                result = (*i).get();

                if (offset)
                {
                    *offset = CHKPTR(result)->offset();

                    if (debug_object_layout())
                    {
                        clog << __func__ << ": " << name << " in ";
                        clog << this->name() << ": offs=" << *offset << endl;
                    }
                }
                break;
            }
            else if (recursive)
            {
                if (ClassType* klass = interface_cast<ClassType*>(type))
                {
                    result = klass->lookup_base(name, offset, recursive);

                    if (result)
                    {
                        if (offset)
                        {
                            *offset += (*i)->offset();

                            if (debug_object_layout())
                            {
                                clog << __func__ << ": " << name;
                                clog << " in " << this->name();
                                clog << ": Offs=" << *offset << endl;
                            }
                        }
                        break;
                    }
                }
            }
        }
    } // bases_.get()

    return result;
}


////////////////////////////////////////////////////////////////
template<typename T>
static void check_range(const char* fun, const T& list, size_t n)
{
    if (n >= list.size())
    {
        ostringstream err;

        err << fun << "[" << n << "]: out of range, size=";
        err << list.size();

        throw out_of_range(err.str());
    }
}


template<typename T>
static void check_range_ptr(const char* fun, const T* list, size_t n)
{
    if ((list == NULL) || (n >= list->size()))
    {
        ostringstream err;

        err << fun << "[" << n << "]: out of range, size=";
        err << (list ? list->size() : 0);

        throw out_of_range(err.str());
    }
}


////////////////////////////////////////////////////////////////
const BaseClass* ClassTypeImpl::base(size_t n) const
{
    check_range_ptr(__func__, bases_.get(), n);
    return (*bases_)[n].get();
}


////////////////////////////////////////////////////////////////
const MemberImpl* ClassTypeImpl::member(size_t n) const
{
    check_range(__func__, members_, n);
    return members_[n].get();
}


////////////////////////////////////////////////////////////////
size_t ClassTypeImpl::method_count() const
{
    return methods_.get() ? methods_->size() : 0;
}


////////////////////////////////////////////////////////////////
const MethodImpl* ClassTypeImpl::method(size_t n) const
{
    check_range_ptr(__func__, methods_.get(), n);
    return (*methods_)[n].get();
}


////////////////////////////////////////////////////////////////
string ClassTypeImpl::description() const
{
    string result = BaseType::description();

    if (bases_.get())
    {
        result += _line;
        result += "\nBases:\n";

        BaseList::const_iterator i = bases_->begin();
        for (; i != bases_->end(); ++i)
        {
            result +=
                (boost::format("Offset=%1% Size=%2% %3%\n")
                    % (*i)->offset()
                    % (*i)->type()->size()
                    % (*i)->name(NULL)->c_str()).str();
        }
    }
    if (!members_.empty())
    {
        result += _line;
        result += "\nMembers:\n";

        MemberList::const_iterator j = members_.begin();
        for (; j != members_.end(); ++j)
        {
            MemberPtr m = *j;

            result +=
                (boost::format("Offset=%1% Size=%2% %3% %4% (%5%)\n")
                    % (m->bit_offset() / byte_size)
                    % m->type()->size()
                    % (m->is_static() ? "static " : "")
                    % m->name()->c_str()
                    % m->type()->name()->c_str()).str();
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
size_t
ClassTypeImpl::enum_template_value_param(
        EnumCallback<TemplateValueParam*>*) const
{
    return 0; //todo
}


////////////////////////////////////////////////////////////////
MemberImpl::MemberImpl
(
    TypeSystem*     typeSys,
    SharedString*   name,
    off_t           bitOffs,
    bitsize_t       bitSize,
    DataType&       type,
    bool            isStatic,
    DebugSymbol*    value
)
  : isStatic_(isStatic)
  , isBitField_(false)
  , bitOffs_(bitOffs)
  , bitSize_(bitSize)
  , type_(&type)
  , value_(value)
{
    if (!name)
    {
        name_ = unnamed_type();
    }
    else if (typeSys)
    {
        name_ = typeSys->get_string(name);
    }
    else
    {
        name_ = name;
    }

#ifdef DEBUG
    if (bitOffs % byte_size)
    {
        const RefPtr<DataType>& type = type_;
        // if it does not start at a round number of bytes,
        // it is expected to be a bit field; bit fields are expected
        // to be of an integral type (at least in C/C++)
        if (!interface_cast<IntType>(type)
         && !interface_cast<EnumType>(type)
         && !interface_cast<IndirectType>(type))
        {
            clog << type->_name() << " is not an IntType\n";
            abort();
        }
    }
#endif // DEBUG
/* TODO: make sure that we conform to
   http://www.codesourcery.com/cxx-abi/abi.html

 "If D is a (possibly unnamed) bitfield whose declared
 type is T and whose declared width is n bits:

 There are two cases depending on sizeof(T) and n:

    1. If sizeof(T)*8 >= n, the bitfield is allocated as required
    by the underlying C psABI, subject to the constraint that a
    bitfield is never placed in the tail padding of a base class of C.

    If dsize(C) > 0, and the byte at offset dsize(C) - 1 is partially
    filled by a bitfield, and that bitfield is also a data member declared
    in C (but not in one of C's proper base classes), the next available
    bits are the unfilled bits at offset dsize(C) - 1. Otherwise, the next
    available bits are at offset dsize(C).

    Update align(C) to max (align(C), align(T)).

    2. If sizeof(T)*8 < n, let T' be the largest integral POD type with
    sizeof(T')*8 <= n. The bitfield is allocated starting at the next offset
    aligned appropriately for T', with length n bits. The first sizeof(T)*8
    bits are used to hold the value of the bitfield, followed by
    n - sizeof(T)*8 bits of padding.

    Update align(C) to max (align(C), align(T')).

    In either case, update dsize(C) to include the last byte containing (part of)
    the bitfield, and update sizeof(C) to max(sizeof(C),dsize(C))".

 */
    if (IntType* intType = interface_cast<IntType*>(&type))
    {
        if (is_bit_field(bitSize, intType->bit_size()))
        {
            isBitField_ = true;

            // C/C++ language limit: bitfields cannot exceed MAX_BITS
            if (bitSize > MAX_BITS)
            {
                bitSize = MAX_BITS;
            }

            const bool isSigned = intType->is_signed();
            assert(bitSize);

            type_ = new IntTypeImpl(bitSize, bitOffs, isSigned);
        }
    }
    else if (bitSize % byte_size)
    {
        // in C/C++ bitfields must be of integral type
        assert(bitSize <= MAX_BITS);
        assert(type.name());

        if (interface_cast<EnumType*>(&type))
        {
            type_ = new IntTypeImpl(bitSize, bitOffs, false);
        }
    }
}


////////////////////////////////////////////////////////////////
DataType* MemberImpl::type() const
{
    return CHKPTR(get_pointer(type_));
}


////////////////////////////////////////////////////////////////
SharedString* MemberImpl::name() const
{
    return name_.get();
}


////////////////////////////////////////////////////////////////
void MemberImpl::read (
    DebugSymbol&        sym,
    addr_t              addr,
    DebugSymbolEvents*  events)
{
    if (!isBitField_)
    {
        addr += (bit_offset() / byte_size);
    }
    else
    {
        assert(bit_size() <= MAX_BITS);

        size_t wordSize = sizeof(word_t);

        if (sym.thread() && sym.thread()->is_32_bit())
        {
            wordSize = 4;
        }
        // bitfields are of type int, round up to machine-words
        addr += wordSize * (bit_offset() / (wordSize * byte_size));
    }

    read(sym, addr, events, 0);
}


////////////////////////////////////////////////////////////////
void MemberImpl::read (
    DebugSymbol&        sym,
    addr_t              addr,
    DebugSymbolEvents*  events,
    unsigned long     //bitmask
    )
{
    assert(sym.thread());

    if (value_) // constant member?
    {
        sym.add_child(value_.get());

        if (events && events->is_expanding(&sym))
        {
            value_->read(events);
        }
        return;
    }

    if (isStatic_)
    {
        SymbolEnum symEnum;

        CHKPTR(sym.type());

        SymbolMap* symbols = CHKPTR(sym.thread()->symbols());

        if (linkageName_)
        {
            symbols->enum_symbols(linkageName_->c_str(),
                                  &symEnum,
                                  SymbolTable::LKUP_ISMANGLED);
        }
        else
        {
            string symName(CHKPTR(sym.type()->name())->c_str());
            symName += "::";
            symName += CHKPTR(name())->c_str();

            symbols->enum_symbols(symName.c_str(), &symEnum);
        }
        if (symEnum.empty())
        {
            return;
        }

        if (!type_->is_fundamental() && has_loop(sym, addr))
        {
            return;
        }
        addr = symEnum.front()->addr();
    }
    add_child_symbol(sym, addr, events);
}


////////////////////////////////////////////////////////////////
void MemberImpl::add_child_symbol(
    DebugSymbol& parent,
    addr_t addr,
    DebugSymbolEvents* events)
{
    // create a tentative child symbol, it may change
    // after applying DataFilter::transform()

    // foetus == tentative child, clever oh so clever
    RefPtr<DebugSymbolImpl> foetus =
        DebugSymbolImpl::create(parent.reader(),
                               *parent.thread(),
                               *type_,
                               *name(),
                               addr);

    bool readChild = false;

    if (events)
    {
        if (events->is_expanding(&parent))
        {
            readChild = true;
            foetus->read(events);
        }
    }

    foetus->set_tentative_parent(&parent);
    RefPtr<DebugSymbol> child(apply_transform(*foetus, &parent, events));

    parent.add_child(child.get());

    if (events)
    {
        events->symbol_change(child.get(), foetus.get());
    }
    if (readChild)
    {
        child->read(events);
    }
}


////////////////////////////////////////////////////////////////
BaseImpl::BaseImpl (
    Access              access,
    off_t               bitOffs,
    size_t              bitSize,
    DataType&           type,
    size_t              vindex
)
  : impl_(new MemberImpl(NULL, NULL, bitOffs, bitSize, type, false))
  , vindex_(vindex)
  , access_(access)
{
}


////////////////////////////////////////////////////////////////
DataType* BaseImpl::type() const
{
    assert(impl_.get());
    return impl_->type();
}


////////////////////////////////////////////////////////////////
SharedString* BaseImpl::name(TypeSystem* ts) const
{
    assert(impl_.get());

    if (!impl_->name() || impl_->name()->is_equal2(unnamed_type()))
    {
        const char* inherit = ":public ";
        switch (access_)
        {
        case ACCESS_PRIVATE:
            inherit = ":private ";
            break;
        case ACCESS_PROTECTED:
            inherit = ":protected ";
            break;
        case ACCESS_PUBLIC:
            break;
        }

        RefPtr<SharedString> name = shared_string(inherit);
        DataType* type = CHKPTR(impl_->type());
        name = name->append(type->name()->c_str());
        if (ts)
        {
            name = ts->get_string(name.get());
        }
        impl_->set_name(name);
    }
    return impl_->name();
}


////////////////////////////////////////////////////////////////
off_t BaseImpl::bit_offset() const
{
    assert(impl_.get());
    return impl_->bit_offset();
}


////////////////////////////////////////////////////////////////
void BaseImpl::read (
    DebugSymbol&        sym,
    addr_t              addr,
    DebugSymbolEvents*  events)
{
    Thread* thread = CHKPTR(sym.thread());
    const off_t off = offset();

    if (off < 0)
    {
        assert(virtual_index());
        // stabs+: the offset is the negative
        // offset in the virtual table where the
        // actual offset that I am looking for is

        addr += get_vtable_adjustment(*thread, addr, off);
    }
    else
    {
        addr += off;
    }
    assert(impl_.get());

    disable_auto_rtti_in_scope(impl_->type(), thread);

    //query for the TypeSystem interface
    TypeSystem* types = interface_cast<TypeSystem*>(thread);

    this->name(types); // force name re-calculation, if needed
    impl_->read(sym, addr, events, 0);
}


////////////////////////////////////////////////////////////////
off_t BaseImpl::offset() const
{
    assert(impl_.get());
    assert((impl_->bit_size() % byte_size) == 0);
    assert((bit_offset() % byte_size) == 0);

    return bit_offset() / byte_size;
}


////////////////////////////////////////////////////////////////
MethodImpl::MethodImpl (
    SharedString* name,
    const RefPtr<SharedString>& linkage,
    FunType* type,
    Access access,
    bool isVirtual,
    Qualifier qualifier
)
  : name_(name)
  , linkageName_(linkage)
  , funType_(type)
  , access_(access)
  , isVirtual_(isVirtual)
  , isInline_(false)
  , qualifier_(qualifier)
  , callingConvention_(CC_NORMAL)
  , addr_(0)
  , vtableOffset_(0)
{
}


////////////////////////////////////////////////////////////////
addr_t MethodImpl::start_addr() const
{
    return addr_;
}


////////////////////////////////////////////////////////////////
addr_t MethodImpl::end_addr() const
{
    return 0; // todo: compute the end addres of the method
}


////////////////////////////////////////////////////////////////
off_t MethodImpl::vtable_offset() const
{
    return vtableOffset_;
}


////////////////////////////////////////////////////////////////
void MethodImpl::set_calling_convention(CallingConvention cc)
{
    assert(callingConvention_ == CC_NORMAL); // set once
    callingConvention_ = cc;
}


////////////////////////////////////////////////////////////////
bool MethodImpl::is_static() const
{
    bool isStatic = true;

    // if the first parameter type of this method's signature
    // is a class, then
    // enumerate the methods of the class; if no method matches
    // then this is a static method
    //
    RefPtr<FunType> funType = funType_.ref_ptr();
    if (funType && funType->param_count())
    {
        DataType* type = remove_qualifiers(funType->param_type(0));
        assert(type);
        if (PointerType* ptr = interface_cast<PointerType*>(type))
        {
            type = remove_qualifiers(ptr->pointed_type());

            if (ClassType* klass = interface_cast<ClassType*>(type))
            {
                const size_t numMethods = klass->method_count();
                for (size_t i = 0; i != numMethods; ++i)
                {
                    const Method* method = klass->method(i);
                    if (method == this ||
                        (this->name()->is_equal2(method->name()) &&
                         this->type()->is_equal(method->type()))
                       )
                    {
                        isStatic = false;
                        break;
                    }
                }
            }
        }
    }
    return isStatic;
}


////////////////////////////////////////////////////////////////
ArrayTypeImpl::ArrayTypeImpl
(
    TypeSystem& typeSys,
    DataType& type,
    const Range& range
)
 : BaseType(get_name(typeSys, type, range),
            get_bit_size(type, range))
 , elemType_(&type)
 , range_(range)
 , wordSize_(typeSys.word_size())
{
}


////////////////////////////////////////////////////////////////
SharedString*
ArrayTypeImpl::get_name(TypeSystem& typesys,
                        DataType& elem,
                        const Range& range)
{
    ostringstream name;

    name << '(' << elem.name()->c_str()
         << ")[" << range.second - range.first + 1 << ']';

#if 0
    return shared_string(name.str()).detach();
#else
    //
    //  go thru the TypeSystem string pool
    //
    return typesys.get_string(name.str().c_str(), name.str().length());
#endif
}


////////////////////////////////////////////////////////////////
bool ArrayTypeImpl::is_equal(const DataType* type) const
{
    if (const ArrayType* at = interface_cast<const ArrayType*>(type))
    {
        return elem_type()->is_equal(at->elem_type())
            && elem_count() == at->elem_count()
            && !is_cv_qualified(type);
    }
    return false;
}


////////////////////////////////////////////////////////////////
size_t ArrayTypeImpl::get_bit_size(DataType& elem, const Range& range)
{
    bitsize_t bitSize = elem.bit_size();

    if (bitSize < byte_size)
    {
        bitSize = byte_size;
    }
    if (range.second >= range.first || (range.second == -1 && range.first == 0))
    {
        return bitSize * (range.second - range.first + 1);
    }
    throw invalid_argument("invalid range of array elements");
}


////////////////////////////////////////////////////////////////
SharedString* ArrayTypeImpl::read(
    DebugSymbol*        sym,
    DebugSymbolEvents*  events
   ) const
{
    assert(sym);
    assert(sym->thread());

    if (RefPtr<DataType> elemType = elemType_.ref_ptr())
    {
        ostringstream outs;

        addr_t addr = sym->addr();

        // same trick we did for pointers:
        if ((addr == 0) && sym->is_constant() && sym->value())
        {
            addr = strtoull(sym->value()->c_str(), 0, 0);
        }
        if (addr == 0)
        {
            if (SharedString* value = sym->value())
            {
                return value;
            }
            return null_ptr();
        }
        if (!read_as_string(*elemType, events, sym, addr, outs, elem_count()))
        {
            outs << '[' << hex << "0x" << addr << ']';

            DebugSymbolImpl& impl = interface_cast<DebugSymbolImpl&>(*sym);
            read_elements(impl, addr, events);
        }
        return shared_string(outs.str()).detach();
    }
    return NULL;
}


////////////////////////////////////////////////////////////////
size_t ArrayTypeImpl::parse(const char* str, Unknown2* unk) const
{
    char* ptr = 0;
    addr_t addr = 0;

    if (strcmp(str, "NULL") == 0)
    {
        ptr = const_cast<char*>(str) + 4;
    }
    else
    {
        addr = strtoul(str, &ptr, 0);
    }
    if (wordSize_ == 32)
    {
        put(unk, uint32_t(addr), Variant::VT_ARRAY);
    }
    else
    {
        put(unk, addr, Variant::VT_ARRAY);
    }
    if (ptr)
    {
        assert(ptr >= str);
        return ptr - str;
    }

    return 0;
}


////////////////////////////////////////////////////////////////
DataType* ArrayTypeImpl::elem_type() const
{
    return elemType_.ref_ptr().get();
}


////////////////////////////////////////////////////////////////
RefPtr<DataType> ArrayTypeImpl::elem_type_ref() const
{
    RefPtr<DataType> elemType = elemType_.ref_ptr();
    if (!elemType)
    {
        throw logic_error(__func__ + string(": null elem type in array"));
    }
    return elemType;
}


////////////////////////////////////////////////////////////////
size_t ArrayTypeImpl::elem_count() const
{
    return range_.second - range_.first + 1;
}


////////////////////////////////////////////////////////////////
void ArrayTypeImpl::read_elements(
    DebugSymbolImpl& array,
    addr_t addr,
    DebugSymbolEvents* events
    ) const
{
    DebugInfoReader* reader = array.reader();

    const uint64_t upper = range_.second - range_.first;

    DebugSymbolArray* elems = dynamic_cast<DebugSymbolArray*>(array.children().get());
    if (!elems)
    {
        elems = new DebugSymbolArray;
        array.set_children(auto_ptr<DebugSymbolCollection>(elems));
    }
    //
    // delegate the reading of array elements
    //
    elems->read(reader,
                array,
                addr,
                upper,
                *elem_type_ref(),
                events,
                range_.first);
}



////////////////////////////////////////////////////////////////
FunTypeImpl::~FunTypeImpl() throw()
{
}


////////////////////////////////////////////////////////////////
FunTypeImpl::FunTypeImpl (
    TypeSystem&         typeSystem,
    RefPtr<DataType>    retType,
    const ParamTypes*   paramTypes
)
  : BaseType(0, typeSystem.word_size())
  , retType_(retType.get())
  , vargs_(false)
  , strict_(true)
{
    assert(retType_.ref_ptr());

    init(typeSystem, paramTypes);
    set_name(*make_pointer_name(0));
}


////////////////////////////////////////////////////////////////
FunTypeImpl::FunTypeImpl (
    TypeSystem&         typeSystem,
    DataType&           retType,
    DataType* const*    argTypes,
    size_t              argCount
)
  : BaseType(0, typeSystem.word_size())
  , retType_(&retType)
  , vargs_(false)
  , strict_(true)
{
    ParamTypes paramTypes;

    if (argTypes && argCount)
    {
        paramTypes.assign(argTypes, argTypes + argCount);
    }
    init(typeSystem, &paramTypes);
    set_name(*make_pointer_name(0, retType_, &paramTypes));
}


////////////////////////////////////////////////////////////////
FunTypeImpl::FunTypeImpl (
    TypeSystem&         typeSystem,
    SharedString&       name,
    WeakDataTypePtr     retType,
    const ParamTypes*   paramTypes
)
  : BaseType(0, typeSystem.word_size())
  , retType_(retType.ref_ptr())
  , vargs_(false)
  , strict_(true)
{
    init(typeSystem, paramTypes);
    set_name(name);
}


////////////////////////////////////////////////////////////////
void FunTypeImpl::init(TypeSystem& types, const ParamTypes* paramTypes)
{
    observ_ = create_observer_delegate(this);

    assert(strict_);
    if (RefPtr<DataType> retType = retType_.ref_ptr())
    {
        retType->attach_to_observer(observ_.get());
    }
    else
    {
        retType_ = types.get_void_type();
    }

    if (paramTypes)
    {
        paramTypes_ = *paramTypes;

        ParamTypes::iterator i = paramTypes_.begin();
        for (; i != paramTypes_.end(); ++i)
        {
            assert((*i)->ref_count() > 0);
            (*i)->attach_to_observer(observ_.get());
        }
    }
    types.manage(this);
}


////////////////////////////////////////////////////////////////
bool FunTypeImpl::is_equal(const DataType* type) const
{
    if (const FunType* ft = interface_cast<const FunType*>(type))
    {
        assert(return_type());

        if (!return_type()->is_equal(ft->return_type())
          || param_count() != ft->param_count())
        {
            return false;
        }

        const size_t nparam = param_count();

        for (size_t i = 0; i != nparam; ++i)
        {
            if (!param_type(i)->is_equal(ft->param_type(i)))
            {
                return false;
            }
        }
        return true;
    }
    return false;
}


////////////////////////////////////////////////////////////////
RefPtr<SharedString> FunTypeImpl::make_pointer_name (
    const char* ptr,
    const WeakDataTypePtr& retType,
    const ParamTypes* paramTypes)
{
    string str;
    str.reserve(10);

    if (!retType.ref_ptr())
    {
        str += "void";
    }
    else
    {
        str += retType->name()->c_str();
    }

    str += '(';
    if (ptr)
    {
        str += ptr;
    }
    str += ')';
    str += '(';

    if (paramTypes)
    {
        ParamTypes::const_iterator i = paramTypes->begin();
        for (; i != paramTypes->end(); ++i)
        {
            if (i != paramTypes->begin())
            {
                str += ',';
            }
            str += (*i)->name()->c_str();
        }
    }
    str += ')';
    return shared_string(str);
}


////////////////////////////////////////////////////////////////
SharedString* FunTypeImpl::make_pointer_name (
    const char* ptr,
    RefTracker* tracker
    ) const
{
    RefPtr<SharedString> result =
        make_pointer_name(ptr, retType_, &paramTypes_);

    if (tracker)
    {
        tracker->register_object(result.get());
    }
    return result.detach();
}


////////////////////////////////////////////////////////////////
static bool is_hex(const char* str)
{
    if (str)
    {
        return str[0] == '0' && str[1] == 'x' && str[2];
    }
    return false;
}


////////////////////////////////////////////////////////////////
int FunTypeImpl::compare( const char* lhs, const char* rhs) const
{
    if (is_hex(lhs))
    {
        if (is_hex(rhs))
        {
            addr_t lval = strtoul(lhs, 0, 0);
            addr_t rval = strtoul(rhs, 0, 0);

            if (lval < rval)
            {
                return -1;
            }
            return lval > rval;
        }
        else
        {
            // hack: not enough info here, so assume equality
            return 0;
        }
    }
    else if (is_hex(rhs))
    {
        // same hack as above:
        return 0;
    }
    // otherwise, compare them as strings
    return BaseType::compare(lhs, rhs);
}


////////////////////////////////////////////////////////////////
SharedString* FunTypeImpl::read (
    DebugSymbol*        debugSym,
    DebugSymbolEvents*  events
    ) const
{
    assert(debugSym);

    const addr_t addr = debugSym->addr();

    string value;
    if (addr)
    {
        RefPtr<Thread> thread = debugSym->thread();
        assert(thread.get());

        RefPtr<SymbolMap> symbols = CHKPTR(thread->symbols());
        RefPtr<Symbol> sym = symbols->lookup_symbol(addr);

        if (sym.get())
        {
            if (events && events->numeric_base(debugSym) == 16)
            {
                ostringstream os;
                os << hex << showbase << sym->addr();
                value = os.str();
            }
            else
            {
                return sym->demangled_name();
            }
        }
        else
        {
            value = "???";
        }
    }
    else
    {
        value = "NULL";
    }
    return shared_string(value).detach();
}


////////////////////////////////////////////////////////////////
DataType* FunTypeImpl::return_type() const
{
    if (RefPtr<IndirectType> indirect =
        interface_cast<IndirectType>(retType_.ref_ptr()))
    {
        return indirect->link();
    }
    return retType_.ref_ptr().get();
}


////////////////////////////////////////////////////////////////
DataType* FunTypeImpl::param_type(size_t n) const
{
    check_range(__func__, paramTypes_, n);

    RefPtr<DataType> type = CHKPTR(paramTypes_[n].ref_ptr());

    if (RefPtr<IndirectType> i = interface_cast<IndirectType>(type))
    {
        if (DataType* link = i->link())
        {
            type = link;
        }
    }
    return type.detach();
}


////////////////////////////////////////////////////////////////
void FunTypeImpl::on_state_change(Subject* sub)
{
    assert(sub);

    // todo: verify that the subject is either one of the
    // param types or the return type
    RefPtr<SharedString> fname = make_pointer_name(0);
    set_name(*fname);
}


////////////////////////////////////////////////////////////////
PtrToMemberTypeImpl::PtrToMemberTypeImpl (
    TypeSystem& types,
    DataType& base,
    DataType& type
)
  : DataTypeImpl<PtrToMemberType>(make_name(types, base, type),
                                  types.word_size())
  , types_(&types)
  , base_(&base)
  , type_(&type)
{
}


////////////////////////////////////////////////////////////////
RefPtr<SharedString> PtrToMemberTypeImpl::make_name(
    TypeSystem& stringPool,
    const DataType& base,
    const DataType& type)
{
    ostringstream s;
    s << type.name() << " " << base.name() << "::*";
    return stringPool.get_string(s.str().c_str(), s.str().size());
}


////////////////////////////////////////////////////////////////
SharedString* PtrToMemberTypeImpl::read(
    DebugSymbol* sym,
    DebugSymbolEvents* events
    ) const
{
    if (RefPtr<TypeSystem> types = types_.ref_ptr())
    {
        // According to the Itanium C++ ABI section 2.3
        // (http://www.codesourcery.com/cxx-abi/abi.html):
        //  "It has the size and alignment attributes of a ptrdiff_t.
        //  A NULL pointer is represented as -1."
        RefPtr<DataType> sizeType =
            types->get_int_type(types->get_string("ptrdiff_t"),
                                types->word_size(), false);
        if (sizeType)
        {
            SharedString* result = sizeType->read(sym, events);
            if (result &&
                strtoul(result->c_str(), 0, 0) == numeric_limits<size_t>::max())
            {
                if (RefPtr<TypeSystem> types = types_.ref_ptr())
                {
                    result = types->get_string("NULL");
                }
            }
            return result;
        }
    }
    return NULL;
}


////////////////////////////////////////////////////////////////
bool PtrToMemberTypeImpl::is_equal(const DataType* other) const
{
    if (const PtrToMemberType* ptr = interface_cast<PtrToMemberType*>(other))
    {
        if (ptr->base_type() != base_type())
        {
            return false;
        }
        if (base_ && !base_->is_equal(ptr->base_type()))
        {
            return false;
        }
        if (ptr->pointed_type() != pointed_type())
        {
            return false;
        }
        if (type_ && !type_->is_equal(ptr->pointed_type()))
        {
            return false;
        }
        return true;
    }
    return false;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
