# -------------------------------------------------------------------------
# This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# -------------------------------------------------------------------------
import gtk
import os.path
import zero


def set_filename(fileButton, process):
	name = process.name()
	basename = os.path.basename(name)
	btn = fileButton.get_children()[0]
	box = btn.get_children()[0]
	image = box.get_children()[0]
	label = box.get_children()[1]
	iconName = "gnome-mime-application-x-executable"
	if process.origin() == zero.Process.Origin.Core:
		iconName = "gnome-mime-application-x-core"

	image.set_from_icon_name(iconName, gtk.ICON_SIZE_MENU)
	label.set_text(basename)


class FileOpen(object):
	"""
	Implements the logic associated with File Chooser Dialog
	"""
	def __init__(self, fileChooser, filters = ()):
		self.__filters = filters

		box = gtk.HBox()
		try:
			fileChooser.set_extra_widget(box)
			fileChooser.connect("file-activated", self.on_file_activated)
			fileChooser.connect("update-preview", self.on_update_preview, box)
			for f in filters:
				fileChooser.set_filter(f.filter)
				f.connect("update_preview", self.on_update_preview)
		except:
			fileChooser.connect("response", self.__on_response)


	def on_update_preview(self, fileChooser, preview):
		#clear the preview:
		for w in preview.get_children():
			preview.remove(w)
		#pass the preview to filters
		filename = fileChooser.get_preview_filename()
		if filename:
			for f in self.__filters:
				if f.update_preview(preview, filename):
					break


	def on_file_activated(self, chooser):
		filename = chooser.get_filename()
		if filename:
			for f in self.__filters:
				if f.open(filename):
					chooser.hide()
					break

	def __on_response(self, dlg, respID):
		if respID == gtk.RESPONSE_OK:
			self.on_file_activated(dlg)
		else:
			dlg.hide()
