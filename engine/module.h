#ifndef MODULE_H__619CF040_4E05_4704_8021_6040BDB68378
#define MODULE_H__619CF040_4E05_4704_8021_6040BDB68378
//
// $Id: module.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "zdk/module.h"
#include "zdk/persistent.h"
#include "zdk/symbol_table.h"
#include "breakpoint_img.h"


/**
 * Implements the module interface. The modules that make up a
 * debugged program can also be saved to a stream (the Streamable
 * and InputStreamEvents interfaces are also implemented here);
 * when a program is loaded into the debugger, the engine can
 * determine whether its modules have been modified, or if they
 * are loaded at a different address, etc.
 */
CLASS ModuleImpl : public RefCountedImpl<Module>, public Persistent
{
public:
    DECLARE_UUID("8f09096c-e2b3-45f9-89c1-c922604f94c9")

BEGIN_INTERFACE_MAP(ModuleImpl)
    INTERFACE_ENTRY(ModuleImpl)
    INTERFACE_ENTRY(Module)
END_INTERFACE_MAP()

    explicit ModuleImpl(SymbolTable&);

    explicit ModuleImpl(const char*);

    virtual ~ModuleImpl() throw();

    void add_breakpoint_image(RefPtr<BreakPointImage>);

    void set_name(const char*);

    void restore(Debugger&, Process&, Module&);

    void erase_breakpoint_images() { breakpoints_.clear(); }

    /**
     * @return the process in whose VM space this module is loaded
     */
    Process* process (ZObjectManager* mgr) const
    {
        if (RefPtr<SymbolTable> symTab = symTable_.ref_ptr())
        {
            return symTab->process(mgr);
        }
        return NULL;
    }

    /*** Module ***/
    virtual SharedString* name() const;

    virtual time_t last_modified() const;

    virtual off_t adjustment() const;

    virtual Platform::addr_t addr() const;

    virtual Platform::addr_t upper() const;

    virtual SymbolTable* symbol_table_list() const;

    virtual size_t enum_units(EnumCallback<TranslationUnit*, bool>*);

    /*** InputStreamEvents ***/
    void on_word(const char*, word_t);

    size_t on_object_begin(InputStream*, const char*, uuidref_t, size_t);

    /*** Streamable ***/
    uuidref_t uuid() const { return _uuid(); }

protected:
    size_t write(OutputStream*) const;

    /**
     * @return the Debugger instance that manages this module
     */
    Debugger* debugger() const;

private:
    typedef ext::hash_map<
        addr_t,
        RefPtr<BreakPointImage> > BreakPointMap;

    void restore_breakpoint(Debugger& debugger,
                            RefPtr<BreakPointImage>,
                            addr_t,
                            Thread*,
                            Module&,
                            SymbolTable*,
                            word_t off);

    WeakPtr<SymbolTable>    symTable_;
    RefPtr<SharedString>    name_;
    addr_t                  addr_;
    addr_t                  upper_;
    time_t                  lastModified_;

    /* user breakpoints inside this module; NOTE that
       information on breakpoints set internally by the
       debugger is not recorded here */
    BreakPointMap           breakpoints_;
};

#endif // MODULE_H__619CF040_4E05_4704_8021_6040BDB68378
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
