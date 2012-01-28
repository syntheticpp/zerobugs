# -------------------------------------------------------------------------
# This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# -------------------------------------------------------------------------
from code import Code
import zero


class Disasm(Code):
	"""
	Specialized Code view for showing disassembly
	"""
	def __init__(self, win):
		Code.__init__(self, win)
		self.__line_to_addr = None
		self.__addr_to_line = None

	def read_file(self, thread, addr, sym, brkpoints):
		addr -= 128
		start = thread.symbols().lookup(addr)
		if start:
			start = thread.symbols().lookup(start.addr() - start.offset())
		if not start:
			start = sym
		model = self._Code__model
		model.clear()
		self.__line_to_addr = {}
		self.__addr_to_line = {}
		line_num = 0
		for line in thread.disassemble(start, 256, zero.Disasm.WithSource):
			addr = line[0]
			self.__line_to_addr[line_num] = addr
			self.__addr_to_line[addr] = line_num
			iter = model.append()
			model.set_value(iter, 1, hex(addr))
			try:
				asm = line[1].split(":")[1].split("\t")
				#print asm
				model.set_value(iter, 2, asm[1])
				model.set_value(iter, 3, asm[2])
			except:
				asm = line[1].split(" ")[-1]
				model.set_value(iter, 2, asm)
			line_num += 1


	def set_current_line(self, addr, line):
		try:
			line = self.__addr_to_line[addr]
			super(Disasm, self).set_current_line(addr, line)
		except KeyError:
			pass

	def get_addresses(self, iter):
		a = long(self._Code__model.get_value(iter, 1), 16)
		return [a,]
	
