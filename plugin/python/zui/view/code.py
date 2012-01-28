# vim: expandtab
# -------------------------------------------------------------------------
# This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# -------------------------------------------------------------------------

from base import Base
import gobject
import gtk
import gtk.gdk
import os
from pixman import PixMan
import zero


class Code(Base):
    """
    Base class for source code and assembly views
    """
    def __init__(self, w):
        Base.__init__(self)
        self.__fname = None
        self.__pixman = PixMan(w)
        self.__currentLine = 0
        #cache the breakpoints in this code view:
        self.__breakpoints = { }
        self.__model = gtk.ListStore(
            gobject.TYPE_OBJECT,
            gobject.TYPE_STRING,
            gobject.TYPE_STRING,
            gobject.TYPE_STRING,
        )
        self.__init_pixbufs(w)
        self.__scrolled = gtk.ScrolledWindow()
        self.__scrolled.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        self.__w = gtk.TreeView(self.__model)
        self.__thread = None
        self.__addr = 0
        self.__scrolled.add(self.__w)
        tree = self.__w
        tree.connect("button_press_event", self.__on_button_press)
        tree.set_headers_visible(False)
        tree.get_selection().set_mode(gtk.SELECTION_SINGLE)
        # for the arrow at the current program counter:
        renderer = gtk.CellRendererPixbuf()
        tree.append_column(gtk.TreeViewColumn("", renderer, pixbuf=0))
        renderer = gtk.CellRendererText()
        tree.append_column(gtk.TreeViewColumn("", renderer, text=1))
        tree.append_column(gtk.TreeViewColumn("", renderer, text=2))
        tree.append_column(gtk.TreeViewColumn("", renderer, text=3))

    def __init_pixbufs(self, w):
        self.__arrow = self.__pixman.get_pixbuf("arrow")
        self.__stop = self.__pixman.get_pixbuf("brkpnt")

    def widget(self):
        return self.__scrolled

    def filename(self):
        return self.__fname

    def set_current_line(self, addr, line):
        iter = self.__model.iter_nth_child(None, self.__currentLine)
        try:
            b = self.__breakpoints[self.__addr]
            self.__model.set_value(iter, 0, self.__stop)
        except KeyError:
            self.__model.set_value(iter, 0, None)
        self.__currentLine = line


    def read(self, thread, addr, sym):
        if thread.process():
            breakpoints = thread.process().breakpoints()
        else:
            breakpoints = thread.breakpoints()
        #note: template method design pattern
        self.read_file(thread, addr, sym, breakpoints)

    def show(self, thread, addr, sym):
        assert sym
        if sym:
            if self.__fname != sym.filename():
                self.read(thread, addr, sym)
                self.__fname = sym.filename()
            self.set_current_line(addr, sym.line() - 1)

        iter = self.mark_current_line()
        if thread.program_count() == addr:
            pixbuf = self.__arrow
            try:
                b = self.__breakpoints[addr]
                pixbuf = self.__pixman.compose(self.__stop, pixbuf)
            except KeyError:
                pass
            self.__model.set_value(iter, 0, pixbuf)
        self.__thread = thread
        self.__addr = addr
        self.__sym = sym


    def mark_current_line(self):
        line = self.__currentLine
        iter = self.__model.iter_nth_child(None, line)
        tree = self.__w
        path = self.__model.get_path(iter)
        try:
            #available in Pygtk 2.2 and above
            tree.set_cursor_on_cell(path)
        except:
            tree.set_cursor(path, None, False)
        return iter

    def clear(self):
        self.__model.clear()

    def get_token(self, iter):
        "Get the token under cursor in the code view"
        return ""

    def __on_button_press(self, w, event):
        if event.button == 3:
            self.__popup_menu(event)

    def __popup_menu(self, event):
        x = int(event.x)
        y = int(event.y)
        path = self.__w.get_path_at_pos(x, y)[0]
        if path:
            iter = self.__model.get_iter(path)
            if iter:
                #get a tuple of addresses that correspond
                # to the source (or asm) line at event position
                addresses = self.get_addresses(iter)
                #get the word at event position
                token = self.get_token(iter)
                menu = self.__make_contextual_menu(addresses, token)
                if menu:
                    menu.popup(None, 
                                None, 
                                None, 
                                event.button, 
                                event.get_time())

    def __make_contextual_menu(self, addrs, token):
        menu = gtk.Menu()
        if self.__thread:
            self.__add_breakpoint_menu_items(menu, self.__thread, addrs)
        menu.show_all()
        return menu

    def __add_breakpoint_menu_items(self, menu, thread, addresses):
        breakpoints_to_enable = []
        breakpoints_to_disable = []
        breakpoints_to_remove = []
        breakpoints_to_set = [] 

        for a in addresses:
            allBreakpoints = thread.process().breakpoints(a)
            for b in allBreakpoints:
                breakpoints_to_remove.append(b)
                if b.is_enabled():
                    breakpoints_to_disable.append(b)
                else:
                    breakpoints_to_enable.append(b)
            if len(allBreakpoints) == 0:
                breakpoints_to_set.append(a)
        if len(breakpoints_to_set):
            self.__add_set_breakpoint_item(menu, 
                                        "Set Breakpoint", 
                                        thread.process(),
                                        breakpoints_to_set)
            self.__add_set_breakpoint_item(menu, 
                                        "Set Thread Breakpoint", 
                                        thread,
                                        breakpoints_to_set)
        
        if len(breakpoints_to_remove):
            self.__add_remove_breakpoint_item(menu, breakpoints_to_remove)

        # todo: breakpoints_to_enable, to_disable, to_remove


    def __add_set_breakpoint_item(self, menu, label, thread, breakpoints):
        item = gtk.ImageMenuItem("gtk-dialog-error")
        item.get_child().set_label(label)
        menu.append(item)
        if len(breakpoints) > 1:
            subMenu = gtk.Menu()
            item.set_submenu(subMenu)
            for a in breakpoints:
                print "__add_set_breakpoint_item",hex(a)
                subItem = gtk.MenuItem(hex(a))
                subMenu.append(subItem)
                subItem.connect("activate", self.__set_breakpoint, thread, a)
        elif len(breakpoints):
            a = breakpoints[0]
            item.connect("activate", self.__set_breakpoint, thread, a)


    def __add_remove_breakpoint_item(self, menu, breakpoints):
        assert len(breakpoints)
        item = gtk.ImageMenuItem("todo")
        item.get_child().set_label("Remove Breakpoint")
        menu.append(item)
        if len(breakpoints) == 1:
            b = breakpoints[0]
            item.connect("activate", self.__remove_breakpoint, b)
        else:
            subMenu = gtk.Menu()
            item.set_submenu(subMenu)
            for b in breakpoints:
                subItem = gtk.MenuItem(hex(b.addr()))
                subMenu.append(subItem)
                subItem.connect("activate", self.__remove_breakpoint, b)


    def __set_icon_at_line(self, lineNum, pixbuf):
        assert lineNum
        iter = self.__model.iter_nth_child(None, lineNum - 1)
        self.__model.set_value(iter, 0, pixbuf)
    
    
    def set_icon_at_symbol(self, sym, pixbuf):
        self.__set_icon_at_line(self.symbol_line(sym), pixbuf)


    def __set_breakpoint(self, item, thread, addr):
        thread.set_breakpoint(addr)
        self.__breakpoints[addr] = thread.breakpoints(addr)[0]
        sym = thread.symbols().lookup(addr)
        if sym:
            if sym.filename() == self.filename():
                self.set_icon_at_symbol(sym, self.__stop)


    def __remove_breakpoint(self, item, brkpnt):
        a = brkpnt.addr()
        sym = brkpnt.symbol()
        brkpnt.remove()
        if not len(self.__thread.process().breakpoints(a)):
            del self.__breakpoints[a]
            if a == self.__thread.program_count():
                self.set_icon_at_symbol(sym, self.__arrow)
            else:
                self.set_icon_at_symbol(sym, None)


    def symbol_line(self, sym):
        return sym.line()
