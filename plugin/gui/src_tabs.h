#ifndef SRC_TABS_H__A4365FFC_041C_4310_A954_0B7EA4AB4C35
#define SRC_TABS_H__A4365FFC_041C_4310_A954_0B7EA4AB4C35
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: src_tabs.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/persistent.h"
#include "zdk/ref_ptr.h"
#include "zdk/shared_string.h"
#include "zdk/zobject_impl.h"
#include <vector>


/**
 * For persisting and restoring open source tabs.
 */
class SourceTabs : public ZObjectImpl<>, public Persistent
{
public:
    typedef std::vector<RefPtr<SharedString> > container_type;
    typedef container_type::const_iterator const_iterator;


    DECLARE_UUID("7b4a8b70-812c-4826-96b2-da394fb82110")

BEGIN_INTERFACE_MAP(SourceTabs)
    INTERFACE_ENTRY(SourceTabs)
    INTERFACE_ENTRY(Streamable)
    INTERFACE_ENTRY(InputStreamEvents)
END_INTERFACE_MAP()


    explicit SourceTabs(const char* name);

    virtual ~SourceTabs() throw();

    void add_file_name(const RefPtr<SharedString>&);

    const_iterator begin() const { return fileNames_.begin(); }
    const_iterator end() const { return fileNames_.end(); }

private:
    void on_word(const char*, word_t);
    void on_string(const char*, const char*);

    void on_object_end();

    /*** Streamable ***/
    uuidref_t uuid() const { return _uuid(); }

    size_t write(OutputStream*) const;

private:
    container_type fileNames_;
};

#endif // SRC_TABS_H__A4365FFC_041C_4310_A954_0B7EA4AB4C35
