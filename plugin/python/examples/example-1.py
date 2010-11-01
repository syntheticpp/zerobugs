#
# Example 1: "Standard" Callbacks
#
import zero


# Callback emitted by the debugger when a new process is detected
# in the debugged target
def on_process(process, thread):
	print '----- process attached:',process.command_line()
	print '----- thread lpwid:',thread.lwpid()
	
	#show debugged process' environment
	#process.environ() returns a Python dict
	print process.environ()

	#process.debugger().message(repr(process.environ()), zero.Message.Info)


# Callback emitted by the debugger when a new thread is detected
# in the debugged target
def on_thread(thread):
	print '------ thread attached:',thread.lwpid(),'/',thread.id()



def on_thread_detach(thread):
	print '----- thread detaching:',thread.lwpid()



def on_process_detach(process):
	if process:
		print '----- process detached:',process.pid()



def on_event(event):
	thread = event.thread()
	if thread:
		print '----- event type:',event.type(),'thread:',thread.lwpid()
		sym = thread.symbols().lookup(thread.program_count())
		if sym:
			name = sym.demangle(zero.Symbol.Demangle.Param)
			print '----- %x' % thread.program_count(), name



def on_progress(what, percent):
	print "%d%% %s" % (percent * 100, what)
	return True



# callback emitted when the debugger is finished loading a symbol table
def on_table_done(symtab):
	print '-----',symtab.filename(),symtab.process().name(),len(symtab)
