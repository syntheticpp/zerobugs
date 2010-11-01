#
# Implementation of the Debugger Machine Interface
# See: http://www.linux-foundation.org/en/DMI
# 
# Copyright (c) 2007 Cristian L. Vlasceanu
#
# So far this is only a very sketchy prototype. What it basically
# does: bypasses the built-in command line and parses the input 
# looking for MI commands. If the input does not start with a '-',
# then pass the command to the debugger to be executed.
#
# todo: auto-completion (possibly using readline module?)
#import readline 
import os
import zero


#Helper function for formatting information about a symbol in the
#debugged program.
def symbol_to_string(sym):
	file = sym.filename()
	return 'func="%s",file="%s",fullname="%s",line="%d"' % (
			sym.demangle(), os.path.basename(file), file, sym.line())

#Helper function for formatting a Stack Frame as a string.
#Frame info is printed when the debugged program stops.
def frame_to_string(frame):
	#todo: args
	return "frame={" + symbol_to_string(frame.function()) + ",args=[]}"


class Interpreter(object):
	"Implement the Machine Interface open standard"
	#todo: enforce singleton pattern
	def __init__(self, debugger):
		self.__debugger = debugger


	def run_interactive(self, thread):
		zero.bypass_builtin_command_interpreter(True)
		self.__debugger.command("list", thread)
		line = raw_input("(zero) ")
		if line:
			tokens = line.split(" ")
			if tokens[0][0] == '-':
				return self.__mi_command(tokens, thread)
		self.__debugger.command(line, thread)


	#interpret the tokens in a fashion that is compatible with:
	#http://sources.redhat.com/gdb/current/onlinedocs/gdb_25.html#SEC240
	def __mi_command(self, tokens, thread):
		commands = tokens[0].split("-")
		cmd = commands[1]
		try:
			{ 'break' : self.__mi_break,
			  'exec'  : self.__mi_exec,			  
			} [ cmd ](commands[2], tokens, thread)
		except KeyError:
			print '^error,msg="Undefined MI command: %s"' % tokens[0]


	def __mi_break(self, subcommand, tokens, thread):
		try:
			{ 'insert' : self.__mi_break_insert
			} [ subcommand ] (tokens, thread)
		except KeyError:
			print '^error,msg="Undefined MI command: -break-%s"' % subcommand


	def __mi_break_insert(self, tokens, thread):
		for i in range(1, len(tokens)):
			arg = tokens[i]
			if arg == "-t":
				pass #todo: temporary breakpoint
			elif arg == "-h":
				#treat "hardware breakpoint" as "per thread breakpoint"
				#todo
				pass
			elif arg == "-i":
				pass #todo
			elif arg == "-c":
				pass #todo
			else:
				addrs = []
				func = tokens[1]
				line = func.split(":")
				#todo: emit error for foo.cpp:42:blah?

				if len(line) > 1:
					try:
						file = line[0]
						n = int(line[1])
						if len(file) == 0:
							frame = thread.stack_trace().selection()
							file = frame.function().filename()
						addrs = self.__debugger.line_to_addr(file, n)

					except:
						func = line[1]
						print "func:",func
				else:
					if func[0] == '*':
						addrs.append(int(func[1:], 16))
						print addrs

				if len(addrs) == 0:	
					for sym in thread.symbols().lookup(func):
						addrs.append(sym.addr())
				for a in addrs:
					thread.set_breakpoint(a)
					#thread.process().set_breakpoint(a)
	
	
	def __mi_exec(self, subcommand, tokens, thread):
		try:
			{ 
				'continue' 	: self.__mi_continue,
				'finish'	: self.__mi_finish,
				'next'	   	: self.__mi_next,
				'run'	   	: self.__mi_continue,
				'step'	   	: self.__mi_step,
			} [ subcommand ] (tokens, thread)
		except KeyError:
			print '^error,msg="Undefined MI command: -exec-%s"' % subcommand


	def __mi_continue(self, tokens, thread):
		assert self.__debugger
		self.__debugger.command("continue", thread)
		print "^running"

	def __mi_finish(self, tokens, thread):
		print "^running"
		thread.step(zero.Step.Return)

	def __mi_next(self, tokens, thread):
		print "^running"
		thread.step(zero.Step.OverStatement)

	def __mi_step(self, tokens, thread):
		print "^running"
		thread.step(zero.Step.Statement)



#global interpreter instance, instantiated by on_init
interp = None

def on_init(debugger):
	"Callback emitted when the debugger initializes Python"
	global interp
	interp = Interpreter(debugger)


def on_process(process, thread):
	"""
	Callback emitted by the debugger when a new process is
	detected in the target.
	"""
	pass


def on_thread(thread):
	"Callback emitted by the debugger when a new thread is detected"
	pass


def on_thread_detach(thread):
	pass


def on_process_detach(process):
	if process:
		pass


def on_new_breakpoint_action(n, brkpnt, action):
	sym = brkpnt.symbol()
	count = action.counter()
	print '^done,bkpt={number="%d",addr="0x%x",%s,times="%d"}' \
			% (n, brkpnt.addr(), symbol_to_string(sym), count)

	
def on_event(event):
	"Callback emitted when a debug event occurs"

	try:
		reason = { 
			zero.DebugEvent.Type.Signal : 'signal-received',
			zero.DebugEvent.Type.Breakpoint : 'breakpoint-hit',
			zero.DebugEvent.Type.BreakPoint : 'breakpoint-hit',
			zero.DebugEvent.Type.Finished : 'exited',
			zero.DebugEvent.Type.SysCallEnter : 'syscall-enter', #
			zero.DebugEvent.Type.SysCallLeave : 'syscall-leave', #
			zero.DebugEvent.Type.SingleStep : 'end-stepping-range',
			zero.DebugEvent.Type.EvalComplete : 'eval-complete', #
			zero.DebugEvent.Type.DoneStepping : 'end-stepping-range',
			zero.DebugEvent.Type.CallReturned : 'function-finished'
		} [event.type()]
	
		msg = '*stopped,reason="%s"' % reason
	
		thread = event.thread()
		if thread:
			frame = thread.stack_trace().selection()
			if frame:
				msg += "," + frame_to_string(frame)
		print msg

	except KeyError:
		pass

	if interp:
		interp.run_interactive(event.thread())


def on_progress(what, percent):
	print "%d%% %s" % (percent * 100, what)
	return True


def on_table_done(symtab):
	"Callback emitted when the debugger is finished loading a symbol table"
	#print '-----',symtab.filename(),symtab.process().name(),len(symtab)
	pass

