#ifndef HELP_VIEWER_H__FFE6325B_093D_4255_8176_C5ECA45FA115
#define HELP_VIEWER_H__FFE6325B_093D_4255_8176_C5ECA45FA115
//
// $Id: help_viewer.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "gtkmm/clist.h"
#include "gtkmm/ctree.h"
#include "gtkmm/notebook.h"
#include "dialog_box.h"

class HTMLView;
class Properties;   // zdk/properties.h
class TextEntry;
class Topic;        // private to .cpp


namespace Gtk
{
    class Button;
    class Notebook;
}

typedef std::vector<boost::shared_ptr<Topic> > TopicList;


/**
 * A dialog for navigating the help pages. Help is displayed
 * in a HTML view. The user can select a topic from the Contents
 * tree, or search for a keyword in all topics, then select a
 * topics from the list of matching results.
 */
class ZDK_LOCAL HelpViewer : public DialogBox
{
public:
    explicit HelpViewer(Properties*);
    ~HelpViewer();

    void search(const std::string&);

private:
    /// Looks for a file named topics.hlp in the help directory,
    /// and reads it. Topics.hlp is a text file, the first column
    /// is the topic name, the second is the name of a HTML file
    /// or another .hlp file (if the topic has subtopics).
    void load();

    void construct_contents_tab(Gtk::Notebook&);

    void construct_search_tab(Gtk::Notebook&);

    void construct_index_tab(Gtk::Notebook&);

    void on_select_topic(Gtk::CTree::Row, int);

    void on_select_search_result(Gtk::RowHandle, int, GdkEvent*);

    /// When the return key is pressed, search for the keyword
    /// in all topics and populate a list with the matching topics
    void on_search();

    /// Save current topic in the back pages list, make the
    /// given topic current and display it.
    /// If highlite is true, then search for the keyword that
    /// was types in the search entry and highlite all occurences.
    void navigate(const Topic&, bool highlite = false);

    void navigate_back();

    void navigate_next();

    void activate_menu(Gtk::Widget&);

    std::string on_link(const char* link);

private:
    Properties* prop_;
    Notebook_Adapt* nbook_;
    Gtk::CTree* topicsTree_;
    Gtk::CList* searchList_;
    HTMLView*   htmlView_;
    TextEntry*  search_;
    TopicList   topics_;

    Gtk::Button* back_;
    Gtk::Button* next_;

    const Topic* current_;

    std::vector<const Topic*> backPages_;
    std::vector<const Topic*> nextPages_;
};

#endif// HELP_VIEWER_H__FFE6325B_093D_4255_8176_C5ECA45FA115
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
