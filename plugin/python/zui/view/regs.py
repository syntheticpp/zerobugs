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


class Registers(Base):
	"""
	Displays the contents of CPU registers 
	"""
	def __init__(self, tree):
		Base.__init__(self)
		self.__model = gtk.ListStore(
			gobject.TYPE_PYOBJECT,
			gobject.TYPE_STRING,
			gobject.TYPE_STRING,
		)
		model_sort = gtk.TreeModelSort(self.__model)
		#start in a sorted state
		model_sort.set_sort_column_id(1, gtk.SORT_ASCENDING)

		if tree:
			renderer = gtk.CellRendererText()

			tree.set_model(model_sort)
			tree.append_column(gtk.TreeViewColumn("Name", renderer, text=1))
			tree.append_column(gtk.TreeViewColumn("Value", renderer, text=2))
			tree.get_column(0).set_resizable(True)
			tree.get_column(0).set_clickable(True)
			tree.get_column(1).set_clickable(True)
			tree.get_column(0).set_sort_column_id(1)
			tree.get_column(1).set_sort_column_id(2)

			#start in a sorted state
			tree.get_column(0).set_sort_order(gtk.SORT_ASCENDING)
			tree.get_column(0).set_sort_indicator(True)


	def update(self, event):
		if event.thread():
			model = self.__model
			model.clear()

			regs = event.thread().regs()
			for name in regs:
				iter = model.append()
				model.set_value(iter, 1, name)
				val = regs[name]
				try:
					model.set_value(iter, 2, hex(val))
				except:
					model.set_value(iter, 2, val)
		else:
			self.clear()


	def clear(self):
		self.__model.clear()

