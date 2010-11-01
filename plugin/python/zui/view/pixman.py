# -------------------------------------------------------------------------
# This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# -------------------------------------------------------------------------
import gtk
import os
import pixmaps


class PixMan(object):
	"""
	Pixbuf Manager
	"""
	def __init__(self, w):
		self.__pixbufs = { }
		self.__w = w
		self.create_from_data("arrow", pixmaps.right_arrow)
		self.create_from_file("brkpnt", "stop.png")


	def create_from_data(self, name, pixdata, width = 20, height = 20):
		try:
			pixbuf = gtk.gdk.pixbuf_new_from_xpm_data(pixdata)
		except:
			colormap = self.__w.get_colormap()
			pix, mask = gtk.gdk.pixmap_colormap_create_from_xpm_d(
				None,
				colormap, 
				None, 
				pixdata)
			pixbuf = gtk.gdk.Pixbuf(
				gtk.gdk.COLORSPACE_RGB,
				True,
				8,
				20,
				20)
			pixbuf.get_from_drawable(mask, colormap, 0, 0, 0, 0, -1, -1)
		self.__pixbufs[name] = pixbuf

	
	def create_from_file(self, name, filename):
		topPath = os.environ["ZTOP"]
		try:
			#from Open Clipart project:
			filename = topPath + "/view/" + filename
			pixbuf = gtk.gdk.pixbuf_new_from_file(filename)
			self.__pixbufs[name] = pixbuf
		except:
			pass


	def get_pixbuf(self, name):
		#return self.__pixbufs[name]
		try:
			return self.__pixbufs[name]
		except KeyError:
			return None
			
	
	def compose(self, first, second):
		"Compose two pixbufs"
		pixbuf = first.copy()
		second.composite(
			pixbuf, 0, 0, 
			first.props.width, first.props.height,
			0, 0, 
			1.0, 1.0, #scale
			gtk.gdk.INTERP_HYPER, 
			127)
		return pixbuf

