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
import traceback


def add_variable(view, model, iter, var):
	model.set_value(iter, 0, var)
	model.set_value(iter, 1, var.name())
	model.set_value(iter, 2, var.type().name())
	try:
		model.set_value(iter, 3, unicode(var.value(), "utf-8"))
	except:
		#traceback.print_exc()
		pass
	if var.has_children():
		#insert a place-holder
		model.insert_before(iter, None)
	if var.is_expanded():
		view.expand_row(model.get_path(iter), False)
		

class Data(Base):
	"""
	Displays program data, which may be local and/or global 
	variables, data members, expression evaluation results, etc.
	"""
	
	def __init__(self, tree):
		Base.__init__(self)
		self.__model = gtk.TreeStore(
			gobject.TYPE_PYOBJECT, 
			gobject.TYPE_STRING,
			gobject.TYPE_STRING,
			gobject.TYPE_STRING 
		)
		if tree:
			self.__view = tree
			tree.set_model(self.__model)
			renderer = gtk.CellRendererText()
			tree.append_column(gtk.TreeViewColumn("Name", renderer, text=1))
			tree.append_column(gtk.TreeViewColumn("Type", renderer, text=2))
			tree.append_column(gtk.TreeViewColumn("Value", renderer, text=3))
			tree.get_column(0).set_resizable(True)
			tree.get_column(1).set_resizable(True)
			tree.connect("row-collapsed", self.on_row_collapsed)
			tree.connect("row-expanded", self.on_row_expanded)


	def clear(self):
		self.__model.clear()

	
	def show(self, vars):
		self.clear()
		for v in vars:
			model = self.__model
			iter = model.insert_before(None, None)
			add_variable(self.__view, model, iter, v)
		

	def on_row_collapsed(self, treeview, iter, path):
		var = self.__model.get_value(iter, 0)
		var.set_expanded(False)


	def on_row_expanded(self, treeview, iter, path):
		model = self.__model
		var = model.get_value(iter, 0)
		var.set_expanded(True)
		child = model.iter_nth_child(iter, 0)
		if model.get_value(child, 0):
			pass
		else:
			#has place-holder, replace with actual children
			for v in var.children():
				i = model.insert_before(iter, None)
				add_variable(self.__view, model, i, v)
			model.remove(child)


