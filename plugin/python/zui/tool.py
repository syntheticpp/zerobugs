# -------------------------------------------------------------------------
# This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# -------------------------------------------------------------------------
import gtk


class Bar(object):
    def __init__(self, toolbar):
        self.__toolbar = toolbar


    def add_space(self):
        try:
            self.__toolbar.insert(gtk.SeparatorToolItem(), -1)
        except:
            self.__toolbar.append_space()


    def add_widget(self, widget, tip_text, private_text):
        try:
            toolitem = gtk.ToolItem()
            toolitem.add(widget)
            toolitem.set_expand(False)
            toolitem.set_homogeneous(False)
            toolitem.set_tooltip(gtk.Tooltips(), tip_text, private_text)
            self.__toolbar.insert(toolitem, -1)
        except:
            self.__toolbar.append_element(
                gtk.TOOLBAR_CHILD_WIDGET,            
                widget,
                tip_text,
                private_text,
                None,
                None,
                None,
                None)


    def add_stock(self, name, stock_id, label, tip_text, important, cb):
        try:
            toolitem = gtk.ToolButton(stock_id)
            toolitem.connect('clicked', cb)
            toolitem.set_tooltip(gtk.Tooltips(), tip_text, tip_text)
            toolitem.set_name('tool_' + name)
            toolitem.set_label(label)
            toolitem.set_is_important(important)
            self.__toolbar.insert(toolitem,-1)
        except:
            icon = gtk.Image()
            icon.set_from_stock(stock_id, gtk.ICON_SIZE_SMALL_TOOLBAR)
            w = self.__toolbar.insert_item(
                label,
                tip_text,
                tip_text,
                icon,
                cb,
                None,
                -1)
            w.set_name('tool_' + name)

