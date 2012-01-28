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

#include <iomanip>
#include <fstream>
#include <map>
#include <sstream>
#include "zdk/check_ptr.h"
#include "zdk/debug_sym.h"
#include "zdk/zobject_scope.h"
#include "dharma/symbol_util.h"
#include "generic/temporary.h"
#include "debugger_engine.h"

using namespace std;


////////////////////////////////////////////////////////////////
/*** BEGIN Disassembler Helpers ***/
namespace
{
    /**
     * Helper class that looks up the source code that
     * corresponds to addresses in the assembly code.
     */
    CLASS ListingHelper
        : public Disassembler::SourceCallback
        , public Disassembler::SymbolCallback
        , private EnumCallback<DebuggerPlugin*>
        , private EnumCallback2<SharedString*, size_t>
    {
    public:
        virtual ~ListingHelper() throw() {}

        ListingHelper(Debugger& debugger, Symbol* start, SymbolMap* syms)
            : debugger_(debugger)
            , start_(start)
            , nextLine_(0)
            , nextAddr_(0)
            , symbols_(syms)
        {}

        bool init_source_info();

        /**
         * Disassembler::SourceCallback interface
         */
        vector<string>* notify(addr_t, size_t);

        /**
         * Disassembler::SymbolCallback interface.
         * Lookup symbol by address. If an exact match was
         * requested it means that we are interested in symbols
         * with zero offset (symbols with non-zero offsets are
         * artifacts that we create and do not have a corresponding
         * entry in the ELF symbol tables).
         */
        const Symbol* notify(addr_t addr, bool exact);

    private:
        /**
         * EnumCallback<DebuggerPlugin*>
         */
        void notify(DebuggerPlugin*);

        void notify(SharedString* file, size_t line);

        void read_file(SharedString*);

        bool inline output_line(size_t);

        void output_info(SharedString*, size_t);

        void reset_current_func() { func_ = sym_, nextLine_ = 0; }

    private:
        typedef vector<DebugInfoReader*> DebugInfo;

        typedef map<RefPtr<SharedString>, vector<string> > SrcMap;

        Debugger&       debugger_;

        RefPtr<Symbol>  start_; // start symbol
        RefPtr<Symbol>  sym_;   // last lookup symbol
        RefPtr<Symbol>  func_;  // current function
        vector<string>  snippet_;
        SrcMap          srcMap_;
        DebugInfo       dinfo_;
        size_t          lastLine_;
        size_t          nextLine_;
        set<size_t>     emitted_;
        size_t          nextAddr_;
        RefPtr<SharedString> lastFile_;
        RefPtr<SymbolMap> symbols_;
    };
}


////////////////////////////////////////////////////////////////
bool ListingHelper::init_source_info()
{
    bool result = false;
    debugger_.enum_plugins(this);

    if (!dinfo_.empty())
    {
        result = true;
    }
    return result;
}


////////////////////////////////////////////////////////////////
void ListingHelper::notify(DebuggerPlugin* p)
{
    DebugInfoReader* dinfo = interface_cast<DebugInfoReader*>(p);
    if (dinfo)
    {
        dinfo_.push_back(dinfo);
    }
}


////////////////////////////////////////////////////////////////
vector<string>* ListingHelper::notify(addr_t addr, size_t len)
{
#if (__GNUC__ < 3)
    try { // force optimizations off, prevent old compiler crash
#endif
    if (!start_)
    {
        return NULL;
    }
    ZObjectScope scope;

    const SymbolTable* symtab = start_->table(&scope);
    if ((symtab != NULL) && (addr < (symtab)->addr()))
    {
    /* #if DEBUG
        clog << hex << addr << dec << ": not in table ";
        clog << CHKPTR(symtab)->filename() << endl;
    #endif */
        return NULL;
    }
    if ((addr == 0) || (addr < nextAddr_))
    {
        return NULL;
    }

    addr -= CHKPTR(symtab)->adjustment();
    snippet_.clear();

    for (DebugInfo::iterator i(dinfo_.begin()); i != dinfo_.end(); ++i)
    {
        CHKPTR(*i)->addr_to_line(symtab, addr, 0, this);

        if (!lastFile_)
        {
            continue;
        }
        addr_t nextAddr = (*i)->next_line_addr(
            symtab, addr, lastFile_.get(), lastLine_);

        if (nextAddr)
        {
            nextAddr_ = nextAddr + CHKPTR(symtab)->adjustment();
        }
    }

    return snippet_.empty() ? NULL : &snippet_;

#if (__GNUC__ < 3)
    } catch (...) { throw; } return NULL;
#endif
}


////////////////////////////////////////////////////////////////
bool inline ListingHelper::output_line(size_t line)
{
    assert(line);
    assert(lastFile_.get());

    const vector<string>& lines = srcMap_[lastFile_];

    // emit source line once
    if (line <= lines.size() && emitted_.insert(line).second)
    {
        ostringstream os;

        os << setw(8) << line << ' ' << lines[line - 1];
        snippet_.push_back(os.str());
        return true;
    }
    return false;
}


////////////////////////////////////////////////////////////////
void
ListingHelper::output_info(SharedString* filename, size_t line)
{
    ostringstream os;

    os << filename << ':' << line;
    snippet_.push_back(os.str());
}


////////////////////////////////////////////////////////////////
void ListingHelper::notify(SharedString* filename, size_t line)
{
    if (line)
    {
        read_file(filename);
        lastLine_ = line;

        if (nextLine_ == 0)
        {
            output_info(filename, line);
            output_line(line);
            nextLine_ = line + 1;
        }
        else
        {
            if (line < nextLine_)
            {
                output_line(line);
            }
            else if (!emitted_.count(line))
            {
                for (; nextLine_ <= line; ++nextLine_)
                {
                    output_line(nextLine_);
                }
                assert(nextLine_ == line + 1);
            }
        }
    }
}


////////////////////////////////////////////////////////////////
void ListingHelper::read_file(SharedString* filename)
{
    SrcMap::iterator i = srcMap_.find(filename);
    if (i == srcMap_.end())
    {
        vector<string> lines;
        ifstream f(CHKPTR(filename)->c_str());

        // assume lines in the source code are not wider
        vector<char> buf(4096);

        while (f)
        {
            f.getline(&buf[0], buf.size());
            lines.push_back(string(&buf[0]));
        }
        srcMap_.insert(i, make_pair(filename, lines));
    }
    if (!filename->is_equal2(lastFile_.get()))
    {
        lastFile_ = filename;
        nextLine_ = 0;
        emitted_.clear();
    }
}


////////////////////////////////////////////////////////////////
const Symbol* ListingHelper::notify(addr_t addr, bool exact)
{
    if (exact && sym_
              && sym_->offset() == 0
              && sym_->addr() <= addr)
    {
        // optimization: short-circuit the lookup
        if (sym_->addr() == addr)
        {
            reset_current_func();

            return sym_.get();
        }
        return NULL;
    }
    if (symbols_.is_null())
    {
        return NULL;
    }
    sym_ = symbols_->lookup_symbol(addr);

    if (exact)
    {
        if (sym_ && sym_->offset() == 0)
        {
            reset_current_func();
        }
        else
        {
            sym_.reset();
        }
    }

    return sym_.get();
}
/*** END Disassembler Helpers ***/


////////////////////////////////////////////////////////////////
size_t DebuggerEngine::disassemble (
    Thread* thread,
    Symbol* start,
    size_t  howMany,     // how many addresses to disassemble?
    bool    withSource,  // mix source with assembly
    const uint8_t* membuf,
    Disassembler::OutputCallback* callback)
{
    assert(start);
    assert(thread);

    ZObjectScope scope;
    SymbolTable* table = start->table(&scope);
    if (!table)
    {
        return 0;
    }
    if (howMany == 0)
    {
        return 0;
    }
    if (disasm_.is_null())
    {
        cerr << "*** Warning: No disassembler plugin found.\n";
    }
    else
    {
        ListingHelper helper(*this, start, thread->symbols());

        if (withSource)
        {
            withSource = helper.init_source_info();
        }

        if (table->addr() == 0)
        {
            membuf = NULL;
        }
        return disasm_->disassemble(
            start->addr(),
            table->adjustment(),
            membuf,
            howMany,
            table->filename(),
            callback,
            &helper,
            withSource ? &helper : 0);
    }
    return 0;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
