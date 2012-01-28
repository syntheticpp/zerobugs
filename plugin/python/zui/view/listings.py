# -------------------------------------------------------------------------
# This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# -------------------------------------------------------------------------
import os.path
import base
import gtk
from disasm import Disasm
from source import Source


class Listings(base.Composite):
	"""
	Display a collection of source code, disassembly, and mixed listing
	"""
	def __init__(self, book):
		base.Composite.__init__(self)
		self.__current_view = None
		assert book
		self.__book = book


	def add(self, v, filename):
		super(Listings, self).add(v)
		w = v.widget()
		book = self.__book
		shortFileName = os.path.basename(filename)
		try:
			book.append_page(w)
			book.set_tab_label_text(w, shortFileName)
		except:
			#Gtk 2.0
			book.append_page(w, gtk.Label(shortFileName))
		w.show_all()
		book.set_current_page(book.page_num(w))


	def show(self, thread, addr, sym):
		if sym:
			filename = sym.filename()
			line = sym.line()
			self.__show(thread, addr, sym, filename, line)
			try:
				self.__show(thread, addr, sym, filename, line)
			except:
				#force fallback to disassembly:
				line = 0
				#filename = sym.table().filename()
				self.__show(thread, addr, sym, filename, line)
			
	
	def __show(self, thread, addr, sym, filename, line):
		assert(sym)
		view = None
		#find a view that matches the current filename
		for v in self._Composite__views:
			if v.filename() == filename:
				view = v
				book = self.__book
				book.set_current_page(book.page_num(v.widget()))
				break
		#not found, add a new view
		if not view:
			w = self.__book
			if line:
				view = Source(w)
				view.read(thread, addr, sym)
			else:
				view = Disasm(w)
			self.add(view, filename)
		assert view
		view.show(thread, addr, sym)
		self.__current_view = view


	def show_frame(self, stackView, thread, frame):
		self.show(thread, frame.program_count(), frame.function())
	
	
	def update(self, event):
		w = self.__book.get_nth_page(0)
		if w and w.get_name() == "placeholder":
			self.__book.remove_page(0)
		thread = event.thread()
		pc = thread.program_count()
		sym = thread.symbols().lookup(pc)
		self.show(thread, pc, sym)


	def clear(self):
		try:
			while self.__book.get_n_pages():
				self.__book.remove_page(0)
		except:
			while len(self.__book.get_children()):
				self.__book.remove_page(0)
		w = gtk.Frame()
		w.set_name("placeholder")
		w.set_shadow_type(gtk.SHADOW_ETCHED_IN)
		w.show()
		self.__book.add(w)
		self.__book.set_tab_label_text(w, "main.cpp")
		self._Composite__views = []
