# -------------------------------------------------------------------------
# This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# -------------------------------------------------------------------------
from code import Code
import zero


class Source(Code):
	def __init__(self, win):
		Code.__init__(self, win)


	def read_file(self, thread, addr, sym, brkpoints):
		assert sym
		filename = sym.filename()
		f = open(filename)
		line = f.readline()
		lineNum = 0
		model = self._Code__model
		model.clear()
		while line:
			lineNum += 1
			iter = model.append()
			for b in brkpoints:
				sym = b.symbol()
				if sym.filename() == filename and sym.line() == lineNum:
					self.__set_visual_breakpoint(model, iter, b)
					self._Code__breakpoints[b.addr()] = b
			model.set_value(iter, 1, lineNum)
			model.set_value(iter, 2, line.strip("\n"))
			line = f.readline()
		f.close()


	def __set_visual_breakpoint(self, model, iter, bkpoint):
		model.set_value(iter, 0, self._Code__stop)	


	def get_addresses(self, iter):
		lineNum = int(self._Code__model.get_value(iter, 1))
		#convert line number to addresses:
		addrs = zero.debugger().line_to_addr(self.filename(), lineNum)
		return addrs

