#
# Example 6: Tracing System Calls
#
import zero


def current_line(thread):
	sym = thread.symbols().lookup(thread.program_count())
	if sym:
		func = sym.demangle(zero.Symbol.Demangle.NameOnly)
		if sym.line():
			return "%s:%d %s" % (sym.filename(),sym.line(),func)
		else:
			#line number may be zero, in which case filename() is the 
			#name of the binary module (exe or shared object)
			return "%s %s" % (sym.filename(), func)



def on_process(process, thread):
	#System calls are not traced by default
	process.debugger().set_option(zero.Debugger.Option.TraceSysCalls)

	#The debugger does not normally stop at system calls
	#process.debugger().set_option(zero.Debugger.Option.BreakOnSysCalls)



def on_event(event):
	if event.type() == zero.DebugEvent.Type.SysCallEnter:
		print '>>>>> SysCall #:',event.syscall(),current_line(event.thread())
	elif event.type() == zero.DebugEvent.Type.SysCallLeave:
		print '<<<<< SysCall #:',event.syscall(),current_line(event.thread())

