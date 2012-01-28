#
# Example 4: Set a breakpoint and print a stack trace everytime
# 		     it gets hit; show CPU regs for each stack frame
#
import zero


def format_symbol(symbol):
	name = symbol.demangle(zero.Symbol.Demangle.Param)
	return "0x%0x %s +<0x%0x>" % (symbol.addr(), name, symbol.offset())
		

def print_stack_trace(thread):
	trace = thread.stack_trace()
	for frame in trace.frames():
		print '-----',frame.index(),format_symbol(frame.function())
		print '-----',thread.regs(frame)
		#NOTE: thread.regs() returns a Python dictionary



def my_breakpoint(thread, breakpoint):
	print_stack_trace(thread)



def on_process(process, thread):
	#call my_breakpoint every time a function named "bar"
	#is executed by the debugged process:

	for sym in process.symbols().lookup("bar"):
		thread.set_breakpoint(sym.addr(), my_breakpoint)
		for b in thread.breakpoints():
			print '-----',b.type(),'breakpoint at','%x' % b.addr()	

