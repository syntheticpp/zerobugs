#
# Base classes
#
# -------------------------------------------------------------------------
# This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# -------------------------------------------------------------------------
import gobject

class Base(gobject.GObject):
	def __init__(self):
		gobject.GObject.__init__(self)


class Composite(Base):
	"""
	A collection of views
	"""
	def __init__(self):
		Base.__init__(self)
		self.__views = []

	def add(self, view):
		self.__views.append(view)

	def update(self, event):
		for v in self.__views:
			v.update(event)

	def clear(self):
		for v in self.__views:
			v.clear()

