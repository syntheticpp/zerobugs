//
// $Id: compile_unit.cpp 715 2010-10-17 21:43:59Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#ifndef _GNU_SOURCE
 #define _GNU_SOURCE // turn on ISOC99 features
#endif

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <dwarf.h>
#include "public/class_type.h"
#include "public/debug.h"
#include "public/error.h"
#include "public/function.h"
#include "public/imported_decl.h"
#include "public/inlined_instance.h"
#include "public/namespace.h"
#include "public/type.h"
#include "public/utils.h"
#include "public/variable.h"
#include "private/comp_dir_attr.h"
#include "private/log.h"
#include "private/factory.h"
#include "private/generic_attr.h"
#include "impl.h"
#include "public/compile_unit.h"
#include "unmangle/unmangle.h"
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"


using namespace Dwarf;
using namespace boost;
using namespace std;

#include "line_info_cache.h"


MacroEvents::~MacroEvents()
{
}


CompileUnit::TypeEnumeration::TypeEnumeration(
    const boost::shared_ptr<CompileUnit>& unit)
    : unit_(unit)
    , iterByOffs_(unit->tspecs_.begin())
    , iterByName_(unit->types_.begin())
{
}

boost::shared_ptr<Type> CompileUnit::TypeEnumeration::next()
{
    boost::shared_ptr<Type> type;

    if (iterByOffs_ != unit_->tspecs_.end())
    {
        type = iterByOffs_->second;
        ++iterByOffs_;
    }
    else if (iterByName_ != unit_->types_.end())
    {
        type = iterByName_->second;
        ++iterByName_;
    }
    return type;
}


CompileUnit::CompileUnit (
    Dwarf_Debug     dbg,
    Dwarf_Die       entry,
    Dwarf_Unsigned  next,
    WeakPtr<StringCache> strCache
    )
  : Die(dbg, entry)
  , next_(next)
  , lineInfoCache_(0)
  , populated_(false)
  , base_(Dwarf_Addr(-1))
  , lowPC_(Dwarf_Addr(-1))
  , highPC_(0)
  , srcFiles_(0)
  , strCache_(strCache)
{
}


CompileUnit::~CompileUnit() throw()
{
    if (lineInfoCache_)
    {
        delete lineInfoCache_;
    }
}


const char* CompileUnit::producer() const
{
    if (producer_.empty())
    {
        if (Utils::has_attr(dbg(), die(), DW_AT_producer))
        {
            typedef GenericAttr<DW_AT_producer, char*> AttrType;
            producer_ = AttrType(dbg(), die()).str();
        }
    }
    return producer_.c_str();
}


const CompileUnit::LineInfoCache& CompileUnit::line_info_cache() const
{
    if (!lineInfoCache_)
    {
        lineInfoCache_ = new LineInfoCache(*this, dbg(), die());
    }
    return *lineInfoCache_;
}


boost::shared_ptr<CompileUnit>
CompileUnit::next_unit(Dwarf_Debug dbg, Dwarf_Unsigned offs)
{
    boost::shared_ptr<CompileUnit> result;
    if (dbg)
    {
        Dwarf_Error err = 0;
        Dwarf_Unsigned next = offs;

        int rc = dwarf_next_cu_header(dbg, 0, 0, 0, 0, &next, &err);

        if (rc == DW_DLV_ERROR)
        {
            throw Error("dwarf_next_cu_header", dbg, err);
        }
        if (rc == DW_DLV_OK)
        {
            Dwarf_Die die = 0;

            if (dwarf_siblingof(dbg, 0, &die, &err) != DW_DLV_OK)
            {
                throw Error("dwarf_siblingof", dbg, err);
            }

            Dwarf_Half tag = 0;
            if (dwarf_tag(die, &tag, &err) != DW_DLV_OK)
            {
                dwarf_dealloc(dbg, die, DW_DLA_DIE);
                throw Error("dwarf_tag", dbg, err);
            }
            if (tag != DW_TAG_compile_unit)
            {
                dwarf_dealloc(dbg, die, DW_DLA_DIE);
                throw runtime_error("DW_TAG_compile_unit");
            }
            if (const Debug* dptr = Debug::get_wrapper(dbg))
            {
                result = dptr->get_compile_unit(die, next);
            }
            else
            {
                log<debug>() << "next_unit: invalid dbg handle.\n";
            }
        }
    }
    return result;
}


static void
get_string(WeakPtr<StringCache> strCache,
           const string& strIn,
           RefPtr<SharedString>& strOut)
{
    if (RefPtr<StringCache> cache = strCache.lock())
    {
        strOut = cache->get_string(strIn.c_str(), strIn.length());
        assert(strOut);
    }
    else
    {
        strOut = shared_string(strIn);
    }
}


RefPtr<SharedString> CompileUnit::build_path() const
{
    if (buildpath_.is_null())
    {
        CompDirAttr attr(dbg(), die());
        string buildpath = attr.value();

        //todo: pull this into function ensure_path_is_delim_ended
        if (!buildpath.empty())
        {
            if (buildpath[buildpath.size() - 1] != '/')
            {
                buildpath += '/';
            }
        }
        //end todo
        get_string(strCache_, buildpath, buildpath_);
    }
    return buildpath_;
}


RefPtr<SharedString> CompileUnit::full_path() const
{
    if (fullpath_.is_null())
    {
        assert(Die::name());
        string fullpath = Die::name();

        if (Die::name()[0] != '/')
        {
            fullpath = build_path()->c_str() + fullpath;
        }
        get_string(strCache_, fullpath, fullpath_);
        assert(fullpath_); // post-cond
    }
    return fullpath_;
}


RefPtr<SharedString> CompileUnit::filename_by_index(size_t n) const
{
    if (n < source_files_count())
    {
        return srcFiles_[n];
    }
    return RefPtr<SharedString>(); // NULL
}

/**
 *  Lookup a source filename
 */
RefPtr<SharedString> CompileUnit::filename(const char* name) const
{
    const size_t n = source_files_count();

    for (size_t i = 0; i != n; ++i)
    {
        if (srcFiles_[i]->is_equal(name))
        {
            return srcFiles_[i];
        }
    }
    return RefPtr<SharedString>(); // NULL
}


size_t CompileUnit::source_files_count() const
{
    if (srcFiles_.empty())
    {
        Dwarf_Error err = 0;
        char** srcFiles = NULL;
        Dwarf_Signed numSrcFiles = 0;

        if (dwarf_srcfiles(die(), &srcFiles, &numSrcFiles, &err) == DW_DLV_ERROR)
        {
            throw Error(dbg(), err);
        }
        if (srcFiles)
        {
            assert(numSrcFiles > 0);
            srcFiles_.reserve(numSrcFiles);

            for (Dwarf_Signed i = 0; i != numSrcFiles; ++i)
            {
                try
                {
                    RefPtr<SharedString> fullPath =
                        ::full_path(*this, srcFiles[i], NULL, false);

                    srcFiles_.push_back(fullPath);
                }
                catch (...)
                { }
                dwarf_dealloc(dbg(), srcFiles[i], DW_DLA_STRING);
            }
            dwarf_dealloc(dbg(), srcFiles, DW_DLA_LIST);
        }
        else
        {
            assert(numSrcFiles == 0);
        }

        if (srcFiles_.size() != size_t(numSrcFiles))
        {
            srcFiles_.clear();
            throw runtime_error(__func__);
        }
    }
    return srcFiles_.size();
}


size_t CompileUnit::addr_to_line(Dwarf_Addr      addr,
                                 Dwarf_Addr*     nearest,
                                 SrcLineEvents*  events) const
{
    return line_info_cache().addr_to_line(addr, nearest, events);
}



Dwarf_Addr CompileUnit::next_line(const char* file,
                                  size_t      line,
                                  Dwarf_Addr  addr,
                                  size_t*     next) const
{
    size_t nextLine = 0;
    const LineInfoCache& cache = line_info_cache();
    Dwarf_Addr result = cache.next_line(file, line, addr, &nextLine);

    if (result)
    {
        if (next)
        {
            *next = nextLine;
        }
    }
    return result;
}


bool CompileUnit::cache_srclines(SrcLineEvents* events)
{
    if (!lineInfoCache_)
    {
        lineInfoCache_ = new LineInfoCache(*this, dbg(), die());
    }
    bool result = lineInfoCache_->export_line_info(events);
    return result;
}


List<Namespace> CompileUnit::namespaces() const
{
    return List<NamespaceT<CompileUnit> >(dbg(), die());
}


/**
 * @return the functions defined in this translation unit
 */
const FunList& CompileUnit::functions() const
{
    if (funcs_.empty())
    {
        if (!populated_)
        {
            read_children();
            assert(populated_);
        }
        for (vector<Dwarf_Off>::const_iterator i = funcOffs_.begin();
             i != funcOffs_.end();
             ++i)
        {
            Dwarf_Error err = 0;
            Dwarf_Die die = 0;

            if (dwarf_offdie(dbg(), *i, &die, &err) == DW_DLV_ERROR)
            {
                throw Error("dwarf_offdie", dbg(), err);
            }
            boost::shared_ptr<Function> f(new Function(dbg(), die));
            f->unit_ = this;

            funcs_.push_back(f);
        }
    }
    return funcs_;
}


/**
 * lookup type by declaration
 */
boost::shared_ptr<Type> CompileUnit::lookup_type(const Type& decl) const
{
    boost::shared_ptr<Type> type;
    if (!populated_)
    {
        read_children();
    }

    TypeMapByOffset::const_iterator i = tspecs_.find(decl.offset());
    if (i != tspecs_.end())
    {
        type = i->second;
        assert(type->is_complete());
    }
    else
    {
        type = lookup_type(decl.name());
    }
    return type;
}


boost::shared_ptr<Type> CompileUnit::lookup_type(const char* name) const
{
    boost::shared_ptr<Type> type;
    if (!populated_)
    {
        read_children();
    }

    TypeMapByName::const_iterator i = types_.find(name);
    if (i != types_.end())
    {
        type = i->second;
    }
#ifdef DEBUG
    if (!type)
    {
        for(i = types_.begin(); i != types_.end();++i)
        {
            assert(strcmp_ignore_space(i->first, name));
        }
    }
#endif
    return type;
}


void CompileUnit::get_range_from_funcs() const
{
    if ((highPC_ == 0) || (lowPC_ == Dwarf_Addr(-1)))
    {
        const FunList& funcs = functions();
        FunList::const_iterator i = funcs.begin();
        const FunList::const_iterator end = funcs.end();

        for (; i != end; ++i)
        {
            const Dwarf_Addr high = (*i)->high_pc();

            if (high > highPC_)
            {
                highPC_ = high;
            }

            const Dwarf_Addr low = (*i)->low_pc();

            if (low && (low < lowPC_))
            {
                lowPC_ = low;
            }
        }
        uncache_functions();
    }
}


Dwarf_Addr CompileUnit::base_pc() const
{
    if (base_ == Dwarf_Addr(-1))
    {
        Dwarf_Error err = 0;

        if (Utils::has_attr(dbg(), die(), DW_AT_low_pc))
        {
            if (dwarf_lowpc(die(), &base_, &err) == DW_DLV_ERROR)
            {
                assert(base_ == Dwarf_Addr(-1));
                throw Error(dbg(), err);
            }
        }
        else
        {
            base_ = 0;
        }
    }
    return base_;
}


Dwarf_Addr CompileUnit::low_pc() const
{
    if (lowPC_ == Dwarf_Addr(-1))
    {
        if (Dwarf_Addr pc = base_pc())
        {
            lowPC_ = pc;
        }
        else // infer from func
        {
            get_range_from_funcs();
        }
    }
    return lowPC_;
}


Dwarf_Addr CompileUnit::high_pc() const
{
    if (highPC_ == 0)
    {
        Dwarf_Error err = 0;

        if (Utils::has_attr(dbg(), die(), DW_AT_high_pc))
        {
            if (dwarf_highpc(die(), &highPC_, &err) == DW_DLV_ERROR)
            {
                throw Error(dbg(), err);
            }
        }
        else
        {
            get_range_from_funcs();
        }
    }
    return highPC_;
}


const VarList& CompileUnit::variables() const
{
    if (!populated_)
    {
        read_children();
    }
    return vars_;
}


boost::shared_ptr<Function>
CompileUnit::lookup_function(Dwarf_Addr addr, const char* linkage) const
{
    return Utils::lookup_function(functions(), addr, linkage);
}


boost::shared_ptr<CompileUnit>
IterationTraits<CompileUnit>::first(Dwarf_Debug dbg, Dwarf_Die)
{
    return CompileUnit::next_unit(dbg, 0);
}


void IterationTraits<CompileUnit>::next
(
    boost::shared_ptr<CompileUnit>& elem
)
{
    assert(elem);
    elem = CompileUnit::next_unit(elem->dbg(), elem->next_);
}


/**
 * A list of children tags that we handle when reading a compilation unit
 */
static const Dwarf_Half tags[] =
{
    DW_TAG_array_type,
    DW_TAG_base_type,
    DW_TAG_class_type,
    DW_TAG_const_type,
    DW_TAG_enumeration_type,
//  DW_TAG_inlined_subroutine,
    DW_TAG_imported_declaration,
//  DW_TAG_interface_type,      // Java-specific. todo: ask Walter if D needs it
    DW_TAG_lexical_block,
    DW_TAG_module,
    DW_TAG_namespace,

    DW_TAG_pointer_type,
    DW_TAG_ptr_to_member_type,
    DW_TAG_reference_type,
//  DW_TAG_restrict_type,       // as of GCC 4.2.3 it seems as this tag is not generated

//  DW_TAG_set_type,            // Not needed for C/C++. As per DWARF3 (http://dwarfstd.org/):
// "Pascal provides the concept of a “set,” which represents a group of values of ordinal type.
//  A set is represented by a debugging information entry with the tag DW_TAG_set_type.

    DW_TAG_string_type,
    DW_TAG_structure_type,
    DW_TAG_subrange_type,       // todo
    DW_TAG_subroutine_type,
    DW_TAG_subprogram,          // function

    DW_TAG_thrown_type,         // todo: investigate whether Intel Compiler generates this;
                                // as of GCC 4.2.3 it seems as this tag is not generated
    DW_TAG_typedef,
    DW_TAG_union_type,
//  DW_TAG_unspecified_type,
    DW_TAG_variable,
    DW_TAG_volatile_type,
};
static const size_t numTags = sizeof(tags)/sizeof(tags[0]);


void CompileUnit::read_children(Dwarf_Die die,
                                const char* prefix,
                                bool ignoreVars) const
{
    Dwarf_Half tag = 0;
    Dwarf_Die child = Utils::first_child(dbg(), die, tags, numTags, &tag);

    for (Dwarf_Die trash = 0; child;)
    {
        boost::shared_ptr<Die> ptr = process_entry(child, tag, prefix, ignoreVars);
        if (!ptr)
        {
            trash = child;
        }

        child = Utils::next_sibling(dbg(), child, tags, numTags, &tag);

        if (trash && (trash != child))
        {
            dwarf_dealloc(dbg(), trash, DW_DLA_DIE);
            trash = 0;
        }
    }
    populated_ = true;
}


boost::shared_ptr<Die>
CompileUnit::process_entry( Dwarf_Die die,
                            Dwarf_Half tag,
                            const char* prefix,
                            bool ignoreVars) const
{
    assert(die);
    assert(tag);
    boost::shared_ptr<Die> child;

    switch (tag)
    {
    case DW_TAG_module:
        // C/C++ do not have module support within the language;
        // so far the only supported language that has modules
        // is D, and the simplifying assumption is that there is
        // only one module per compilation unit.
        if (language() == DW_LANG_D)
        {
            string name;
            if (prefix)
            {
                 name.assign(prefix);
                 name += ".";
            }

            string tmp = Die::name(dbg(), die);
            if (RefPtr<StringCache> cache = strCache_.lock())
            {
                module_ = cache->get_string(tmp.c_str(), tmp.length());
            }
            else
            {
                module_ = shared_string(tmp);
            }
            name += tmp + ".";
            read_children(die, name.c_str(), ignoreVars);
        }
        break;

    case DW_TAG_namespace:
        {
            boost::shared_ptr<Namespace> ns(new Namespace(dbg(), die));

            child = ns;
            string name;
            if (ns->name() && *ns->name() && strcmp(ns->name(), "::"))
            {
                if (prefix)
                {
                    name.assign(prefix);
                }
                name += ns->name();
                name += "::";
            }
            read_children(die, name.c_str(), ignoreVars);
        }
        break;

    case DW_TAG_subprogram:
        {
            ignoreVars = true;
            read_children(die, prefix, ignoreVars);

            funcOffs_.push_back(Die::offset(dbg(), die));
        }
        break;

    case DW_TAG_variable:
        if (!ignoreVars)
        {
            boost::shared_ptr<Variable> v(new Variable(dbg(), die));
            child = v;
            vars_.push_back(v);
        }
        break;

    case DW_TAG_class_type:
    case DW_TAG_structure_type:
        child = add_struct(die, prefix);

        if (1)
        {
            // look for possible nested class definitions, etc.
            string name = child->name();
            if (prefix)
            {
                name = prefix + name;
            }
            name += "::";
            read_children(die, name.c_str(), ignoreVars);
        /*
            if (boost::shared_ptr<KlassType> klass =
                shared_dynamic_cast<KlassType>(child))
            {
                const MethodList& memFun = klass->methods();
                funcs_.insert(funcs_.end(), memFun.begin(), memFun.end());
            }
         */
        }
        break;

    case DW_TAG_lexical_block:
        read_children(die, prefix, ignoreVars);
        break;

/* todo: handle imported decls

    case DW_TAG_imported_declaration:
        child = Factory::instance().create(dbg(), die, tag, false);
        if (boost::shared_ptr<ImportedDecl> decl =
            shared_dynamic_cast<ImportedDecl>(child))
        {
            cout << __func__ << ": " << decl->name() << endl;
            if (boost::shared_ptr<Type> import =
                shared_dynamic_cast<Type>(decl->get_import()))
            {
                if (prefix)
                {
                    string name(prefix);
                    const char* n = import->name();
                    if (const char* p = strstr(n, "::"))
                    {
                        name += p + 2;
                    }
                    else
                    {
                        name += n;
                    }
                    import->set_name(name.c_str());
                    types_.insert(make_pair(import->name(), import));
                }
            }
        }
        break; */

    default:
        if (Utils::has_attr(dbg(), die, DW_AT_specification))
        {
            child = Factory::instance().create(dbg(), die, tag, false);

            if (boost::shared_ptr<Type> type = shared_dynamic_cast<Type>(child))
            {
                Dwarf_Off offset = type->decl_offset();
                assert(offset);
                tspecs_.insert(make_pair(offset, type));
            }
            else if (boost::shared_ptr<KlassType> klass =
                shared_dynamic_cast<KlassType>(child))
            {
                types_.insert(make_pair(klass->name(), klass));
            }
        }
        break;
    }
    return child;
}


boost::shared_ptr<Dwarf::Die>
CompileUnit::add_struct(Dwarf_Die die, const char* prefix) const
{
    // First choice that may come to mind here is:
    // static const Dwarf_Half tag = DW_TAG_structure_type;
    // boost::shared_ptr<Die> child =
    //     Factory::instance().create(dbg(), die, tag, false);
    //
    // Don't do the above because: get_object uses a cache internally;
    // DW_AT_specification objects will be linked to type
    // (whose name may be modified here to include a namespace prefix)
    Dwarf_Off off = Die::offset(dbg(), die);
    boost::shared_ptr<Die> child(owner().get_object(off));

    if (boost::shared_ptr<Type> type = shared_dynamic_cast<Type>(child))
    {
        if (prefix)
        {
            string name = prefix;
            name += type->name();
            type->set_name(name.c_str());
        }
        if (type->is_complete())
        {
            // complete type definitions override forward decls
            TypeMapByName::iterator i = types_.find(type->name());
            if (i != types_.end() && i->second->is_incomplete())
            {
                types_.erase(i);
            }
        }
        types_.insert(make_pair(type->name(), type));
    }
    return child;
}


size_t
CompileUnit::enum_macros(MacroEvents* events, size_t maxCount) const
{
    GenericAttr<DW_AT_macro_info, Dwarf_Off> attr(dbg(), die());
    if (attr.is_null())
    {
        return 0;
    }
    const Dwarf_Off off = attr.value();

    Dwarf_Macro_Details* details = 0;
    Dwarf_Signed count = 0;
    Dwarf_Error err = 0;
    int res = dwarf_get_macro_details(dbg(), off, maxCount,
                                      &count, &details, &err);
    if (res != DW_DLV_OK)
    {
        throw Error("dwarf_get_macro_details", dbg(), err);
    }
    Utils::AutoDealloc<Dwarf_Macro_Details, DW_DLA_STRING>
        autoDealloc(dbg(), details);

    if (events)
    {
        for (Dwarf_Signed i = 0; i != count; ++i)
        {
            if (!events->on_macro(details[i]))
            {
                count = 1;
                break;
            }
        }
    }
    return count;
}


int CompileUnit::language() const
{
    if (Utils::has_attr(dbg(), die(), DW_AT_language))
    {
        return GenericAttr<DW_AT_language, int>(dbg(), die()).value();
    }
    return 0;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
