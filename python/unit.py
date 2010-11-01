import zero

def on_event(event):
	thread = event.thread()
	addr = thread.program_count()
	unit = thread.debugger().lookup_unit_by_addr(addr)
	if unit:
		print "lang:", unit.language(),unit.filename()



