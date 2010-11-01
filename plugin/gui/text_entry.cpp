//
// $Id: text_entry.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <cassert>
#include <iostream>
#include <gdk/gdkkeysyms.h>
#include "generic/temporary.h"
#include "gtkmm/connect.h"
#include "gtkmm/flags.h"
#include "gtkmm/label.h"
#include "gtkmm/main.h"
#include "assert_thread.h"
#include "dialog_box.h"
#include "text_entry.h"
#include "utils.h"

using namespace std;


////////////////////////////////////////////////////////////////
TextEntry::TextEntry
(
    DialogBox& dlg,
    WeakPtr<Properties> prop,
    const char* name
)
    : dialog_(&dlg)
    , prop_(prop)
    , dropList_(new DropList(name, this))
    , mayAutoComplete_(false)
    , allowEmpty_(false)
{
    init(name);
}


////////////////////////////////////////////////////////////////
TextEntry::TextEntry(WeakPtr<Properties> prop, const char* name)
    : dialog_(0)
    , prop_(prop)
    , dropList_(new DropList(name, this))
    , mayAutoComplete_(false)
    , allowEmpty_(false)
{
    init(name);
}


////////////////////////////////////////////////////////////////
TextEntry::~TextEntry()
{
    if (idle_.connected())
    {
        idle_.disconnect();
    }
}


////////////////////////////////////////////////////////////////
void TextEntry::populate_from(Properties& prop)
{
    Lock<Mutex> lock(dropList_->mutex());
    prop_ = &prop;

    const char* name = dropList_->name();

    ZObject* obj = prop_->get_object(name);

    if (RefPtr<DropList> dropList = interface_cast<DropList*>(obj))
    {
        dropList_->copy_items(*dropList);
        set_history_strings();
    }
    if (property_value_in_list() && !dropList_->items().empty())
    {
        get_entry()->set_text(dropList_->items().front());
    }
}


////////////////////////////////////////////////////////////////
void TextEntry::init(const char* name)
{
    Lock<Mutex> lock(dropList_->mutex());
    assert(get_entry());

    get_entry()->set_flags(Gtk_FLAG(CAN_DEFAULT) | Gtk_FLAG(CAN_FOCUS));

    // do not show the drop down list when hitting <Enter>
    disable_activate();

    set_case_sensitive(true);
    set_use_arrows(false);

    assert(name);

    if (RefPtr<Properties> prop = prop_.ref_ptr())
    {
        populate_from(*prop);
    }
    Gtk_CONNECT(get_entry(), activate, this->activate.slot());
    if (dialog_)
    {
        Gtk_CONNECT_0(get_entry(), activate, this, &TextEntry::do_activate);
    }
    Gtk_CONNECT_0(get_entry(), changed, this, &TextEntry::on_changed);
}


////////////////////////////////////////////////////////////////
void TextEntry::set_history_strings()
{
    Lock<Mutex> lock(dropList_->mutex());
    const deque<string>& items = dropList_->items();

    if (!items.empty())
    {
        set_popdown_strings(items);
        get_entry()->set_text("");
    }
}


////////////////////////////////////////////////////////////////
void TextEntry::do_activate()
{
    assert_ui_thread();
    if (dialog_)
    {
        dialog_->on_button(DialogBox::btn_ok);
    }
}


////////////////////////////////////////////////////////////////
void TextEntry::set_text(const string& text)
{
    Lock<Mutex> lock(dropList_->mutex());
    // prevent auto-completion from kicking in if the text
    // is being set programmatically
    Temporary<bool> setFlag(mayAutoComplete_, false);

    CHKPTR(get_entry())->set_text(text);
    prev_ = text;

    if (property_value_in_list())
    {
        const DropList::container_type& items = dropList_->items();
        if (find(items.begin(), items.end(), text) == items.end())
        {
            add_to_list(text);
        }
    }
}


////////////////////////////////////////////////////////////////
void TextEntry::add_to_list(const string& text)
{
    Lock<Mutex> lock(dropList_->mutex());
    dropList_->add(text);

    assert(dropList_->name());
    if (RefPtr<Properties> prop = prop_.ref_ptr())
    {
        prop->set_object(dropList_->name(), dropList_.get());
    }

    set_popdown_strings(dropList_->items());
}


////////////////////////////////////////////////////////////////
string TextEntry::get_text(bool addToHistory, bool stripSpace)
{
    Lock<Mutex> lock(dropList_->mutex());
    string text = CHKPTR(get_entry())->get_text();

    if (stripSpace)
    {
        // strip leading spaces
        while (!text.empty() && isspace(text[0]))
        {
            text.erase((size_t)0, (size_t)1);
        }
        // strip trailing spaces
        while (size_t n = text.size())
        {
            if (isspace(text[--n]))
            {
                text.resize(n);
            }
            else
            {
                break;
            }
        }
    }
    if (addToHistory)
    {
        add_to_list(text);
    }
    return text;
}


////////////////////////////////////////////////////////////////
unsigned int TextEntry::get_text_length() const
{
    Lock<Mutex> lock(dropList_->mutex());
    assert(get_entry());
    return get_entry()->get_text_length();
}


////////////////////////////////////////////////////////////////
void TextEntry::select_region(int start, int end)
{
    assert_ui_thread();
    assert(get_entry());
    get_entry()->select_region(start, end);
}


////////////////////////////////////////////////////////////////
void TextEntry::grab_focus()
{
    assert_ui_thread();
    get_entry()->grab_default();
    get_entry()->grab_focus();
}


////////////////////////////////////////////////////////////////
void TextEntry::on_changed()
{
    Lock<Mutex> lock(dropList_->mutex());

    if (mayAutoComplete_ && !idle_.connected())
    {
        idle_ = GLIB_SIGNAL_IDLE.connect(Gtk_SLOT(this,
                    &TextEntry::on_idle_auto_complete));
    }
}


////////////////////////////////////////////////////////////////
int TextEntry::on_idle_auto_complete()
{
    assert_ui_thread();
    Lock<Mutex> lock(dropList_->mutex());

    if (!mayAutoComplete_)
    {
        return 1; // retry later
    }

    Temporary<bool> setFlag(mayAutoComplete_, false);

    if (!auto_complete(this, prev_))
    {
        prev_.clear();
    }

    return 0; // run once
}


////////////////////////////////////////////////////////////////
void TextEntry::clear()
{
    assert_ui_thread();
    Lock<Mutex> lock(dropList_->mutex());

    Temporary<bool> setFlag(mayAutoComplete_, false);
    CHKPTR(dropList_)->items().clear();
    assert(dropList_->items().empty());

    prev_.clear();

    Gtk::Entry* entry = CHKPTR(get_entry());
#ifdef GTKMM_2
    Gtk::ComboDropDown* list = CHKPTR(get_list());
    do
    {
        entry->set_text("");

        list->children().clear();
        assert(list->children().empty());
    } while (!entry->get_text().empty());
#else
    Gtk::List* list = CHKPTR(get_list());
    do
    {
        entry->set_text("");

        list->items().clear();
        assert(list->items().empty());
    } while (!entry->get_text().empty());
#endif
}


////////////////////////////////////////////////////////////////
DropList::DropList(const char* name, TextEntry* owner)
    : Persistent(name)
    , owner_(owner)
{
}


////////////////////////////////////////////////////////////////
DropList::~DropList() throw()
{
}


////////////////////////////////////////////////////////////////
size_t DropList::write(OutputStream* stream) const
{
    size_t nbytes = 0;

    if (*name())
    {
        container_type::const_iterator i = items_.begin();
        for (; i != items_.end(); ++i)
        {
            nbytes += stream->write_string("", (*i).c_str());
        }
    }
    return nbytes;
}


////////////////////////////////////////////////////////////////
void DropList::on_string(const char*, const char* cstr)
{
    assert(cstr);
    add(cstr);
}


////////////////////////////////////////////////////////////////
void DropList::on_object_end()
{
    Lock<Mutex> lock(mutex_);
    if (owner_)
    {
        string text = owner_->get_text(false);
        owner_->set_popdown_strings(items_);

        if (!text.empty() || owner_->allow_empty())
        {
            owner_->set_text(text);
        }
    }
}


////////////////////////////////////////////////////////////////
void DropList::add(const string& text)
{
    Lock<Mutex> lock(mutex_);
    if (text.empty() && (!owner_ || !owner_->allow_empty()))
    {
        return;
    }

    container_type::iterator iter =
        find(items_.begin(), items_.end(), text);
    if (iter != items_.end()) // item already in list?
    {
        items_.erase(iter);
    }

    items_.push_front(text);
    assert(!items_.empty());

    if (items_.size() > max_items)
    {
        items_.pop_back();
    }
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
