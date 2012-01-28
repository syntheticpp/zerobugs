#ifndef SCOPE_HELPERS_H__53591A52_12D5_43D8_9382_0B7D01569E05
#define SCOPE_HELPERS_H__53591A52_12D5_43D8_9382_0B7D01569E05
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

#include <boost/utility.hpp> // noncopyable

/**
 * Uses resource-acquisition-is-initialization idiom to
 * temporarily freeze a Gtk:: widget.
 */
template<typename T>
class ScopedFreeze : boost::noncopyable
{
public:
    explicit ScopedFreeze(T& t) : t_(t)
    {
#ifdef GTKMM_2
        if (Glib::RefPtr<Gdk::Window> w = t_.get_window())
        {
            w->freeze_updates();
        }
#else
        t_.freeze();
#endif
    }

    ~ScopedFreeze() throw()
    {
        try
        {
#ifdef GTKMM_2
            if (Glib::RefPtr<Gdk::Window> w = t_.get_window())
            {
                w->thaw_updates();
            }
#else
            const_cast<T&>(t_).thaw();
#endif
        }
        catch (...)
        {
        }
    }

protected:
    void* operator new(size_t);
    void operator delete(void*) {}

private:
    T& t_;
};


/**
 * Uses resource-acquisition-is-initialization idiom to
 * temporarily change the sensitivity of a Gtk::Widget.
 */
template<typename T, bool F = false>
class ScopedSensitive : boost::noncopyable
{
public:
    explicit ScopedSensitive(T& t) : t_(t)
    {
        t_.set_sensitive(F);
    }

    ~ScopedSensitive() throw()
    {
        t_.set_sensitive(!F);
    }

protected:
    void* operator new(size_t);
    void operator delete(void*) {}

private:
    T& t_;
};

#endif // SCOPE_HELPERS_H__53591A52_12D5_43D8_9382_0B7D01569E05
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
