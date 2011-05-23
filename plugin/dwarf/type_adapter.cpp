//
// $Id: type_adapter.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// Adapt type objects from the Dwarf namespace to the types
// defined in the typez library (see types.h). The dwarfz
// library was designed as a standalone, reusable piece; also
// it was developed somewhat independently from the typez and
// stabz libraries. A future version may use the typez types
// directly (as stabz does) and not need the adaptation layer.
//
#include "zdk/config.h"
#include <algorithm>
#include <functional>
#include <map>
#include <stdexcept>
#include "generic/temporary.h"
#include "zdk/check_ptr.h"
#include "zdk/platform.h"                   // for byte_size
#include "zdk/thread_util.h"
#include "zdk/types.h"
#include "zdk/type_system_util.h"
#include "zdk/zobject_scope.h"
#include "dharma/symbol_util.h"
#include "dwarfz/public/array_type.h"
#include "dwarfz/public/base_type.h"
#include "dwarfz/public/class_type.h"
#include "dwarfz/public/compile_unit.h"
#include "dwarfz/public/const_value.h"
#include "dwarfz/public/debug.h"
#include "dwarfz/public/delegate.h"
#include "dwarfz/public/enum_type.h"
#include "dwarfz/public/inheritance.h"
#include "dwarfz/public/const_type.h"
#include "dwarfz/public/location.h"
#include "dwarfz/public/member.h"
#include "dwarfz/public/pointer_type.h"
#include "dwarfz/public/ptr_to_member_type.h"
#include "dwarfz/public/subroutine_type.h"
#include "dwarfz/public/template_type.h"
#include "dwarfz/public/typedef.h"
#include "dwarfz/public/volatile_type.h"
#include "dwarfz/public/impl.h"
#include "typez/public/debug_symbol_impl.h"
#include "typez/public/enum_type.h"
#include "typez/public/types.h"
#include "typez/public/template_param.h"
#include "assoc_array.h"
#include "class_type_proxy.h"
#include "const_value.h"
#include "dbgout.h"
#include "dynamic_array.h"
#include "reader.h"
#include "type_adapter.h"


// #define DEBUG_ADAPTER 1

using namespace std;
using namespace boost;
using namespace Platform;


////////////////////////////////////////////////////////////////
// return a pointer to function returning void
static RefPtr<DataType> get_vtable_ptr_type(TypeSystem* types)
{
    RefPtr<DataType> voidType = CHKPTR(types)->get_void_type();
    RefPtr<DataType> funType =
        types->get_fun_type(voidType.get(), 0, 0, false);
    RefPtr<DataType> ptrType(types->get_pointer_type(funType.get()));

    return ptrType;
}


////////////////////////////////////////////////////////////////
static Dwarf_Addr
evaluate(const Dwarf::Location& loc, addr_t base, Thread& thread)
{
    dbgout(0) << " ****** " << (void*)base << " *****" << endl;
    addr_t moduleBase = 0;
    addr_t pc = thread.program_count();

    if (StackTrace* trace = thread.stack_trace())
    {
        if (trace->size())
        {
            Frame* frame = CHKPTR(trace->selection());

            pc = frame->program_count();
            if (Symbol* sym = frame->function())
            {
                ZObjectScope scope;
                moduleBase = CHKPTR(sym->table(&scope))->adjustment();
            }
        }
    }
    return loc.eval(base, moduleBase, 0, pc);
}


////////////////////////////////////////////////////////////////
static TypeSystem* type_system(const RefPtr<Thread>& thread)
{
    TypeSystem* types = interface_cast<TypeSystem*>(thread.get());

    if (!types)
    {
        throw logic_error("dwarf: null typesystem");
    }
    return types;
}


////////////////////////////////////////////////////////////////
namespace
{
    using namespace Dwarf;

    /**
     * Function object which aggregates a base class or
     * member data into the owner class.
     * Called from the Dwarf::KlassType visitation.
     */
    CLASS Aggregator
    {
    public:
        /**
         * Ctor takes a frame-base address, a ref-counted
         * pointer to the class type into which a new
         * part is to be aggregated, and a type map; the
         * latter is for optimization purposes: caches
         * types that are already adapted.
         */
        Aggregator
        (
            Reader*                 reader,
            const RefPtr<Thread>&   thread,
            addr_t                  frameBase,
            RefPtr<ClassTypeImpl>   klass,
            Dwarf::TypeMap&         typeMap
        )
          : reader_(reader)
          , thread_(thread)
          , baseAddr_(frameBase)
          , klass_(klass)
          , typeMap_(typeMap)
          , typeSys_(interface_cast<TypeSystem*>(thread.get()))
        {
            assert(klass_.get());
            assert(get_addr_operations());
        }

        /// aggregate a non-static member datum
        void operator()(const Dwarf::DataMember& part) const;

        /// aggregate a base class
        void operator()(const Dwarf::Inheritance& part) const;

        /// aggregate a static member
        void operator()(const boost::shared_ptr<Dwarf::StaticMember>&);

        void operator()(const Dwarf::TemplateType<Type>& templ) const;

    private:
        template<typename T>
        void add_static_member(const T& part) const
        {
            boost::shared_ptr<Type> type = part.type();
            if (!type)
            {
                clog << "unknown type: " << part.name() << endl;
                return;
            }
            TypeAdapter adapter(reader_, thread_, baseAddr_, typeMap_);
            RefPtr<DataType> adaptedType = adapter.apply(type);
            if (!adaptedType)
            {
                throw_null_type("static member", part);
            }
            RefPtr<SharedString> name(shared_string(part.name()));
            RefPtr<SharedString> linkName = part.linkage_name();

            RefPtr<DebugSymbol> v = const_value(*thread_, part, *adaptedType, name.get());
            clog << "const_value: " << v.get() << endl;
            klass_->add_member(name, linkName, 0, 0, *adaptedType, true, v.get());
        }

        Reader*                 reader_;
        RefPtr<Thread>          thread_;
        addr_t                  baseAddr_;
        RefPtr<ClassTypeImpl>   klass_;
        Dwarf::TypeMap&         typeMap_;
        TypeSystem*             typeSys_;
    };


    template<typename T>
    void throw_null_type(const char* msg, const T& die)
    {
        assert(msg);
        assert(die.type());
        assert(die.name());

        string err = "null type for ";
        err += msg;
        err += ": ";
        err += die.type()->name();
        err += " ";
        err += die.name();

        throw logic_error(err);
    }
} // namespace


////////////////////////////////////////////////////////////////
static size_t
adjust_bit_offs(const DataMember& part,
                size_t bitOffs,
                size_t structBitSize,
                size_t bitSize,
                int debugLevel_)
{
#if (__BYTE_ORDER == __LITTLE_ENDIAN)

    size_t alignTo = part.byte_size() * byte_size;

    if (alignTo == 0)
    {
        alignTo = structBitSize;
    }

    bitOffs = alignTo - bitOffs - bitSize;

#endif

    return bitOffs;
}


////////////////////////////////////////////////////////////////
void Aggregator::operator()(const Dwarf::DataMember& part) const
{
    RefPtr<SharedString> name;
    bool isVTablePtr = false;

    if (part.name() == 0 || *part.name() == 0)
    {
        name = unnamed_type();
    }
    else if (strncmp(part.name(), "_vptr", 5) == 0)
    {
        name = shared_string(".vptr");
        isVTablePtr = true;
    }
    else
    {
        // name = new SharedStringImpl(part.name());
        // use string pool to minimize memory usage
        name = typeSys_->get_string(part.name());
    }
    Dwarf_Unsigned bitSize = part.bit_size();
    Dwarf_Off bitOffs = part.bit_offset();

    if (bitOffs || (bitSize < part.byte_size() * byte_size))
    {
        dbgout(0) << "bitOffs=" << bitOffs << endl;
        bitOffs = adjust_bit_offs(part,
                                  bitOffs,
                                  klass_->bit_size(),
                                  bitSize,
                                  debugLevel_);

        dbgout(0) << "adjusted bitOffs=" << bitOffs << endl;
    }

    RefPtr<DataType> adaptedType;

    if (boost::shared_ptr<Dwarf::Type> type = part.type())
    {
        Dwarf_Addr addr = baseAddr_;
        TypeAdapter adapter(reader_, thread_, addr, typeMap_);

        if (type->is_pointer_or_ref())
        {
            // do not follow pointer types of member data
            adapter.set_depth(ADAPT_VERY_SHALLOW);
        }

        // get the offset relative to the beginning of
        // the owner class or struct
        if (boost::shared_ptr<Location> loc = part.loc())
        {
            addr = evaluate(*loc, baseAddr_, *CHKPTR(thread_));
            bitOffs += (addr - baseAddr_) * byte_size;

            dbgout(0) << "struct-based bit offs=" << bitOffs << endl;

            adapter.set_base_addr(addr);
        }
        else if (!klass_->is_union())
        {
            cerr << part.name() << ": Location not found." << endl;
            add_static_member(part);
        }

        adaptedType = adapter.apply(type);

        if (bitSize == 0)
        {
            bitSize = type->byte_size() * byte_size;
        }
    }
    else if (isVTablePtr)
    {
        assert(thread_.get());
        TypeSystem* types = type_system(thread_);
        adaptedType = get_vtable_ptr_type(types);
    }
    else
    {
        throw logic_error(string("null type: ") + part.name());
    }
    if (!adaptedType)
    {
        throw_null_type("member", part);
    }
    if (bitSize != adaptedType->bit_size())
    {
        if (bitSize == 0)
        {
            bitSize = adaptedType->bit_size();
        }
       /*
        else
        {
            clog << "bitSize=" << bitSize << " adapted bitsize=";
            clog << adaptedType->bit_size() << endl;
            clog << "adapted type="  << adaptedType->name() << endl;

            assert(interface_cast<IntType*>(adaptedType.get())
                || interface_cast< ::EnumType* >(adaptedType.get()));
        }
        */
    }

    klass_->add_member(name, NULL, bitOffs, bitSize, *adaptedType);
}


////////////////////////////////////////////////////////////////
void Aggregator::operator()(const Inheritance& part) const
{
    if (!part.type())
    {
        throw runtime_error(string("null type: ") + part.name());
    }

    Dwarf_Off bitOffs = 0;
    Dwarf_Addr addr = baseAddr_;

    // get the offset relative to the beginning of
    // the final (derived) class or struct
    if (boost::shared_ptr<Location> loc = part.loc())
    {
        addr = evaluate(*loc, baseAddr_, *CHKPTR(thread_));
        bitOffs = (addr - baseAddr_) * byte_size;

        dbgout(0) << "loc->eval(" << hex << baseAddr_ << ")="
                  << addr << dec << " offset=" << addr - baseAddr_
                  << endl;
    }
    else
    {
#ifdef DEBUG
        cerr << part.name() << ": location not found." << endl;
#endif
        return;
    }

    TypeAdapter adapter(reader_, thread_, addr, typeMap_);
    RefPtr<DataType> adaptedType = adapter.apply(part.type());

    if (!adaptedType)
    {
        throw_null_type("base class", part);
    }

    ::Access accessType = ACCESS_PRIVATE;
    switch (part.access())
    {
    case a_public: accessType = ACCESS_PUBLIC;
        break;
    case a_protected: accessType = ACCESS_PROTECTED;
        break;
    case a_private: // do nothing
        break;
    }

    const bool virt = part.is_virtual();
    klass_->add_base(*adaptedType, bitOffs, accessType, virt);
}


////////////////////////////////////////////////////////////////
void
Aggregator::operator()(const boost::shared_ptr<StaticMember>& part)
{
    assert(part);

    if (!part)
    {
        return;
    }
    assert(part->type());
    assert(part->name());

    add_static_member(*part);
}


////////////////////////////////////////////////////////////////
void
Aggregator::operator()(const TemplateType<Type>& templ) const
{
    if (typeSys_)
    {
        RefPtr<SharedString> name = typeSys_->get_string(templ.name());

        TypeAdapter adapter(reader_, thread_, baseAddr_, typeMap_);
        if (RefPtr<DataType> type = adapter.apply(templ.type()))
        {
            RefPtr<TemplateTypeParam> param =
                new TemplateTypeParamImpl(name.get(), type.get());

            if (klass_)
            {
                klass_->add_template_type_param(*param);
            }
        }
        else
        {
            throw_null_type("template type param", templ);
        }
    }
}


////////////////////////////////////////////////////////////////
TypeAdapter::TypeAdapter
(
    Reader*                 reader,
    const RefPtr<Thread>&   thread,
    addr_t                  addr,
    TypeMap&                typeMap
)
  : reader_(reader)
  , thread_(thread)
  , baseAddr_(addr)
  , modAdjust_(0)
  , pc_(0)
  , typeMap_(typeMap)
  , depth_(reader == NULL ? ADAPT_FULL : ADAPT_SHALLOW)
  , context_(NULL)
{
    if (Symbol* fun = thread_current_function(thread.get()))
    {
        ZObjectScope scope;
        if (SymbolTable* table = fun->table(&scope))
        {
            modAdjust_ = table->adjustment();
            modName_ = table->filename();
        }
        pc_ = fun->addr();
    }
}


/**
 * Adapt element types and key types
 */
template<typename T, typename U>
static RefPtr<DataType>
adapt_elem(TypeAdapter& a, const T& array, U subType)
{
    if (!array.elem_type())
    {
        throw logic_error(string("null elem type: ") + array.name());
    }

    RefPtr<DataType> type = a.apply(subType);

    if (!type)
    {
        // not expected to fail, there must be a mistake
        // in the logic if this happens
        throw logic_error(
            string("could not adapt array element type in: ")
            + array.name());
    }
    return type;
}


////////////////////////////////////////////////////////////////
void TypeAdapter::visit(const Dwarf::ArrayType& array)
{
    assert(!typeMap_.find(array));
    TypeAdapter adapter(reader_, thread_, baseAddr_, typeMap_);

    RefPtr<DataType> elemType = adapt_elem(adapter, array, array.elem_type());

    // get array dimensions
    typedef vector<boost::shared_ptr<Dimension> > vect_type;
    vect_type v;

    List<Dimension> dim = array.dimensions();
    for (List<Dimension>::const_iterator i = dim.begin(); i != dim.end(); ++i)
    {
        v.push_back(i);
    }
    RefPtr<DataType> type = elemType;

    // iterate thru dimension in reverse order
    // and make one-dimensional arrays of arrays
    for (vect_type::reverse_iterator j = v.rbegin(); j != v.rend(); ++j)
    {
        assert(type);
        type.reset(
            type_system().get_array_type((*j)->lower_bound(),
                                         (*j)->upper_bound(),
                                         type.get()));
    }
    type_ = type;
}


/**
 * Helper for adapting C/C++ structs, classes, and unions
 */
static RefPtr<ClassType>
get_class_type(TypeSystem& types, const char* name, size_t bits, bool isUnion)
{
    RefPtr<ClassType> klass;
    if (!name || !name[0])
    {
        klass = types.get_unnamed_class_type(bits, isUnion);
    }
    else
    {
        klass = types.get_class_type(name, bits, isUnion);
    }
    return klass;
}


////////////////////////////////////////////////////////////////
void TypeAdapter::visit(const Dwarf::Delegate& delegate)
{
    TypeAdapter adapter(reader_, thread_, baseAddr_, typeMap_);
    RefPtr<DataType> thisType = adapter.apply(delegate.this_type());
    if (thisType.is_null())
    {
        throw runtime_error("could not adapt ptr type in delegate");
    }
    RefPtr<DataType> funcType = adapter.apply(delegate.function_type());
    if (funcType)
    {
        funcType = type_system().get_pointer_type(funcType.get());
    }
    if (funcType.is_null())
    {
        throw runtime_error("could not adapt function type in delegate");
    }
    const size_t ptrSize = (thread_->is_32_bit() ? 4 : 8);
    RefPtr<ClassType> k = get_class_type(type_system(),
                                         delegate.name(),
                                         2 * 8 * ptrSize,
                                         false);
    type_ = k;

    RefPtr<ClassTypeImpl> impl = interface_cast<ClassTypeImpl>(k);
    impl->add_member(type_system().get_string("context"),
                     NULL, 0, ptrSize * 8, *thisType);
    impl->add_member(type_system().get_string("function"),
                     NULL, ptrSize * 8, ptrSize * 8, *funcType);
}


////////////////////////////////////////////////////////////////
void TypeAdapter::visit(const Dwarf::DynArrayType& array)
{
    TypeAdapter adapter(reader_, thread_, baseAddr_, typeMap_);
    RefPtr<DataType> elemType = adapt_elem(adapter, array, array.elem_type());

    type_ = new DynamicArray(type_system(), *elemType);
}


////////////////////////////////////////////////////////////////
void TypeAdapter::visit(const Dwarf::AssocArrayType& array)
{
    TypeAdapter adapter(reader_, thread_, baseAddr_, typeMap_);
    RefPtr<DataType> keyType = adapt_elem(adapter, array, array.key_type());
    RefPtr<DataType> elemType = adapt_elem(adapter, array, array.elem_type());

    type_ = new AssociativeArray(type_system(), *keyType, *elemType);
}


////////////////////////////////////////////////////////////////
void TypeAdapter::visit(const Dwarf::BaseType& type)
{
    assert(type.name());
    assert(*type.name());

    RefPtr<SharedString> name(shared_string(type.name()));
    const size_t bitSize = type.byte_size() * byte_size;

    const Dwarf::BaseType::Encoding enc = type.encoding();

    switch (enc)
    {
    case BaseType::e_address:
        assert(false);
        break;

    case BaseType::e_boolean:
        if (type.bit_size() % Platform::byte_size)
        {
            type_ = type_system().get_int_type(name.get(),
                                               type.bit_size(),
                                               false); // unsigned
            break;
        }
        if (type.byte_size() == 1)
        {
            type_ = type_system().get_bool_type(byte_size);
            break;
        }
        assert(false);
        break;

    case BaseType::e_float:
        type_ = type_system().get_float_type(name.get(), type.byte_size());
        break;

    case BaseType::e_signed:
        if (bitSize == 0)
        {
            assert(name->is_equal("void"));
            type_ = type_system().get_void_type();
            break;
        }
        // fallthru
    case BaseType::e_signed_char:
        type_ = type_system().get_int_type(name.get(), bitSize, true);
        break;

    case BaseType::e_unsigned:
    case BaseType::e_unsigned_char:
        type_ = type_system().get_int_type(name.get(), bitSize, false);
        break;

    case BaseType::e_complex_float:
        type_ = type_system().get_complex_float(name.get(), type.byte_size());
        break;

    case BaseType::e_complex_int:
        type_ = type_system().get_complex_int(name.get(), type.byte_size());
        break;

    default:
        cerr << "dwarf: unhandled basetype: " << (void*)enc << endl;
        break;
    }
}


////////////////////////////////////////////////////////////////
//
#if 0 // DEBUG
/**
 * Some classes take a long time to visit. Log some info to help
 * better understand the problem.
 */
CLASS ClassInfoLogger : boost::noncopyable
{
    static long level_;
    string name_, enterMsg_, leaveMsg_;

    string time_stamp() const
    {
        time_t now = time(NULL);
        string stamp(ctime(&now));
        assert(!stamp.empty());
        stamp.erase(stamp.size() - 1);
        return stamp + ": ";
    }

public:
    ClassInfoLogger(const char* name, const char* enter, const char* leave = NULL)
        : name_(name ? name : "???")
        , enterMsg_(enter)
        , leaveMsg_(leave ? leave : ("done " + enterMsg_).c_str())
    {
        clog << "\n" << time_stamp();
        clog << string(2 * level_++, ' ') << enterMsg_ << ": " << name_ << endl;
    }

    ~ClassInfoLogger()
    {
        clog << time_stamp();
        clog << string(2 * --level_, ' ') << leaveMsg_ << ": " << name_ << endl;
    }
};

long ClassInfoLogger::level_ = 0;

#define LOG_SCOPE(n,e,...) ClassInfoLogger log(n, e, ##__VA_ARGS__)

#else
#define LOG_SCOPE(n,e,...)

#endif


////////////////////////////////////////////////////////////////
static void
aggregate_bases(Aggregator& agg, const KlassType& klass, Depth depth)
{
    const KlassType::BaseList& bases = klass.bases();
    if (depth == ADAPT_FULL || depth == ADAPT_SHALLOW) try
    {
        LOG_SCOPE(klass.name(), "Visiting bases of");
        for (KlassType::BaseList::const_iterator i = bases.begin();
             i != bases.end();
             ++i)
        {
            assert(*i);
            agg(**i);
        }
    }
    catch (const std::exception& e)
    {
        if (depth == ADAPT_FULL) throw;
        clog << __func__ << ": " << klass.name() << ": " << e.what() << endl;
    }
}


////////////////////////////////////////////////////////////////
static void
aggregate_member_data(Aggregator& agg, const KlassType& klass)
{
    // aggregate "regular" data members
    {   LOG_SCOPE(klass.name(), "visiting members");

        List<DataMember> members = klass.members();
#if HAVE_LAMBDA_SUPPORT
        for_each(members.begin(), members.end(), 
            [&agg](const DataMember& m) {
                //clog << "member: " << m.name() << endl;
                agg(m);
            });
#else
        for_each<List<DataMember>::iterator, Aggregator&>(
            members.begin(), members.end(), agg);
#endif
    }

    // aggregate static members
    {   LOG_SCOPE(klass.name(), "visiting static members");
        const KlassType::StaticMemData& staticMembers =
            klass.static_members();

        clog << staticMembers.size() << " static member(s) in "
             << klass.name() << endl;

#if HAVE_LAMBDA_SUPPORT
        for_each(staticMembers.begin(), staticMembers.end(), 
            [&agg](const boost::shared_ptr<StaticMember>& m) {
                agg(m);
            });
#else
        typedef KlassType::StaticMemData::const_iterator Iter;
        for_each<Iter, Aggregator&>(
            staticMembers.begin(), staticMembers.end(), agg);
#endif
    }
}


////////////////////////////////////////////////////////////////
void TypeAdapter::visit(const Dwarf::KlassType& type)
{
    assert(!typeMap_.find(type));

    const char* const name = typeName_ ? typeName_->c_str() : type.name();

    RefPtr<ClassType> klass =
        get_class_type(type_system(), name, type.bit_size(), false);

    RefPtr<ClassTypeProxy> proxy;
    if (depth_ != ADAPT_FULL)
    {
        assert(reader_); // must not be NULL if shallow, so
                         // that we can later resolve the type
        if (reader_)
        {
            //
            // See comments in class_type_proxy.h
            //
            proxy = new ClassTypeProxy(*reader_, thread_, type, *klass);
            type_ = proxy;
            type_system().manage(proxy.get());
        }
    }
    else
    {
        type_ = klass;
    }
    typeMap_.add(type, type_, false);

    RefPtr<ClassTypeImpl> impl = interface_cast<ClassTypeImpl>(klass);

    Aggregator aggregate(reader_, thread_, baseAddr_, impl, typeMap_);

    aggregate_bases(aggregate, type, depth_);

    if (depth_ == ADAPT_FULL)
    {
        aggregate_member_data(aggregate, type);

        ////////////////////////////////////////////////////////
        {   LOG_SCOPE(type.name(), "visiting template type params");
            // note as of now (April 2007) GCC does not seem to generate
            // DW_AT_template_param info, but Intel Compiler does
            typedef List<TemplateType<Type> > TemplateTypeParams;

            TemplateTypeParams templateTypes = type.template_types();
#if HAVE_LAMBDA_SUPPORT
            for_each(templateTypes.begin(), templateTypes.end(),
                     [&aggregate](const TemplateType<Type>& t) {
                        aggregate(t); 
                     });
#else
            for_each<TemplateTypeParams::iterator, Aggregator&>(
                    templateTypes.begin(),
                    templateTypes.end(),
                    aggregate);
#endif
        }

        ////////////////////////////////////////////////////////
        // NOTE: if the class is nested in a namespace or
        // another class, then adding the methods may have the
        // side-effect of changing the class' name to its
        // fully-qualified name
        if (klass)
        {
            LOG_SCOPE(type.name(), "adding methods");
            add_methods(type, interface_cast<ClassTypeImpl&>(*klass));
        }
        type_ = klass; // in case add_methods modified it

        // cache it by the fully-qualified name
        typeMap_.add(type, klass);
    }
}


////////////////////////////////////////////////////////////////
template<typename T>
static Dwarf_Addr get_method_addr(const KlassType& type,
                                  ClassTypeImpl& klass,
                                  T method)
{
    Dwarf_Addr addr = method->low_pc();
    SharedString* unqualifiedName = klass.unqualified_name();

    string ctorName;
    FunList ctors;

    // hack: GCC 3.2.2 generates empty linkage names for ctors
    if (addr == 0
        && method->linkage_name().is_null()
        && CHKPTR(unqualifiedName)->is_equal(method->name()))
    {
        if (ctors.empty())
        {
            if (ctorName.empty())
            {
                ctorName = klass.name()->c_str();
                ctorName += "::";
                ctorName += CHKPTR(unqualifiedName)->c_str();

                dbgout(0) << __func__ << ": " << ctorName << endl;
            }
            ctors = type.owner().lookup_global_funcs(ctorName.c_str());
        }

        // find ctor that matches the method's prototype
        FunList::const_iterator fi = ctors.begin();
        for (; fi != ctors.end(); ++fi)
        {
            if ((*fi)->compare_prototype(*method))
            {
                addr = (*fi)->low_pc();
                break;
            }
        }
    }
    return addr;
}


////////////////////////////////////////////////////////////////
void TypeAdapter::add_methods
(
    const Dwarf::KlassType& type, // type to adapt, from .debug
    ClassTypeImpl& klass
)
{
    // enumerate methods
    const MethodList& methods = type.methods();

    MethodList::const_iterator m = methods.begin();
    const MethodList::const_iterator end = methods.end();

    boost::shared_ptr<CompileUnit> unit = type.owner().lookup_unit(pc_);
    addr_t unitBase = unit ? unit->base_pc() : 0;

    for (; m != end; ++m)
    {
        const MethodList::value_type& method = *m;
        assert(method->name());

        Dwarf_Addr addr = get_method_addr(type, klass, method);

        // adapt the function type // TODO: pull in separate func?
        TypeAdapter adapt(reader_, thread_, baseAddr_, typeMap_);
        adapt.context_ = this; // ... in this context

        adapt.get_type(*method, &type);

        ::Access access = ACCESS_PUBLIC;
        switch (method->access())
        {
        case a_private: access = ACCESS_PRIVATE; break;
        case a_protected: access = ACCESS_PROTECTED; break;
        case a_public: break;
        }

        // todo: cv-qualifiers?
        // also: can do without the interface_cast?
        FunType& funType = interface_cast<FunType&>(*adapt.type());
        RefPtr<SharedString> methodName =
            type_system().get_string(method->name());

        RefPtr<MethodImpl> m = klass.add_method(methodName,
                                                method->linkage_name(),
                                                &funType,
                                                access,
                                                method->is_virtual());
        m->set_start_addr(addr);
        off_t voffs = method->vtable_offset(baseAddr_, modAdjust_, unitBase, pc_);
        m->set_vtable_offset(voffs);
    }
    //dbgout(1) << __func__ << ": done" << endl;
}


////////////////////////////////////////////////////////////////
void TypeAdapter::visit(const Dwarf::ConstType& type)
{
    assert(!typeMap_.find(type));

    boost::shared_ptr<Type> constType = type.type();

    if (constType)
    {
        apply(constType);
        if (!type_)
        {
            throw logic_error("could not adapt const type");
        }
    }
    else
    {
        type_ = type_system().get_void_type();
    }
    type_ = type_system().get_qualified_type(type_.get(), QUALIFIER_CONST);
}


////////////////////////////////////////////////////////////////
void TypeAdapter::visit(const Dwarf::VolatileType& type)
{
    assert(!typeMap_.find(type));
    boost::shared_ptr<Type> volatileType(type.type());

    if (volatileType)
    {
        apply(volatileType);
        if (!type_)
        {
            throw logic_error("could not adapt volatile type");
        }
    }
    else
    {
        type_ = type_system().get_void_type();
    }
    type_ = type_system().get_qualified_type(type_.get(), QUALIFIER_VOLATILE);
}


////////////////////////////////////////////////////////////////
void TypeAdapter::visit(const Dwarf::EnumType& enumType)
{
    type_ = typeMap_.find(enumType);
    if (type_.get())
    {
        return;
    }
    std::map<int, RefPtr<SharedString> > names;

    Dwarf::List<Dwarf::EnumType::Enumerator>::const_iterator
        i = enumType.enums().begin(),
        end = enumType.enums().end();

    for (; i != end; ++i)
    {
        names[i->value()] = shared_string(i->name());
    }
    RefPtr<SharedString> ss(shared_string(enumType.name()));
    type_.reset(type_system().manage(new EnumTypeImpl<int>(ss.get(), names)));
}


////////////////////////////////////////////////////////////////
RefPtr<DataType>
TypeAdapter::get_type(const Dwarf::Function& fun,
                      const Dwarf::KlassType* klass)
{
    type_ = typeMap_.find(fun);

    if (!type_)
    {
        get_fun_type(fun, fun.ret_type(), fun.params(), klass);
        assert(type_);

        typeMap_.add(fun, type_);
        if (reader_)
        {
            reader_->add_fun_to_linkage_map(type_system(), fun);
        }
    }

    assert(type_); // post-condition
    return type_;
}


////////////////////////////////////////////////////////////////
RefPtr<DataType>
TypeAdapter::get_fun_type (
    const Dwarf::Die& die,
    const boost::shared_ptr<Dwarf::Type>& retType,
    const List<Parameter>& params,
    const Dwarf::KlassType* klass)
{
    Dwarf::Function::ParamList args;

    List<Parameter>::const_iterator i = params.begin();
    List<Parameter>::const_iterator end = params.end();
    for (; i != end; ++i)
    {
        args.push_back(i);
    }
    return get_fun_type(die, retType, args, klass);
}


////////////////////////////////////////////////////////////////
RefPtr<DataType>
TypeAdapter::get_fun_type (
    const Dwarf::Die& die,
    const boost::shared_ptr<Dwarf::Type>& retType,
    const Function::ParamList& params,
    const Dwarf::KlassType* // klass
    )
{
    assert(die.get_tag() == DW_TAG_subprogram
        || die.get_tag() == DW_TAG_subroutine_type);

    vector<RefPtr<DataType> > paramTypes; // adapted params

    Function::ParamList::const_iterator i = params.begin();
    const Function::ParamList::const_iterator end = params.end();

    for (size_t n = 0; i != end; ++i, ++n)
    {
        // do not follow pointer types when resolving param types
        Temporary<Depth> setInScope(depth_, ADAPT_SHALLOW);

        if ((*i)->type())
        {
            RefPtr<DataType> adaptedArgType;

            dbgout(1) << "adapting " << (*i)->type()->name() << endl;
            adaptedArgType = apply((*i)->type());

            if (!adaptedArgType)
            {
                throw_null_type("parameter", **i);
            }
            paramTypes.push_back(adaptedArgType);
        }
        else
        {
            ostringstream err;

            err << "null type info for parameter: " << n;
            throw runtime_error(err.str());
        }
    }

    type_.reset();
    if (retType)
    {
        apply(retType);
    }

    const bool hasVarArgs =
        Utils::has_child(die, DW_TAG_unspecified_parameters);

    RefPtr<FunType> funType =
        get_function_type(  type_system(),
                            type_,
                            paramTypes,
                            hasVarArgs);
    dbgout(1) << funType->name() << endl;
    dbgout(1) << paramTypes.size() << " param(s)" << endl;

    type_ = funType;
    return type_;
}


////////////////////////////////////////////////////////////////
void TypeAdapter::visit(const Dwarf::PointerType& type)
{
    TypeAdapter adapter(reader_, thread_, baseAddr_, typeMap_);
    adapter.set_depth(depth_);

    boost::shared_ptr<Type> pointedType(type.type());

    if (pointedType) // pointedType may be NULL for type void
    {
        adapter.apply(pointedType);
#ifdef DEBUG_ADAPTER
        if (adapter.type())
        {
            clog << "pointed=" << pointedType->cu_offset() << endl;
            clog << "Pointed=" << adapter.type()->name() << endl;
        }
#endif
    }

    if (DataType* adaptedType = adapter.type().get())
    {
        if (type.get_tag() == DW_TAG_reference_type)
        {
            type_ = type_system().get_reference_type(adaptedType);
        }
        else
        {
            assert (type.get_tag() == DW_TAG_pointer_type);
            type_ = type_system().get_pointer_type(adaptedType);
        }
    }
    else
    {
        //
        // assume pointer to void
        //
        RefPtr<DataType> vt = type_system().get_void_type();
        if (type.get_tag() == DW_TAG_pointer_type)
        {
            type_ = type_system().get_pointer_type(vt.get());
        }
        else
        {
            type_ = type_system().get_reference_type(vt.get());
        }
    }
}


////////////////////////////////////////////////////////////////
void TypeAdapter::visit(const Dwarf::PtrToMemberType& type)
{
    TypeAdapter adapter(reader_, thread_, baseAddr_, typeMap_);
    adapter.set_depth(depth_);

    if (boost::shared_ptr<Type> pointedType = type.type())
    {
        adapter.apply(pointedType);
    }
    RefPtr<DataType> memberType = adapter.type();
    if (!memberType)
    {
        memberType = type_system().get_void_type();
    }
    if (boost::shared_ptr<Type> baseType = type.containing_type())
    {
        if (adapter.apply(baseType))
        {
            type_ = type_system().get_ptr_to_member_type(adapter.type().get(),
                                                         memberType.get());
        }
    }
}


////////////////////////////////////////////////////////////////
void TypeAdapter::visit(const Dwarf::SubroutineType& sub)
{
    get_fun_type(sub, sub.ret_type(), sub.params());
}


////////////////////////////////////////////////////////////////
void TypeAdapter::visit(const Dwarf::UnionType& type)
{
    type_ = typeMap_.find(type);

    if (!type_)
    {
        RefPtr<ClassType> klass  =
            get_class_type(type_system(), type.name(), type.bit_size(), true);

        assert(klass->is_union());
        typeMap_.add(type, klass);

        RefPtr<ClassTypeImpl> impl(interface_cast<ClassTypeImpl>(klass));
        Aggregator aggregate(reader_, thread_, baseAddr_, impl, typeMap_);

        List<DataMember> members = type.members();
#if HAVE_LAMBDA_SUPPORT
        for_each(members.begin(), members.end(), [&aggregate](const DataMember& m) {
            aggregate(m);
        });
#else
        for_each<List<DataMember>::iterator, Aggregator&>(
            members.begin(), members.end(), aggregate);
#endif
        type_ = klass;
    }
}


////////////////////////////////////////////////////////////////
void TypeAdapter::visit(const Dwarf::Typedef& typeDef)
{
    if (boost::shared_ptr<Type> type = typeDef.type())
    {
        assert(type->name());
        assert(type->name()[0]);

        apply(type);
    }
}


namespace
{
    /**
     * Lookup types using other debug-info readers than DWARF
     */
    class ZDK_LOCAL TypeHelper : public EnumCallback<DebuggerPlugin*>
    {
    public:
        virtual ~TypeHelper() throw() { }

        TypeHelper(const DebugInfoReader* r, Thread* t, const char* name)
            : reader_(r), thread_(t), name_(name)
        {
            assert(name_);
        }

        void notify(DebuggerPlugin* plugin)
        {
            if (!type_)
            {
                DebugInfoReader* reader =
                    interface_cast<DebugInfoReader*>(plugin);

                if (reader && reader_ != reader)
                {
                    type_ = reader->lookup_type(thread_, name_);
                }
            }
        }

        RefPtr<DataType> type() const { return type_; }

    private:
        const DebugInfoReader* reader_;
        Thread* thread_;
        const char* name_;
        RefPtr<DataType> type_;
    };
} // namespace


////////////////////////////////////////////////////////////////
static bool resolve_decl(boost::shared_ptr<Type>& type)
{
    if (type->is_declaration())
    {
        if (boost::shared_ptr<Type> spec = type->owner().lookup_type_by_decl(*type))
        {
            type = spec;
            return true;
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////
RefPtr<DataType> TypeAdapter::apply(boost::shared_ptr<Type> type)
{
    assert(type); // pre-condition
    type_.reset();

    if (type)
    {
        bool decl = false;
        if (depth_ == ADAPT_FULL)
        {
            decl = !resolve_decl(type);
        }
        else if (type->is_declaration())
        {
            decl = true;
            // clog << "decl: " << type->name() << endl;
        }
        type_ = typeMap_.find(*type);   // already adapted?

        if (!type_)
        {
            const char* name = type->name();
            CHKPTR(name);
            dbgout(0) << "type=" << name << endl;

            boost::shared_ptr<Type> fullType;
            if (!decl && type->is_incomplete())
            {
                fullType = resolve(*type);
                if (fullType)
                {
                    swap(type, fullType);
                    type_ = typeMap_.find(*type);
                }
                else if (Thread* t = thread_.get())
                {
                    // lookup the type in other debug formats
                    TypeHelper helper(reader_, t, name);

                    if (Debugger* d = t->debugger())
                    {
                        d->enum_plugins(&helper);
                    }
                    type_ = helper.type();
                }
            }
            if (!type_)
            {
                CHKPTR(type)->accept(this);
            }
            if (type_)
            {
                typeMap_.add(*type, type_);

                if (fullType) // NOTE: has been swapped with type
                {
                    typeMap_.add(*fullType, type_);
                }
            }
        }
    }
    return type_;
}


////////////////////////////////////////////////////////////////
boost::shared_ptr<Type>
TypeAdapter::lookup_type(const Type& type,
                         const RefPtr<SharedString>& module,
                         bool byCtor
                        )const
{
    boost::shared_ptr<Type> result;

    if (thread_)
    {
        Handle dbg = reader_->get_debug_handle(module, thread_->process());
        if (dbg && dbg.get() != &type.owner())
        {
            if (byCtor)
            {
                result = dbg->lookup_type_by_ctor(type.name());
            }
            else
            {
                result = dbg->lookup_type_by_decl(type);
            }
        #ifdef DEBUG
            if (result)
            {
                clog << __func__ << ": " << type.name();
                clog << " found in " << module->c_str() << endl;
            }
        #endif
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
boost::shared_ptr<Type> TypeAdapter::resolve(const Type& type)
{
    assert(type.is_incomplete());
    boost::shared_ptr<Type> fullType;

    // FIXME: if a base is declared (the type is incomplete) and
    // we do not do a deep search, the auto-runtime-type-info
    // (auto-rtti) feature may not work correctly
    if (depth_ != ADAPT_FULL)
    {
        return fullType;
    }
    if (!thread_)
    {
        return fullType; // NULL
    }
    RefPtr<Process> process = thread_->process();

    const char* name = type.name();
    bool isCPlusPlusTemplate = false;

    // this is potentially faster than lookup_type_by_decl because
    // it uses the pubnames section, whereas the former dives into
    // each compilation unit
    if (name)
    {
        isCPlusPlusTemplate = strchr(name, '<');
        if (isCPlusPlusTemplate) // short-circuit
        {
            return fullType;
        }

        fullType = type.owner().lookup_type_by_ctor(name);
    }
    if (!fullType)
    {
        fullType = lookup_type_in_all_modules(process, type, true);
    }

    if (!fullType)
    {
        fullType = type.owner().lookup_type_by_decl(type);
    }
    if (!fullType && modName_)
    {
        fullType = lookup_type(type, modName_);
    }
    if (!fullType)
    {
        bool useExpensiveLookups = (expensive_type_lookup_level() > 1);
    /*
        if ((strncmp(type.name(), "std::", 5) == 0) ||
            (strncmp(type.name(), "_STL::", 6) == 0))
        {
            // lookup up ALL modules is VERY expensive, and stl
            // types that are not found on a first attempt tend
            // not to be resolved at all (example: std::basic_ostream<...)
            useExpensiveLookups = false;
        } */
        if (useExpensiveLookups)
        {
            // give it a more thorough try: lookup all modules by decl
            fullType = lookup_type_in_all_modules(process, type);
        }
    }
    if (fullType)
    {
        if (fullType->offset() == type.offset())
        {
            fullType.reset();
        }
    #ifdef DEBUG_ADAPTER
        else
        {
            type_ = typeMap_.find(*fullType);
            clog << type.cu_offset() << " decl: " << fullType->cu_offset();
            if (type_.get())
            {
                clog << " (" << type_->name() << ")";
            }
            clog << endl;
        }
    #endif
    }
    return fullType;
}



////////////////////////////////////////////////////////////////
boost::shared_ptr<Dwarf::Type>
TypeAdapter::lookup_type_in_all_modules(RefPtr<Process> process,
                                        const Type& what,
                                        bool byCtor
                                       ) const
{
    boost::shared_ptr<Dwarf::Type> type;
    if (!thread_ || !reader_)
    {
        return type;
    }
    std::string name;
    if (byCtor)
    {
        name = Debug::infer_ctor_name(what.name());
    }
    if (SymbolMap* symbols = thread_->symbols())
    {
        for (RefPtr<SymbolMap::LinkData> link = symbols->file_list();
             link;
             link = link->next()
            )
        {
            if (modName_->is_equal2(link->filename()))
            {
                continue;
            }
        #if 0
            type = lookup_type(what, link->filename(), byCtor);
        #else
            if (byCtor)
            {
                if (Handle dbg =
                    reader_->get_debug_handle(link->filename(), process))
                {
                    type = dbg->lookup_type_by_ctor(name);
                }
            }
            else
            {
                type = lookup_type(what, link->filename(), byCtor);
            }

        #endif
            if (type)
            {
                break;
            }
        }
    }
    return type;
}


////////////////////////////////////////////////////////////////
TypeSystem& TypeAdapter::type_system()
{
    return *::type_system(thread_);
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
