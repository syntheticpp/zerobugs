#
# Example 8: Evaluating expressions
#
import zero


def my_breakpoint(thread, breakpoint):
	thread.step(zero.Step.OverStatement)
	for sym in thread.eval('do_something(0)'):
		msg = "%s=%s" % (sym.name(), sym.value())
		thread.debugger().message(msg, zero.Message.Info)
	return 1



def on_process(process, thread):
	for sym in process.symbols().lookup("bar"):
		thread.set_breakpoint(sym.addr(), my_breakpoint)

