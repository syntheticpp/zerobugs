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
#include <iostream>
#include <vector>
#include <boost/tokenizer.hpp>
#include "gtkmm/label.h"
#include "gtkmm/image.h"
#include "gtkmm/pixmap.h"
#include "line_wrap.h"
#include "message_box.h"

using namespace std;
using namespace Gtk;


////////////////////////////////////////////////////////////////
MessageBox::MessageBox
(
    const string& message,
    ButtonID btns,
    const char* title,
    const char* pixmap[]
)
  : DialogBox(btns, title)
  , hbox_(manage(new HBox()))
  , label_(0)
{
    get_vbox()->add(*hbox_);
    get_hbox()->set_border_width(10);
    get_hbox()->set_spacing(5);

    if (pixmap)
    {
        // Hack: This class has been originally written to work
        // with the lowest common denominator between gtk-1.2 and 2.x,
        // and thus used pixmaps;
        // however, Gtk stock icons look nicer, and integrate
        // better with the general look-and-feel of the desktop;
        // the hack is to pass in stock ids where a pixmap array
        // of chars is expected.
        if (pixmap[0] && strncmp(pixmap[0], "gtk-", 4) == 0)
        {
            StockID id(pixmap[0]);
            Image* img = manage(new Image(id, ICON_SIZE_DIALOG));
            get_hbox()->pack_start(*img, false, false);
        }
        else
        {
            Pixmap* p = manage(new Pixmap(pixmap));
            get_hbox()->pack_start(*p, false, false);
        }
    }

    // todo: determine max width based on font metrics,
    // or make it a parameter
    static const size_t maxWrapWidth = 120;

    typedef boost::char_separator<char> Delim;
    typedef boost::tokenizer<Delim> Tokenizer;

    string msg;
    if (!message.empty())
    {
        Tokenizer tok(message, Delim("\n"));
        for (Tokenizer::const_iterator i = tok.begin();;)
        {
            msg += line_wrap(*i, maxWrapWidth);
            if (++i == tok.end())
            {
                break;
            }
            msg += "\n";
        }
    }
    label_ = manage(new Label(msg));
    get_hbox()->pack_start(*label_, false, false);
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
