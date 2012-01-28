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
#include <stab.h>
#include <iostream>
#include <stdexcept>
#include "dharma/canonical_path.h"
#include "zdk/shared_string_impl.h"
#include "generic/temporary.h"
#include "elfz/public/binary.h"
#include "elfz/public/section.h"
#include "public/compile_unit.h"
#include "public/descriptor.h"
#include "public/fwdtype.h"

#include "private/init_events.h"
#include "private/throw.h"
#include "private/util.h"

#define this_func std::string(__func__)

using namespace std;


/*  Usual STAB section names, with their corresponding
    string section names;just appending "str" to the section
    name would've been okay, if it wasn't for $GDB_SYMBOLS$
    and $GDB_STRINGS$. */

static const char* secName[][2] =
{
    { ".stab",          ".stabstr"          },
    { ".stab.excl",     ".stab.exclstr"     },
    { ".stab.index",    ".stab.indexstr"    },
    { "$GDB_SYMBOLS$",  "$GDB_SYMBOLS$"     },
};

static const size_t nameCount = sizeof(secName) / sizeof(secName[0]);

/**
 * Helper function, looks up a section's name in the
 * table above; if there's a match, return the name of
 * the corresponding strings section, otherwise return NULL.
 */
static const char* stab_strings_name(const ELF::Section& section)
{
    for (size_t i = 0; i != nameCount; ++i)
    {
        if (strcmp(section.header().name(), secName[i][0]) == 0)
        {
            return secName[i][1];
        }
    }
    return 0;
}


static ELF::List<ELF::Section>::const_iterator
find_section_by_name(const ELF::Binary& elf, const char* name)
{
    assert(name);

    const ELF::List<ELF::Section>& scns = elf.sections();
    ELF::List<ELF::Section>::const_iterator i = scns.begin();
    const ELF::List<ELF::Section>::const_iterator end = scns.end();

    for (; i != end; ++i)
    {
        if (strcmp(section_name(*i), name) == 0)
        {
            return i;
        }
    }
    return end;
}


/**
 * Locates the strings section that corresponds to the
 * given stab section.
 */
static ELF::List<ELF::Section>::const_iterator
find_stab_strings(
    const ELF::Binary& elf,
    ELF::List<ELF::Section>::const_iterator i)
{
    const ELF::Section& sec = *i;
    const char* name = stab_strings_name(sec);

    ELF::List<ELF::Section>::const_iterator end = elf.sections().end();

    if (name == 0)
    {
        i = end;
    }
    else
    {
        /*  Check if the next section is the strings section
            for this stabs section, and if not, search for the
            strings section. */

        if (++i == end || strcmp(section_name(sec), name))
        {
            i = find_section_by_name(elf, name);
        }
    }
    return i;
}


Stab::Descriptor::Descriptor(const char* fileName)
    : self_(*this)
    , fileName_(shared_string(fileName))
    , initialized_(false)
    , parsed_(false)
    , startIndex_(0)
    , observer_(0)
{
    init_hash_maps();
}


Stab::Descriptor::Descriptor(SharedString& fileName)
    : self_(*this)
    , fileName_(&fileName)
    , initialized_(false)
    , parsed_(false)
    , startIndex_(0)
    , observer_(0)
{
    init_hash_maps();
}


Stab::Descriptor::~Descriptor() throw()
{
}



void Stab::Descriptor::init_hash_maps()
{
    RefPtr<SharedString> null;

    headers_.set_empty_key(null);
    fwdTypeMap_.set_empty_key(null);
    typeMap_.set_empty_key(null);
}


void Stab::Descriptor::init()
{
    if (!initialized_)
    {
        Stab::InitEvents init(*this);
        Stab::Events* events = &init;

        for_each_stab_section(&events, 1);
        initialized_ = true;
    }
}


Stab::Descriptor::UnitList& Stab::Descriptor::unit_list()
{
    if (!initialized_)
    {
        init();
    }
    return unitList_;
}


const Stab::Descriptor::UnitList& Stab::Descriptor::unit_list() const
{
    if (!initialized_)
    {
        self_.init();
    }
    return unitList_;
}


SharedString& Stab::Descriptor::name() const
{
    assert(!fileName_.is_null());
    return *fileName_;
}


void
Stab::Descriptor::for_each_stab_section(Events** events, size_t numEvents)
{
    assert(events);

    ELF::Binary binary(fileName_->c_str());

    string link = ELF::debug_link(binary);

    if (link.empty())
    {
        for_each_stab_section(binary, events, numEvents);
    }
    else
    {
        ELF::Binary externalSymbols(link.c_str());
        for_each_stab_section(externalSymbols, events, numEvents);
    }
}


void
Stab::Descriptor::for_each_stab_section(const ELF::Binary& binary,
                                        Events** events,
                                        size_t numEvents)
{
    assert(events);

    ELF::List<ELF::Section>::const_iterator i = binary.sections().begin();
    const ELF::List<ELF::Section>::const_iterator end = binary.sections().end();

    for (; i != end; ++i)
    {
        ELF::List<ELF::Section>::const_iterator j = find_stab_strings(binary, i);

        if (j != end)
        {
            const char* name = section_name(*i);
            on_section(name);

            Events** ev = events;
            size_t ec = numEvents;

            for (; ec; ++ev, --ec)
            {
                assert(*ev);
                (*ev)->on_section(name);
            }
            for_each_stab(i->data(), j->data(), events, numEvents);
        }
    }
}


/**
 * Process a stab entry, by calling Stab::Events::on_stab.
 * If the stab's associated string ends with a backslash, then
 * assemble the string into a buffer, and defer calling on_stab
 * until a non-backslash-ended string is seen.
 */
bool Stab::Descriptor::process_stab(
    size_t              index,
    const Stab::stab_t& stab,
    const Elf_Data&     strData,
    string&             buf,
    Stab::Events**      events,
    size_t              numEvents)
{
    if (stab.strindex() >= strData.d_size)
    {
    #ifdef DEBUG
        clog << "stab.strindex=" << stab.strindex() << endl;
        clog << "strData.d_size="<< strData.d_size << endl;
    #endif
        THROW (out_of_range("string index exceeds section's size"));
    }
    else
    {
        const char* str =
            static_cast<const char*>(strData.d_buf) + stab.strindex();
        size_t len = strlen(str);
        assert(len <= strData.d_size - stab.strindex());
        if (len && str[len - 1] == '\\')
        {
            buf.append(str, str + len - 1);
    #ifdef DEBUG
            clog << __func__ << ": " << buf << endl;
    #endif
        }
        else
        {
            if (!buf.empty())
            {
                str = buf.c_str();
                len = buf.length();
            }
            if (!on_stab(index, stab, str, len))
            {
                return false;
            }

            for (; numEvents; ++events, --numEvents)
            {
                assert(*events);

                if (!(*events)->on_stab(index, stab, str, len))
                {
                    return false;
                }
            }
            /* done assembling and processing the string */
            buf.erase();
            assert(buf.length() == 0);
        }
    }
    return true;
}


void
Stab::Descriptor::for_each_stab(
    const Elf_Data& stabData,
    const Elf_Data& strData,
    Stab::Events**  events,
    size_t          numEvents)
{
    assert(events);

    const stab_t* stab = reinterpret_cast<const stab_t*>(stabData.d_buf);
    const size_t  size = stabData.d_size / sizeof (stab_t);

    assert(stab);

    //clog << __func__ << ": size=" << size << endl;

    on_begin(0, size);

    /* In case the string ends with a backslash, we need
       to append following string(s) to it. Use a string
       buffer for the purpose. */
    string buf;

    size_t i = 0;
    for (; i < size; ++i, ++stab)
    {
        if (!process_stab(i, *stab, strData, buf, events, numEvents))
        {
            break;
        }
    }
    on_done(i);
}


void
Stab::Descriptor::for_each_stab(
    SharedString&   section,
    size_t          first,
    size_t          last,
    Stab::Events**  events,
    size_t          numEvents,
    SharedString*   msg)
{
    assert(!fileName_.is_null());

    ELF::Binary elf(fileName_->c_str());
    for_each_stab(elf, section, first, last, events, numEvents, msg);
}


void
Stab::Descriptor::for_each_stab(
    ELF::Binary&    elf,
    SharedString&   section,
    size_t          first,
    size_t          last,
    Stab::Events**  events,
    size_t          numEvents,
    SharedString*   msg)
{
    assert(events);

    if (last < first)
    {
        THROW (invalid_argument(this_func + ": last < first"));
    }

    ELF::List<ELF::Section>::const_iterator i =
        find_section_by_name(elf, section.c_str());

    const ELF::List<ELF::Section>::const_iterator end = elf.sections().end();

    if (i == end)
    {
        THROW (runtime_error(this_func + ": " + section.c_str() + ": not found"));
    }
    else
    {
        ELF::List<ELF::Section>::const_iterator j = find_stab_strings(elf, i);

        if (j == end)
        {
            THROW (runtime_error(
                this_func + ": not a STAB section: " + section.c_str()));
        }
        else
        {
            const Elf_Data& data = i->data();
            const Elf_Data& strData = j->data();

            if (last >= data.d_size)
            {
                THROW (out_of_range("range exceeds section's data"));
            }
            else if (first < last)
            {
                // on_begin(section.c_str(), last);
                const size_t size = last - first;
                on_begin(section.c_str(), size, first, msg);
           /*
                size_t ec = numEvents;
                for (Events** ev = events; ec; --ec, ++ev)
                {
                    assert(*ev);
                    (*ev)->on_begin(section, "", last);
                }
            */
                string buf;

                for (; first != last; ++first)
                {
                    const stab_t* stab =
                        reinterpret_cast<const stab_t*>(data.d_buf)
                        + first;

                    if (!process_stab(first, *stab, strData, buf, events, numEvents))
                    {
                        break;
                    }
                }
                //on_done(first);
                on_done(size);
            }
        }
    }
}


/**
 * Create a new CompileUnit, and add it to the unit list.
 */
Stab::CompileUnit& Stab::Descriptor::new_compile_unit(
    SharedString&   section,
    const stab_t&   stab,
    SharedString*   buildPath,
    const char*     filename,
    size_t          index)
{
    assert(filename);

#if DEBUG && !PROFILE
    /*  I believe that this filename should occur only
        once, but just to make sure my theory is correct... */
    UnitList::const_iterator i = unitList_.begin();
    for (; i != unitList_.end(); ++i)
    {
        if ((*i)->name().is_equal(filename))
        {
            assert ( !(*i)->build_path().is_equal2(buildPath) );
        }
    }
#endif

    /*  The stab's value is the start address of the portion
        of the text section corresponding to the file. */

    RefPtr<CompileUnit> unit =
            new CompileUnit(*this,
                            section,
                            buildPath,
                            filename,
                            stab.value(),
                            index);

    unitList_.push_back(unit);
    unitMap_.insert(make_pair(unit->begin_addr(), unit));

    return *unit;
}


/**
 * Set the ending index and ending address, and add the
 * unit to the unit map if the ending address is greather
 * than the starting addr.
 */
void
Stab::Descriptor::finish_compile_unit(
    CompileUnit&    unit,
    addr_t          addr,
    size_t          index)
{
    // pre-condiddtions
    assert(unit.end_index() == 0);
    assert(unit.end_addr() == 0);

    unit.set_end_addr(addr);
    unit.set_end_index(index);

   /*
    if (unitMap_.find(unit.begin_addr()) == unitMap_.end())
    {
        unitMap_.insert(make_pair(unit.begin_addr(), &unit));
    }
    */
}


Stab::CompileUnit*
Stab::Descriptor::get_compile_unit(addr_t addr, size_t index) const
{
    if (!initialized_)
    {
        self_.init();
    }
    addr_t startRange = addr;

    UnitMap::const_iterator i = unitMap_.lower_bound(addr);
    if ((i == unitMap_.end()) || (i->first != addr))
    {
        if (i == unitMap_.begin())
        {
            return NULL;
        }
        --i;
        startRange = i->first;

        while (i != unitMap_.begin() && i->first == startRange)
        {
            --i;
        }
        if (i->first != startRange)
        {
            ++i;
        }
    }

    for (; ; ++i)
    {
        if (i == unitMap_.end() || i->first != startRange)
        {
            return NULL;
        }

        if (index)
        {
            // match the index, if specified
            if (index == i->second->begin_index())
            {
                break;
            }
        }
        else
        {
    #if 0
            // TODO: investigate why this is not working
            if (addr < i->second->end_addr())
            {
                assert(i->second->size());
                break;
            }
            else
            {
                clog << endl << i->second->name()->c_str();
                clog << ": " << hex << addr << " not in ["
                     << i->second->begin_addr() << ", "
                     << i->second->end_addr() << "]\n"
                     << dec;
            }
    #else
            // no index given, lookup first non-empty unit
            if (i->second->size())
            {
                break;
            }
    #endif
        }
    }
    return i->second.get();
}


Stab::TypeTablePtr
Stab::Descriptor::add_type_table(
    const RefPtr<SharedString>& section,
    const RefPtr<SharedString>& file,
    const stab_t& stab)
{
    TypeTablePtr table;
    HeaderHash& hash = headers_[section];

    if (stab.type() == N_EXCL)
    {
        HeaderHash::const_iterator i = hash.find(stab.value());

        if (i != hash.end())
        {
            HeaderMap::const_iterator j = i->second.find(file);

            if (j != i->second.end())
            {
                table = j->second;
            }
        }
        assert(table);
    }

    //  N_BINCL or stab.value not found in hash?
    if (!table)
    {
#if (__GNUC__ >= 3)
        table.reset(new TypeTable);

#else // work-around for 2.95 optimizer bug

        TypeTablePtr(new TypeTable).swap(table);
#endif
        HeaderMap& hmap = hash[stab.value()];
        hmap.insert(make_pair(file, table));
    }

    return table;
}


void Stab::Descriptor::add_source_line(
    const RefPtr<CompileUnit>&  unit,
    const RefPtr<SharedString>& file,
    size_t                      line,
    addr_t                      addr)
{
    if (addr && line)
    {
        UnitMap::iterator i = unitMap_.lower_bound(addr);
        if (i == unitMap_.end())
        {
            unitMap_.insert(make_pair(addr, unit));
        }
        else
        {
            if (i != unitMap_.begin())
            {
                --i;
            }
            if (i->second != unit)
            {
                unitMap_.insert(make_pair(addr, unit));
            }
        }

        AddrByLine& addrByLine = addrByFile_[file];
        addrByLine.insert(make_pair(line, addr));
    }
}



vector<addr_t>
Stab::Descriptor::line_to_addr(RefPtr<SharedString> file, size_t line)
{
    vector<addr_t> result;

    if (!initialized_)
    {
        init();
        assert(initialized_);
    }
    AddrByFile::const_iterator i = addrByFile_.find(file);
    if (i != addrByFile_.end())
    {
        const AddrByLine& addrByLine = i->second;

        /* todo: make a separate function for this */
        /* special case, return the lowest address */
        if (line == 0)
        {
            addr_t addr = 0;
            AddrByLine::const_iterator j = addrByLine.begin();

            for (; j != addrByLine.end(); ++j)
            {
                if (addr == 0 || j->second < addr)
                {
                    addr = j->second;
                }
            }
            if (addr)
            {
                result.push_back(addr);
            }
            return result;
        }
        assert(line);

        /* find the closest address even for lines that
           don't have an entry in the map (i.e. lines in
           the source that did not generate any machine code). */

        AddrByLine::const_iterator j = addrByLine.lower_bound(line);

#if 1
        if (j != i->second.end())
        {
            const size_t nearestLine = j->first;
            for (; j->first == nearestLine && j != addrByLine.end(); ++j)
            {
                result.push_back(j->second);
            }
        }
#else
        if (j == addrByLine.end() || j->first != line)
        {
            if (j == addrByLine.begin())
            {
                return result;
            }
            --j;
        }

        const size_t nearestLine = j->first;
        for (; j->first == nearestLine && j != addrByLine.end(); ++j)
        {
            result.push_back(j->second);
        }
#endif
    }
    return result;
}


addr_t Stab::Descriptor::next_line(
    RefPtr<SharedString> sourceFile,
    size_t               line,
    addr_t               addr,
    size_t*              nextLine) const
{
    if (addr)
    {
        RefPtr<Stab::CompileUnit> unit = get_compile_unit(addr);
        if (unit.is_null())
        {
            addr = 0;
        }
        else
        {
            addr = unit->next_line(sourceFile, line, addr, nextLine);
            if (addr)
            {
                return addr;
            }
        }
    }
    assert(addr == 0);

    AddrByFile::const_iterator i = addrByFile_.find(sourceFile);

    if (i != addrByFile_.end())
    {
        AddrByLine::const_iterator j = i->second.upper_bound(line);

        if (j != i->second.end())
        {
            if (nextLine)
            {
                *nextLine = j->first;
            }
            addr = j->second;
        }
    }
    return addr;
}


/*
size_t Stab::Descriptor::nearest_line(
        addr_t addr,
        RefPtr<SharedString>& file) const
{
    LineByAddr::const_iterator i = lineByAddr_.lower_bound(addr);

    if (i == lineByAddr_.end() || i->first != addr)
    {
        if (i == lineByAddr_.begin())
        {
            return 0;
        }

        --i;
    }

    file = i->second.first;
    return i->second.second;
}
*/


void
Stab::Descriptor::Descriptor::set_observer_events(Events* events)
{
    observer_ = events;
}


void Stab::Descriptor::on_section(const char* section)
{
    if (observer_)
    {
        observer_->on_section(section);
    }
}


void
Stab::Descriptor::on_begin(
    const char* section,
    size_t size,
    size_t start,
    SharedString* msg)
{
    startIndex_ = start;

    if (observer_)
    {
        observer_->on_begin(msg ? *msg : this->name(), section, size);
    }
}


bool Stab::Descriptor::on_stab(
        size_t          index,
        const stab_t&   stab,
        const char*     str,
        size_t          strLen)
{
    if (observer_)
    {
        assert(index >= startIndex_);
        size_t delta = index - startIndex_;
        return observer_->on_stab(delta, stab, str, strLen);
    }
    return true;
}


void Stab::Descriptor::on_done(size_t howManyProcessed)
{
    if (observer_)
    {
        observer_->on_done(howManyProcessed);
    }
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
