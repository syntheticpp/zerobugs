import os
import re
import zero

def on_table_done(symTable):
	""" 
	Calback function automatically invoked
	by the debugger each time a new symbol table
	is loaded into memory
	"""
	if symTable.is_dynamic():
		return
	#print symTable.filename()
	tests = []
	extension = re.compile('.d$')

	process = symTable.process()

	for unit in symTable.module().translation_units():
		#print unit.filename(),unit.language()

		if unit.language() == zero.TranslationUnit.Language.D:
			name = os.path.basename(unit.filename())
			#i = 0
			i = 1
			while True:
				#As of now, the D demangler does not support any flags,
				# and we cannot instruct it to omit result types and 
				# param lists from the mangled name.
				#symName = "void " + extension.sub('.__unittest%d()' % i, name)
				symName = "void " + unit.module() + ('.__unittest%d()' % i)
				print symName
				matches = symTable.lookup(symName)	
				if not len(matches):
					break
				#print symName
				tests.append(symName)
				for sym in matches:
					process.set_breakpoint(sym.addr())
				i += 1

	if len(tests):
		zero.debugger().message(
			'Detected: \n' + ',\n'.join(tests), zero.Message.Info)
