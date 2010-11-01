#ifndef ENTRY_H__65C88402_4396_4ECB_8D8C_1E7E243F2CD8
#define ENTRY_H__65C88402_4396_4ECB_8D8C_1E7E243F2CD8
//
// $Id: text_entry.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <deque>
#include "gtkmm/combo.h"
#include "gtkmm/signal.h"
#include "zdk/check_ptr.h"
#include "zdk/export.h"
#include "zdk/persistent.h"
#include "zdk/properties.h"
#include "zdk/ref_ptr.h"
#include "zdk/zobject_impl.h"


class DialogBox;
class TextEntry;

/**
 * History of text entered in a combo box; history
 * shows up in the drop-down list and it is saved
 * with the debugger settings.
 */
class ZDK_LOCAL DropList : public ZObjectImpl<>, public Persistent
{
public:
    DECLARE_UUID("a758b85d-4856-4db6-885f-6809fac2a37b")

BEGIN_INTERFACE_MAP(DropList)
    INTERFACE_ENTRY(DropList)
    INTERFACE_ENTRY(Streamable)
    INTERFACE_ENTRY(InputStreamEvents)
END_INTERFACE_MAP()

    typedef std::deque<std::string> container_type;

    static const size_t max_items = 100;

    explicit DropList(const char* name, TextEntry* = NULL);

    virtual ~DropList() throw();

    void copy_items(const DropList& other)
    {
        items_ = other.items_;
    }

    const container_type& items() const { return items_; }
    container_type& items() { return items_; }

    void add(const std::string&);

    Mutex& mutex() { return mutex_; }

private:
    void on_string(const char*, const char*);

    void on_object_end();

    /*** Streamable ***/
    uuidref_t uuid() const { return _uuid(); }

    size_t write(OutputStream*) const;

private:
    container_type items_;
    TextEntry* owner_;
    mutable Mutex mutex_;
};



/**
 * Extends the base class with a key-pressed event handler
 * that handles the Enter and Escape keys. Hitting <Enter>
 * inside the entry field has the same effect as clicking
 * the OK button in the enclosing dialog; Escape yields the
 * same effect as clicking Cancel, or the X button.
 */
class ZDK_LOCAL TextEntry : public Gtk::Combo
{
public:
    typedef SigC::Signal2<bool, TextEntry*, std::string> AutoCompleteSignal;

    /**
     * Construct an entry giving it the dialog box
     * that parents it, a collection of properties
     * and the name by which to lookup the drop list
     * history inside the properties.
     */
    TextEntry(DialogBox&, WeakPtr<Properties>, const char* name);

    /**
     * Use this ctor if you don't want <Enter> to close
     * the dialog box -- normally, pressing the <Enter>
     * key in the entry is equivalent to clicking the
     * dialog's OK button.
     */
    TextEntry(WeakPtr<Properties>, const char* name);

    virtual ~TextEntry();

    void set_text(const std::string&);

    unsigned int get_text_length() const;

    void select_region(int start, int end);

    std::string get_text(bool addToHistory = true, bool strip = true);

    SigC::Signal0<void> activate;
    AutoCompleteSignal auto_complete;

#ifdef GTKMM_2
    SigC::Signal0<void>& signal_activate()
    {
        return activate;
    }
#endif

    void grab_focus();

    DialogBox& dialog() { return *CHKPTR(dialog_); }

    void add_to_list(const std::string&);

    DropList* drop_list() { return dropList_.get(); }

    void clear();

    void set_may_auto_complete(bool mayAutoComplete)
    {
        mayAutoComplete_ = mayAutoComplete;
    }

    bool allow_empty() const { return allowEmpty_; }

    void set_allow_empty(bool allowEmpty)
    {
        allowEmpty_ = allowEmpty;
    }

private:
    void populate_from(Properties&);

    void do_activate();

    /**
     * Helper called by ctor
     */
    void init(const char* name);

    void set_history_strings();

    void on_changed();

    int on_idle_auto_complete();

private:
    DialogBox*          dialog_;
    WeakPtr<Properties> prop_;
    RefPtr<DropList>    dropList_;
    SigC::Connection    idle_;
    std::string         prev_;
    bool                mayAutoComplete_;
    bool                allowEmpty_;
};

#endif // ENTRY_H__65C88402_4396_4ECB_8D8C_1E7E243F2CD8
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
