#
# Example: persistent properties
#
import zero


def on_process(process, thread):
	print '----- process attached:',process.command_line()
	print '----- thread lpwid:',thread.lwpid()

	print process.debugger().read_settings()
	



