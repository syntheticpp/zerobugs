#
# Example 7
#
# NOTE:
# The debugger needs to be started with the --ui-disable command line
# 	option to avoid GTK conflicts
#
import gobject
import pygtk
pygtk.require("2.0")
import gtk
import gtk.glade
import zero

gladefile = "plugin/python/examples/example-7/example-7.glade"

#Format the current source file and line number
def current_line(thread):
	sym = thread.symbols().lookup(thread.program_count())
	if sym:
		func = sym.demangle(zero.Symbol.Demangle.NameOnly)
		if sym.line():
			return "%s:%d %s\n" % (sym.filename(),sym.line(),func)
		else:
			#line number may be zero, in which case filename() is the 
			#name of the binary module (exe or shared object)
			return "%s %s\n" % (sym.filename(), func)
	else:
		return "\n"


def handle_events(source, condition):
	zero.event_iteration()
	return True 


# Monitor system calls and display them in a gtk textview
class SysCallMonitor(object):
	def __init__(self):
		self.widgets = gtk.glade.XML(gladefile)
		self.window = self.widgets.get_widget("main")
		if self.window:
			self.window.connect("destroy", gtk.main_quit)
			gobject.io_add_watch(zero.event_pipe(), gobject.IO_IN, handle_events)


	def show_event(self, event):
		if event.thread().finished():
			return

		type = event.type()
		text = ''
		if type == zero.DebugEvent.Type.SysCallEnter:
			thread = event.thread()
			text = '>>> SysCallEnter #:%d %s' % (event.syscall(),current_line(thread))
		elif type == zero.DebugEvent.Type.SysCallLeave:
			thread = event.thread()
			text = '<<< SysCallLeave #:%d %s' % (event.syscall(),current_line(thread))
		else:
			return

		view = self.widgets.get_widget("textview")
		if view:
			view.get_buffer().insert_at_cursor(text)
			

#gtk.threads_init()
mon = SysCallMonitor()


def on_process(process, thread):
	process.debugger().set_option(zero.Debugger.Option.TraceSysCalls)


def on_event(event):
	mon.show_event(event)


gtk.main()

