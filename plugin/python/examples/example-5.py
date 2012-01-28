#
# Example 5: Stepping through code
#
import zero


def print_current_line(thread):
	sym = thread.symbols().lookup(thread.program_count())
	if sym:
		func = sym.demangle(zero.Symbol.Demangle.NameOnly)
		print '>>>>>',sym.filename(),sym.line(),func



def print_local_vars(thread):
	#print variables in local scope
	vars = []
	for v in thread.variables(zero.Scope.Local):
		vars.append('%s: %s' % (v.name(), v.value()))
	print vars



def my_breakpoint(thread, breakpoint):
	print_current_line(thread)

	#step over C/C++ source lines (without diving into function calls)
	thread.step(zero.Step.OverStatement)
	print_current_line(thread)
	thread.step(zero.Step.OverStatement)
	print_current_line(thread)

	#step one C/C++ statement, diving into function calls (if any)
	thread.step(zero.Step.Statement)
	print_current_line(thread)
	
	thread.step(zero.Step.OverStatement)
	print_current_line(thread)
	print_local_vars(thread)

	#step until the current function returns:
	print "----- calling zero.Step.Return -----"
	thread.step(zero.Step.Return)
	print_current_line(thread)

	print "----- breakpoint done ------"
	return True



def on_process(process, thread):
	for sym in process.symbols().lookup("baz"):
		thread.set_breakpoint(sym.addr(), my_breakpoint)
	
	#step one machine instruction
	thread.step(zero.Step.Instruction)


