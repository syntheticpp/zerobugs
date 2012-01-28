#
# Example 9: executing debugger commands 
#
import zero


def on_init(debugger):
	def err(msg):
		debugger.message('Error in script: ' + msg, zero.Message.Error)
	return err



def my_breakpoint(thread, breakpoint):
	thread.debugger().command("eval 100 / 3.", thread)
	return 1



def on_process(process, thread):
	for sym in process.symbols().lookup("bar"):
		thread.set_breakpoint(sym.addr(), my_breakpoint)

