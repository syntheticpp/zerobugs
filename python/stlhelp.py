# vim: expandtab
# -------------------------------------------------------------------------
# This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# -------------------------------------------------------------------------
#
import zero

# tunable params
param = { 
	'vector_as_array'	:[True, 'Show std::vectors as C arrays'],
	'set_as_array'		:[False, 'Show std::set objects as arrays'],
	'string_as_cstr'	:[True, 'Show std::string as C string'],
}
def get_configurable_params():
	return param

def on_param_change(prop, name):
	global param
	#todo: as of now only boolean params are supported,
	# extend to int, double, string etc.
	param[name][0] = bool(prop.get_word(name, 0))

def read_param_from_properties(prop):
	global param
	for name in param:
		param[name][0] = bool(prop.get_word(name, param[name][0]))
		print "****", name, param[name][0]

def show_set_as_array():
	global param
	return param['set_as_array'][0] == True

def show_vector_as_array():
	global param
	return param['vector_as_array'][0] == True

def show_string_as_cstr():
	global param
	return param['string_as_cstr'][0] == True

#
# Misc. helpers
#
def to_long(param):
	"Silent conversion from string to long"
	try:
		return int(param, 0)
	except:
		return 0


def find_child_by_name(sym, tokens, recursive):
	"Find a child of a DebugSymbol object"
	for child in sym.children():
		#print child.name()
		if child.name() == tokens[0]:
			if len(tokens) > 1:
				tokens.pop(0)
				return find_child_by_name(child, tokens, False)
			else:
				return child
		if recursive:
			#recursively descend into children
			tmp = find_child_by_name(child, tokens, recursive)
			if tmp:
				return tmp
		if sym.type().is_pointer() and sym.value() <> "_M_parent":
			child = sym.children()[0]
			tmp = find_child_by_name(child, tokens, False)
			if tmp:
				return tmp


def find_child(sym, path, recursive = True):
	"Lookup a child of the given symbol, by name, which may include dots"
	name = path.split(".")
	return find_child_by_name(sym, name, recursive)


def find_child_sym(sym, name):
	for child in sym.children():
		if child.name() == name:
			return child

		if sym.type().is_pointer() and sym.value() <> "_M_parent":
			child = sym.children()[0]
			tmp = find_child_sym(child, name)
			if tmp:
				return tmp


def lookup_type(thread, name, scope = zero.Scope.Module):
	"Lookup a data type by name"
	type = thread.lookup_type(name, scope)

	#hack: try evaluating a cast
	if not type:
		results = thread.eval("(" + name + ")0")
		if len(results):
			for tmp in results:
				type = tmp.type()
				if type:
					break
	return type


def make_array(sym, size, elem_type, replace, addr = 0):
	if size > 0:
		array = sym.type_system().array_type(size, elem_type)
		if replace:
			#completely replace the object with the array view
			name = sym.name()
			newsym = sym.create(name, array, addr)
			return (newsym, newsym)
		else:
			#add an array view to the vector
			name = '__asArray'
			newsym = sym.create(name, array, addr)
			sym.add_child(newsym)
			newsym.read()
			return (newsym, None)
	return (None, None)


def extract_first_template_param(type): 
	param = ""

	try:
		#only works with compilers that generate 
		# template type debug information

		param = type.template_types()[0].type().name()
		return param
	except:
		pass

	name = type.name()
	tokens = name.split("<")
	if len(tokens) > 0:
		if tokens[1].find(",") >= 0:
			param = tokens[1].split(",")[0]
			return param
		depth = 0
		for tok in tokens[1:]:
			depth += 1
			if len(param):
				param += "<"
			if tok.find(">") < 0:
				param += tok
			else:
				s = tok.split(",")[0]
				c = s.count(">")
				assert depth > c
				depth -= c
				if depth == 1:
					param += s
					break
				else:
					param += tok
	return param


class Tree(object):
	"""
	A class for processing associative containers
	that are implemented in terms of red-black trees
	"""

	def __init__(self, sym, param):
		self.__sym = sym
		self.__thread = sym.thread()
		self.__param = param #name of template type param

	def walk(self, node):
		"walk the tree in order"
		if node.value() == "NULL":
			return
		#left = find_child(node, "_M_left", False)
		left = find_child_sym(node, "_M_left")
		if left:
			self.walk(left)
		expr = "(typename std::_Rb_tree_node<"
		expr = expr + self.__param + " >&)" + node.value()

		eval = self.__thread.eval(expr)
		for result in eval:
			result.read()
			#child = find_child(result, "_M_value_field", False)
			child = find_child_sym(result, "_M_value_field")
			if child:
				child.read()
				sym = on_debug_symbol(child)
				if sym:
					child = sym
				self.__sym.add_child(child)
		right = find_child_sym(node, "_M_right")
		#right = find_child(node, "_M_right", False)
		if right:
			self.walk(right)


	def find_top(self, node):
		"Find the top of a red-black tree"
		value = node.value()
		#parent = find_child(node, "_M_parent", False)
		parent = find_child_sym(node, "_M_parent")
		while parent:
			#Stop when node is the parent of the parent
			if value == parent.value():
				break
			node = parent
			#parent = find_child(node, "_M_parent", False)
			parent = find_child_sym(node, "_M_parent")
			if parent:
				value = parent.value()
		return node



	#okay, not the most effective way to traverse the
	# tree, but good enough for now
	def walk_in_order(self, node):
		top = self.find_top(node)
		self.walk(top)



################################################################
#
# std::string< >
#
def string_as_cstr(sym, replace):
	if show_string_as_cstr():
		p = find_child(sym, "_M_p")
		if p:
			p.read()
			return p


################################################################
#
# std::vector< >
#
def vector_as_array(sym, replace = False):
	"make an array representation for a symbol of std::vector type"
	if not show_vector_as_array():
		return None
	b = find_child(sym, "_M_start")
	e = find_child(sym, "_M_finish")
	if b and e and b.type().is_pointer():
		begin = to_long(b.value())
		elem = b.type().pointed_type()
		size = (to_long(e.value()) - begin) / elem.size()
		newsym, retsym = make_array(sym, size, elem, replace, begin)
		return retsym
	return None


################################################################
#
# std::set< >
#
def set_as_array(sym, replace = False):
	"make an array representation for a symbol of type std::set<>"

	if not show_set_as_array():
		return None

	nodeCount = find_child(sym, "_M_t._M_impl._M_node_count")
	if not nodeCount:
		return
	count = to_long(nodeCount.value())

	#set same sane limit (todo: make it configurable)
	if count > 4096:
		return None

	typeParam = extract_first_template_param(sym.type())
	elemType = lookup_type(sym.thread(), typeParam)
	if not elemType:
		return

	head = find_child(sym, "_M_t._M_impl._M_header", False)
	if head:
		newsym, retsym = make_array(sym, count, elemType, replace)
		if newsym:
			newsym.set_value("*** synthesized ***")
			tree = Tree(newsym, elemType.name())
			tree.walk_in_order(head)
			return retsym

	return None


################################################################
#
# std::map< >
#
def map_as_array(sym, replace = False):
	# TODO
	return None


def on_init(debugger):
	read_param_from_properties(debugger.properties())


################################################################
# 
# entry point
# 
def on_debug_symbol(sym):
	"""
	Debugger callback, this is the script's entry point
	"""

	#set it to True to completely replace the symbol with 
	#the returned sym
	replace = True
	#replace = False

	if sym and sym.is_expanding(): 
		type = sym.type().name().split("<")[0]
		try:
			sym = { 
				"std::map" : map_as_array,
				"map" : map_as_array,
				"std::set" : set_as_array,
				"set" : set_as_array,
				"std::string" : string_as_cstr,
				"string" : string_as_cstr,
				"std::basic_string" : string_as_cstr,
				"basic_string" : string_as_cstr,
				"std::vector" : vector_as_array,
				"vector" : vector_as_array,
			} [type](sym, replace)

			return sym

		except KeyError:
			pass

	return None


