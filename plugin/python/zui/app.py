# -------------------------------------------------------------------------
# This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# -------------------------------------------------------------------------

import os
try:
	topPath=os.environ["ZTOP"]
except KeyError:
	topPath="/home/cristiv/workspace/zero/plugin/python/zui"
	os.environ["ZTOP"] = topPath

gladefile = topPath + "/zui.glade"

import gobject
import gtk
import gtk.glade
import pygtk
import sys
sys.path.append(topPath)
import dlg
import ffilter
import view
import tool
import zero


def update_state(parent, active):
	for w in parent.get_children():
		name = w.get_name()
		if name[0:5] == "tool_":
			#hackish workaround for ToolButton bug
			# see: http://bugzilla.gnome.org/show_bug.cgi?id=56070
			w.hide() 
			w.show() 
			if name == "tool_stop":
				w.set_sensitive(not active)
			else:
				w.set_sensitive(active)


#################################################################
#
#
class ZApp(view.App):
	"""
	The main Debugger ZApplication class
	"""
	def __init__(self):
		view.Composite.__init__(self)

		#Schedule connecting the event pipe to event pump. The reason for
		# doing it this way rather than directly calling gobject.io_add_watch
		# is to let the UI paint itself before starting a (possibly) lengthy
		# debugger operation. (The PythonGate plugin blocks the main thread 
		# until zero.event_pipe() is called).
		gobject.idle_add(self.__connect_event_pipe)

		self.__corefile = None
		self.__debugger = zero.debugger()
		self.__process = None
		self.__thread = None
		self.__widgets = gtk.glade.XML(gladefile)
		self.__window = self.__widgets.get_widget("main")
		self.__window.connect("destroy", self.__on_destroy)

		#The listings view shows C++ code and assembly code
		self.__listings = view.Listings(self.__widgets.get_widget("listings"))
		self.__init_bottom_book()
		self.__init_right_book()
		self.add(self.__listings)
		self.__init_menu()
		self.__init_toolbar(self.__widgets.get_widget("toolbar"))
		self.__init_font_button()

	def bypass_builtin_command_interpreter(self, value):
		zero.bypass_builtin_command_interpreter(value)

	# Event pump
	def __dispatch(self, source, condition):
		zero.event_iteration()
		return True


	# Connect event pipe to event pump above
	def __connect_event_pipe(self):
		gobject.io_add_watch(zero.event_pipe(), gobject.IO_IN, self.__dispatch)
		return False




	############################################################
	#
	# helpers called by __init__
	#
	def __init_toolbar(self, toolbar):
		self.__init_file_open(toolbar)
		btns = [
			("stop", "gtk-media-pause", "Break", "Break execution", 0),
			("continue", "gtk-media-play", "Continue", "Resume execution", 1),
			("next", "gtk-media-forward", "Next", "Step over source line", 0),
			("step", "gtk-media-next", "Step", "Step into function", 1),
			("ret", "gtk-undo", "Return", "Run until function returns", 1)
		]
		tb = tool.Bar(toolbar)			
		tb.add_space()
		for b in btns:
			tb.add_stock(b[0], b[1], b[2], b[3], b[4], self.__on_tool_clicked)
		

	def __init_file_open(self, toolbar):
		try:
			self.__fileDialog = gtk.FileChooserDialog()
			self.__fileDialog.connect("close", self.__on_filechooser_close)
			self.__fileDialog.connect("unmap-event", self.__on_unmap_event)

			self.__fileButton = gtk.FileChooserButton(self.__fileDialog)
			self.__fileButton.set_width_chars(6)

			#connect the widget with the class that implements the logic
			self.__fileOpen = dlg.FileOpen(
				self.__fileDialog, 
				[ffilter.ELF(self, gtk.FileFilter())])

			#wrap FileChooserButton in a ToolItem
			item = gtk.ToolItem()
			item.add(self.__fileButton)
			item.show_all()
			toolbar.insert(item, 0)

		except:
			#FileChooser is available n Pygtk 2.4 and newer;
			# fallback to FileSelection if not available
			self.__fileButton = None
			self.__fileDialog = gtk.FileSelection()
			self.__fileDialog.hide_fileop_buttons()
			#connect the widget with the class that implements the logic
			self.__fileOpen = dlg.FileOpen(
					self.__fileDialog, [ffilter.ELF(self, None)])


	def __init_font_button(self):
		button = self.__widgets.get_widget("fontbutton")
		if button:
			button.connect("font-set", self.__on_font_set)
			button.set_show_style(True)

	def __on_detach(self, item):
		#todo: ask user to confirm
		self.__debugger.detach()	

	def __on_open(self, item):
		self.__fileDialog.set_transient_for(self.__window)
		self.__fileDialog.run()

	def __on_quit(self, item):
		self.__window.destroy()


	def __init_menu(self):
		quit = self.__widgets.get_widget("quit")
		quit.connect("activate", self.__on_quit)
		menuItem = self.__widgets.get_widget("open")
		menuItem.connect("activate", self.__on_open)
		menuItem = self.__widgets.get_widget("detach")
		menuItem.connect("activate", self.__on_detach)

		programMenuItems = [
			"tool_continue", 
			"tool_next", 
			"tool_step", 
			"tool_stop"]

		for name in programMenuItems:
			menuItem = self.__widgets.get_widget(name)
			menuItem.connect("activate", self.__on_tool_clicked)


	def __init_bottom_book(self):
		stackWidget = self.__widgets.get_widget("stack")
		stackView = view.Stack(stackWidget)
		stackView.connect("selection-changed", self.__listings.show_frame)
		stackView.connect("selection-changed", self.__on_stack_selection)
		self.add(stackView)
		localsWidget = self.__widgets.get_widget("locals")
		self.__locals = view.Data(localsWidget)
	
		try:
			bottom = self.__widgets.get_widget("notebook1")
			bottom.set_group_id(1)
			bottom.set_tab_detachable(localsWidget, True)
			bottom.set_tab_detachable(stackWidget, True)
		except:
			pass

	
	def __init_right_book(self):
		threadsWidget = self.__widgets.get_widget("threads")
		regsWidget = self.__widgets.get_widget("registers")
		self.add(view.Threads(threadsWidget))
		self.add(view.Registers(regsWidget))
		try:
			right = self.__widgets.get_widget("notebook2")
			right.set_group_id(1)
			right.set_tab_detachable(threadsWidget, True)
			right.set_tab_detachable(regsWidget, True)
		except:
			pass


	def clear(self):
		super(ZApp, self).clear()
		self.__process = None


	def update_debuggee_name(self, process):
		name = process.name()
		self.__process = process
		if process.origin() <> zero.Process.Origin.Core:
			self.__corefile = None
		self.__window.set_title("Zero: " + name)
		self.__set_filename()


	def update(self, event):
		super(ZApp, self).update(event)
		thread = event.thread()
		self.__locals.show(thread.variables(zero.Scope.Local))
		self.__thread = (thread.lwpid(), thread.id())
		self.__debugger = thread.debugger()


	def load_exec(self, command):
		if self.__debugger:
			self.__debugger.load_exec(command)


	def load_core(self, corefile, program):
		if self.__debugger:
			self.__corefile = corefile
			self.__debugger.load_core(corefile, program)


	def __on_filechooser_close(self, dialog):
		dialog.hide()


	def __set_filename(self):
		process = self.__process
		if process and self.__fileButton:
			if self.__corefile:
				self.__fileDialog.set_filename(self.__corefile)
			else:
				self.__fileDialog.set_filename(process.name())
			dlg.set_filename(self.__fileButton, process)


	#Prevent the resetting of the filename and image in
	# the filechooser button when the dialog is unmapped.
	def __on_unmap_event(self, w, event):
		self.__set_filename()


	def __on_destroy(self, window):
		if self.__debugger:
			self.__debugger.quit()
		self.bypass_builtin_command_interpreter(False)
		gtk.main_quit()


	def __on_font_set(self, button):
		print button.get_font_name()
		# TODO: set font


	def __on_stack_selection(self, stackView, thread, frame):
		self.__locals.show(thread.variables(zero.Scope.Local))


	def __on_tool_clicked(self, button):
		tid = self.__thread
		if tid is None:
			return
		thread = self.__debugger.get_thread(tid[0], tid[1])
		if thread:
			name = button.get_name()
			if name[0:5] == "tool_":
				self.__debugger.command(name[5:], thread)
			else:
				self.__debugger.command(name, thread)


	def on_state(self, active):
		#
		# set toolbar buttons sensitivity
		#
		toolbar = self.__widgets.get_widget("toolbar")
		update_state(toolbar, active)
		menu = self.__widgets.get_widget("menu_program").get_submenu()
		update_state(menu, active)

#end ZApp


if __name__ == "__main__":
	app = ZApp()

	#update the ZApp view to reflect the state of the debuggee after the event
	def on_event(event):
		if event.thread() is None:
			return True
		return app.update(event)


	def on_process(process, thread):
		app.bypass_builtin_command_interpreter(True)
		app.update_debuggee_name(process)


	def on_thread(thread):
		#print '----- on_thread:', thread.lwpid(), '-----'
		app.bypass_builtin_command_interpreter(True)


	def on_process_detach(process):
		print '----- process detached -----'
		app.clear()
		app.bypass_builtin_command_interpreter(False)


	def on_command_loop_state(active):
		app.on_state(active)


	def on_error(msg):
		print msg


	gtk.main()
