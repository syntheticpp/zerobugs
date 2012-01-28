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


class Threads(Base):
	"""
	Threads View, displays the debuggee threads
	"""
	def __init__(self, tree):
		Base.__init__(self)
		self.__selected = False
		self.__debugger = None
		self.__model = gtk.ListStore(
			gobject.TYPE_PYOBJECT,
			gobject.TYPE_STRING,
			gobject.TYPE_STRING,
		)
		if tree:
			self.__view = tree
			tree.set_model(self.__model)
			renderer = gtk.CellRendererText()
			tree.append_column(gtk.TreeViewColumn("PID", renderer, text=1))
			tree.append_column(gtk.TreeViewColumn("Thread ID", renderer, text=2))
			tree.get_column(0).set_resizable(True)

			tree.connect("cursor-changed", self.on_thread_selected)


	def update(self, event):
		if self.__selected:
			self.__selected = False
		else:
			thread = event.thread()

			if thread is None:
				self.clear()
			else:
				self.__debugger = thread.debugger()
				process = thread.process()
				if process:
					model = self.__model
					model.clear()
					for t in process.threads():
						#print '-----',t.lwpid(),'-----'
						iter = model.append()
						model.set_value(iter, 1, t.lwpid())
						model.set_value(iter, 2, t.id())


	def clear(self):
		self.__model.clear()


	def on_thread_selected(self, view):
		if not self.__selected:
			self.__selected = True
			(model, iter) = view.get_selection().get_selected()
			if iter:
				pid = long(model.get_value(iter, 1))
				tid = long(model.get_value(iter, 2))
				self.__debugger.select_thread(pid, tid)

