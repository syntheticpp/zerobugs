# Stack View
# -------------------------------------------------------------------------
# This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# -------------------------------------------------------------------------
import gobject
import gtk
from base import Base

class Stack(Base):
	"""
	Stack Trace View
	"""
	__gsignals__ = {
		"selection-changed" : ( 
			gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (
				gobject.TYPE_PYOBJECT, 
				gobject.TYPE_PYOBJECT,) )
	}
	def __init__(self, tree):
		Base.__init__(self)
		self.__trace = None
		self.__thread = None

		#the first column holds the stack frame and it is not shown in the view
		self.__model = gtk.ListStore(
			gobject.TYPE_PYOBJECT,
			gobject.TYPE_STRING, 
			gobject.TYPE_STRING, 
			gobject.TYPE_STRING)

		if tree:
			tree.set_model(self.__model)
			renderer = gtk.CellRendererText()
			tree.append_column(gtk.TreeViewColumn("Address", renderer, text=1))
			tree.append_column(gtk.TreeViewColumn("Location", renderer, text=2))
			tree.append_column(gtk.TreeViewColumn("Function", renderer, text=3))
			tree.get_column(0).set_resizable(True)
			tree.get_column(1).set_resizable(True)
			tree.get_selection().connect("changed", self.on_selection_changed)
			tree.get_selection().set_mode(gtk.SELECTION_SINGLE)


	def update(self, event):
		self.__thread = event.thread()
		self.__trace = event.thread().stack_trace()
		model = self.__model
		model.clear()
		for frame in self.__trace.frames():
			iter = model.append()
			model.set_value(iter, 0, frame)
			model.set_value(iter, 1, hex(frame.program_count()))
			sym = frame.function()
			if sym:
				if sym.line():
					s = "%s:%d" % (sym.filename(), sym.line())
				else:
					s = sym.filename()
				model.set_value(iter, 2, s)
				model.set_value(iter, 3, sym.demangle())


	def clear(self):
		self.__model.clear()


	def on_selection_changed(self, sel):
		###get_selected_rows() is not available in Pygtk 2.0
		#model, paths = sel.get_selected_rows()
		#if len(paths):
		#	assert len(paths) == 1
		#	iter = model.get_iter(paths[0])
		model, iter = sel.get_selected()
		if iter:
			frame = model.get_value(iter, 0)
			if frame:
				self.__trace.select_frame(frame.index())
				self.emit('selection-changed', self.__thread, frame)


gobject.type_register(Stack)
