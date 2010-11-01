#
# Example 2: print stack trace every time a debug event occurs
#
import zero


def format_symbol(sym):
	name = sym.demangle(zero.Symbol.Demangle.Param)
	#return "0x%0x %s +<0x%0x>" % (sym.addr(), name, sym.offset())
	return "0x%0x %s:%d %s+<0x%0x>" % \
		(sym.addr(), sym.filename(), sym.line(), name, sym.offset())
		

def print_stack_trace(thread):
	trace = thread.stack_trace()
	for frame in trace.frames():
		param = {}
		for p in thread.param(frame):
			param[p.name()] = p.value()

		print '---',frame.index(),format_symbol(frame.function()),param
		


def on_event(event):
	print_stack_trace(event.thread())

