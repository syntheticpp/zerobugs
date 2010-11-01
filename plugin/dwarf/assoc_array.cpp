//
// $Id: assoc_array.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <memory>
#include <sstream>
#include "assoc_array.h"
#include "dharma/environ.h"
#include "zdk/export.h"
#include "zdk/shared_string_impl.h"
#include "zdk/thread_util.h"
#include "zdk/type_system.h"
#include "parse_array.h"
#include "typez/public/data_type_impl.h"
#include "typez/public/debug_symbol_impl.h"
#include "typez/public/debug_symbol_vector.h"
#include "typez/public/util.h"

using namespace std;

static const RefPtr<SharedString> KEY(shared_string(".key"));
static const RefPtr<SharedString> VAL(shared_string(".val"));
static const RefPtr<SharedString> PAIR(shared_string(""));

/**
 * Construct the name of the associative array from the name of
 * the key and value types.
 */
static RefPtr<SharedString>
make_name(TypeSystem& typeSys, DataType& keyType, DataType& elemType)
{
    ostringstream buf;

    buf << elemType.name()->c_str() << '[' << keyType.name()->c_str() << ']';
    return typeSys.get_string(buf.str().c_str(), buf.str().length());
}


/**
 * A synthesized type, to hold both the key and the value for
 * each element of a associative array.
 */
class ZDK_LOCAL ArrayElem : public DataTypeImpl<DataType>
{
    typedef DataTypeImpl<DataType> Base;

public:
    DECLARE_UUID("379f80f9-8b84-46f5-85d4-06a211caa7bb")
    BEGIN_INTERFACE_MAP(ArrayElem)
        INTERFACE_ENTRY(ArrayElem)
        INTERFACE_ENTRY(DataType)
    END_INTERFACE_MAP()

    ArrayElem() : Base(PAIR) { }

    bool is_fundamental() const { return true; }

    bool is_equal(const DataType* other) const
    {
        return interface_cast<ArrayElem*>(other);
    }

    size_t parse(const char*, Unknown2* = NULL) const  { return 0; }

    SharedString* read(DebugSymbol* sym, DebugSymbolEvents* events) const
    {
        assert(sym);

        if (DebugSymbol* child = sym->nth_child(0))
        {
            child->read(events); // read the key
        }

        if (DebugSymbol* child = sym->nth_child(1))
        {
            child->read(events);
            return child->value();
        }
        return null_ptr();
    }
};

static const RefPtr<DataType> nodeType(new ArrayElem);


AssociativeArray::AssociativeArray(TypeSystem& typeSys,
                                   DataType& keyType,
                                   DataType& valType)
    : BaseType(make_name(typeSys, keyType, valType), 0)
    , keyType_(&keyType)
    , valType_(&valType)
    , wordSize_(typeSys.word_size())
    , count_(0)
{
    typeSys.manage(this);
}


/*** D language ABI, from /usr/local/src/phobos/internal/aaA.d
struct aaA
{
    aaA *left;
    aaA *right;
    hash_t hash;
    // key
    // value
};

struct BB
{
    aaA*[] b;
    size_t nodes;       // total number of aaA nodes
};
// This is the type actually seen by the programmer, although
// it is completely opaque.
//
struct AA
{
    BB* a;
}
***/


/**
 * Corresponds to the D internal aaA struct above.
 */
struct AssocArrayNode
{
    addr_t left;
    addr_t right;
    size_t hash;

    // variable-length key
    // variable-length value
};



bool AssociativeArray::walk_tree(DebugSymbolImpl* array,
                                 DebugSymbolEvents* events,
                                 addr_t addr,
                                 Thread& thread,
                                 DataType& keyType,
                                 DataType& valType,
                                 DebugSymbolCollection& children) const
{
    if (children.enumerate() >= max_array_range())
    {
        // add an ellipsis to give a visual clue that we have truncated
        // the number of elements in the array
        RefPtr<DebugSymbolImpl> child =
            DebugSymbolImpl::create(thread, valType, "", shared_string("..."));

        children.add(child);
        return false;
    }
    DebugInfoReader* reader = array->reader();

    addr_t a = 0;
    if (addr)
    {
        thread_read(thread, addr, a);
    }

    while (a)
    {
        AssocArrayNode node;

        thread_read(thread, a, node.left);  advance_ptr(a);
        thread_read(thread, a, node.right); advance_ptr(a);
        thread_read(thread, a, node.hash);  advance_ptr(a);

    #if 0
        clog << "(" << (void*)node.left << ", "
             << (void*)node.right << ", " << node.hash << ")\n";
        clog << "key size=" << keyType.size() << endl;
    #endif

        RefPtr<DebugSymbolImpl> key = array->create(reader, thread, keyType, *KEY, a);
        key->read(events);
        a += keyType.size(); // bump address to beginning of value

        ostringstream name;

        // as a convenience, check for char[] and read and display as string
        if (DynamicArrayType* aType = interface_cast<DynamicArrayType*>(&keyType))
        {
            if (DataType* elemType = aType->elem_type())
            {
                if (elemType->name()->is_equal("char")
                 || elemType->name()->is_equal("unsigned char"))
                {
                    const size_t slen = aType->count(key.get()) + 1;
                    const addr_t saddr = aType->first_elem_addr(key.get());

                    char buf[50] = { 0 };

                    read_string(&thread, saddr, buf, slen);

                    name << array->name() << "[\"" << buf << "\"]";
                }
            }
        }

        if (name.str().empty())
        {
            name << array->name() << "[" << CHKPTR(key->value()) << "]";
        }
        RefPtr<SharedString> elemName = shared_string(name.str());

        RefPtr<DebugSymbolImpl> val = array->create(reader, thread, valType, *VAL, a);
        RefPtr<DebugSymbolImpl> elem = array->create(reader, thread, *nodeType, *elemName, 0);

        elem->add_child(key.get());
        elem->add_child(val.get());
        elem->read(events);

        children.add(elem);

        if (node.right)
        {
            if (!node.left)
            {
                a = node.right;
                continue;
            }
            if (!walk_tree(array, events, node.right, thread, keyType, valType, children))
            {
                break;
            }
        }
        a = node.left;
    }
    return true;
}


///// DataType interface /////

SharedString*
AssociativeArray::read(DebugSymbol* sym, DebugSymbolEvents* events) const
{
    addr_t addr = sym->addr();

    DebugSymbolImpl* array = interface_cast<DebugSymbolImpl*>(sym);
    if (!array)
    {
        return SharedStringImpl::create("#error: unknown array implementation");
    }
    ostringstream buf;

    // show the array's address:
    buf << '[' << hex << "0x" << addr << dec << ']';

    RefPtr<DataType> keyType = keyType_.ref_ptr();
    RefPtr<DataType> valType = valType_.ref_ptr();

    RefPtr<Thread> thread = sym->thread();

    if (thread && keyType && valType)
    {
        auto_ptr<DebugSymbolCollection> children(new DebugSymbolVector);

        // read the address of the BB struct
        thread_read(*thread, addr, addr);

        unsigned len = 0;
        if (addr)
        { // read length
            thread_read(*thread, addr, len);
            advance_ptr(addr);
        }
        addr_t pb = 0;
        if (addr)
        {   // read address where array begins
            thread_read(*thread, addr, pb);
            advance_ptr(addr);
        }
        size_t nodes = 0;
        if (addr)
        {
            thread_read(*thread, addr, nodes);
        }
        buf << " (" << dec << nodes << " node";
        if (nodes > 1)
        {
            buf << "s";
        }
        buf << ")";

        for (unsigned i = 0; i != len; ++i, advance_ptr(pb))
        {
            if (!walk_tree(array, events, pb, *thread, *keyType, *valType, *children))
            {
                break;
            }
        }
        array->set_children(children);
    }
    return shared_string(buf.str()).detach();
}


bool AssociativeArray::is_equal(const DataType* type) const
{
    if (AssociativeArray* other = interface_cast<AssociativeArray*>(type))
    {
        if (RefPtr<DataType> keyType = keyType_.ref_ptr())
        {
            if (!keyType->is_equal(other->key_type()))
            {
                return false;
            }
        }
        if (RefPtr<DataType> valType = valType_.ref_ptr())
        {
            if (!valType->is_equal(other->elem_type()))
            {
                return false;
            }
        }
      /*
        if (count() != other->count())
        {
            return false;
        }
       */
    }
    return true;
}


size_t AssociativeArray::parse(const char* s, Unknown2* unk) const
{
    return parse_array(s, unk, wordSize_);
}


///// ContainerType interface //////
DebugSymbol*
AssociativeArray::first(DebugSymbol* parent) const
{
    return 0; // todo
}


DebugSymbol*
AssociativeArray::next(DebugSymbol* parent, DebugSymbol* child) const
{
    return 0; // todo
}


/**
 * @return current number of elements in container object
 */
size_t AssociativeArray::count(DebugSymbol* container) const
{
    if (!count_ && container)
    {
        if (RefPtr<Thread> thread = container->thread())
        {
            addr_t addr = container->addr();

            // read the address of the BB struct
            thread_read(*thread, addr, addr);

            advance_ptr(addr);
            advance_ptr(addr);

            thread_read(*thread, addr, count_);
        }
    }
    return count_;
}


DataType* AssociativeArray::elem_type() const
{
    return valType_.ref_ptr().get();
}

///// AssociativeContainerType interface //////
DataType* AssociativeArray::key_type() const
{
    return keyType_.ref_ptr().get();
}


size_t
AssociativeArray::enum_by_key(DebugSymbol* parent,
                              DebugSymbol* key,
                              DebugSymbolCallback*) const
{
    return 0; // todo
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
