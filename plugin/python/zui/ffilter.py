# -------------------------------------------------------------------------
# This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# -------------------------------------------------------------------------
import gtk
import zero


class ELF(object):
	def __init__(self, app, filter):
		self.filter = filter
		self.__app = app
		if filter:
			filter.add_custom(	gtk.FILE_FILTER_FILENAME, 
								self.on_filter, 
								None)


	def __get_hdr(filename):
		try:
			if filename:
				return zero.get_elf_header(filename)
		except:
			pass
		return None
	__get_hdr = staticmethod(__get_hdr)


	def update_preview(self, preview, filename):
		hdr = ELF.__get_hdr(filename)
		if hdr:
			preview.set_spacing(5)
			label = gtk.Label(
				"%s %s %s" % (hdr.klass(), hdr.machine(), hdr.type()))
			preview.pack_start(label, False)
			if hdr.type() == zero.Elf_Hdr.Type.Executable:
				frame = gtk.Frame("Command Line Arguments")
				preview.pack_end(frame, False)
				combo = gtk.ComboBoxEntry()
				combo.set_size_request(300, -1)
				frame.add(combo)
				btn = gtk.Button("Edit En_vironment...")
				preview.pack_end(btn, False)
			else:
				pass ### todo: handle core files ###
			preview.show_all()
			return True
		return False


	def on_filter(self, finfo, data):
		hdr = ELF.__get_hdr(finfo[0])
		if hdr:
			type = hdr.type()
			if (type == zero.Elf_Hdr.Type.Executable or 
				type == zero.Elf_Hdr.Type.Core):
				return True
		return False


	def open(self, filename):
		hdr = ELF.__get_hdr(filename)
		if hdr:
			if hdr.type() == zero.Elf_Hdr.Type.Executable:
				#todo: pass args, environment
				self.__app.load_exec(filename)
			else:
				self.__app.load_core(filename, None)
			return True
		return False

