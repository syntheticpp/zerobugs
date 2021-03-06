#
# Example 3: Breakpoints
#
import zero


def my_breakpoint(thread, breakpoint):
	print '-----', breakpoint.type(), \
		'breakpoint hit at 0x%0x' % thread.program_count()

	#you may quit the debugger at any time...
	#thread.debugger().quit()

	#return non-zero to break to interactive mode, 
	#return 0 to resume the debugged program
	return 1



def on_process(process, thread):
	print '----- attached:',process.pid()

	#call my_breakpoint every time a function named "bar"
	#is executed by the debugged process:
	for sym in process.symbols().lookup("bar"):

		# NOTE: the 2nd parameter of set_breakpoint is optional
		process.set_breakpoint(sym.addr(), my_breakpoint)

		# NOTE: to install a per-thread breakpoint, use:
		# thread.set_breakpoint(sym.addr(), my_breakpoint)


		for b in process.breakpoints():
			print '-----',b.type(),'breakpoint at','%x' % b.addr()

