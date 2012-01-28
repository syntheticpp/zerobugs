# vim: expandtab
# -------------------------------------------------------------------------
# This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# -------------------------------------------------------------------------

import zero

# tunable params
param = { 
	'vector_as_array'	:[True, 'Show std::vectors as C arrays'],
	'set_as_array'		:[False, 'Show std::set objects as arrays'],
	'string_as_cstr'	:[True, 'Show std::string as C string'],
	'qstring_as_cstr'	:[True, 'Show Qt Strings as C strings'],
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
		#print "****", name, param[name][0]

def show_set_as_array(sym):
	global param
	return sym.is_expanding() and param['set_as_array'][0] == True

def show_vector_as_array(sym):
	global param
	#return sym.is_expanding() and param['vector_as_array'][0] == True
	return param['vector_as_array'][0] == True

def show_string_as_cstr():
	global param
	return param['string_as_cstr'][0] == True

def show_qstring_as_cstr():
	global param
	return param['qstring_as_cstr'][0] == True


#
# Misc. helpers
#
def to_long(param):
	"Silent conversion from string to long"
	try:
		return int(param, 0)
	except:
		return 0


def find_child_by_name(sym, tokens, recursive, depth = 1):
	"Find a child of a DebugSymbol object"
	# print "find_child_by_name:",depth,tokens
	if depth > 8:
		return None

	for child in sym.children():
		if child.name() == tokens[0]:
			if len(tokens) > 1:
				tokens.pop(0)
				return find_child_by_name(child, tokens, False)
			else:
				return child
		if recursive:
			#recursively descend into children
			tmp = find_child_by_name(child, tokens, recursive, depth + 1)
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


def make_array(sym, size, elem_type, addr = 0):
	if size > 0:
		array = sym.type_system().array_type(size, elem_type)
		name = sym.name()
		newsym = sym.create(name, array, addr)
		return (newsym, newsym)
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
		
			child = find_child_sym(result, "_M_value_field")
			if child:
				child.read()
				sym = on_debug_symbol(child)
				if sym:
					child = sym
				self.__sym.add_child(child)
		right = find_child_sym(node, "_M_right")
		if right:
			self.walk(right)


	def find_top(self, node):
		"Find the top of a red-black tree"
		value = node.value()
		parent = find_child_sym(node, "_M_parent")
		while parent:
			#Stop when node is the parent of the parent
			if value == parent.value():
				break
			node = parent
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
def string_as_cstr(sym):
	if show_string_as_cstr():
		p = find_child(sym, "_M_p")
		if p:
			newsym = p.create(sym.name(), p.type(), p.addr())
			newsym.read()
			newsym.set_type_name(sym.type())
			return newsym


################################################################
#
# std::vector< >
#
def vector_as_array(sym):
	"make an array representation for a symbol of std::vector type"
	if not show_vector_as_array(sym):
		return None
	sym.set_numeric_base(16)
	if not sym.value():
		sym.read()
	b = find_child(sym, "_M_start")
	e = find_child(sym, "_M_finish")
	if b and e and b.type().is_pointer():
		begin = to_long(b.value())
		elem = b.type().pointed_type()

		if elem.size() != 0:
			size = (to_long(e.value()) - begin) / elem.size()
			newsym, retsym = make_array(sym, size, elem, begin)
			if retsym:
				retsym.set_type_name(sym.type())
				retsym.set_numeric_base(16)
				retsym.read()
				# tooltip
				tip = sym.name() + "[" + str(size) + "]( "
				try:
					for i in range(size):
						if i > 10:
							tip += "..."
							break
						s = retsym.children()[i]
						tip += s.value() + " "
				except:
					pass
				tip += ")"
				retsym.set_tooltip(tip)
			return retsym
	return None


################################################################
#
# std::set< >
#
def set_as_array(sym):
	"make an array representation for a symbol of type std::set<>"

	if not show_set_as_array(sym):
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
		newsym, retsym = make_array(sym, count, elemType)
		if newsym:
			newsym.set_value("*** synthesized ***")
			newsym.set_type_name(sym.type())
			tree = Tree(newsym, elemType.name())
			tree.walk_in_order(head)
			return retsym

	return None


################################################################
#
# std::map< >
#
def map_as_array(sym):
	# TODO
	return None

################################################################
#
#
def qstring_as_cstr(sym):
	if show_qstring_as_cstr():
		#try Qt3 string layout
		len = find_child(sym, "len")
		uni = find_child(sym, "unicode")

		if len and uni:
			addr = int(uni.value(), 0)
		else:
			#try Qt4 layout
			len = find_child(sym, "size")
			uni = find_child(sym, "array")
			addr = uni.addr()

		if len and uni:
			len = int(len.value())
			value = sym.thread().read_as_ucs2(addr, len)

			strType = sym.type_system().wstring_type()

			newsym = sym.create(sym.name(), strType, 0)
			newsym.set_value(value)
			newsym.set_type_name(sym.type())

			return newsym

	return None



def on_init(debugger):
	read_param_from_properties(debugger.properties())


################################################################
#
# Linux errno codes, edit as needed to match your system
#
errno = {
	1 : [ 'EPERM', '/* Operation not */'],
	2 : [ 'ENOENT', '/* No such file or directory */'],
	3 : [ 'ESRCH', '/* No such process */'],
	4 : [ 'EINTR', '/* Interrupted system call */'],
	5 : [ 'EIO', '/* I/O error */'],
	6 : [ 'ENXIO', '/* No such device or address */'],
	7 : [ 'E2BIG', '/* Argument list too long */'],
	8 : [ 'ENOEXEC', '/* Exec format error */'],
	9 : [ 'EBADF', '/* Bad file number */'],
	10 : [ 'ECHILD', '/* No child processes */'],
	11 : [ 'EAGAIN', '/* Try again */'],
	12 : [ 'ENOMEM', '/* Out of memory */'],
	13 : [ 'EACCES', '/* Permission denied */'],
	14 : [ 'EFAULT', '/* Bad address */'],
	15 : [ 'ENOTBLK', '/* Block device required */'],
	16 : [ 'EBUSY', '/* Device or resource busy */'],
	17 : [ 'EEXIST', '/* File exists */'],
	18 : [ 'EXDEV', '/* Cross-device link */'],
	19 : [ 'ENODEV', '/* No such device */'],
	20 : [ 'ENOTDIR', '/* Not a directory */'],
	21 : [ 'EISDIR', '/* Is a directory */'],
	22 : [ 'EINVAL', '/* Invalid argument */'],
	23 : [ 'ENFILE', '/* File table overflow */'],
	24 : [ 'EMFILE', '/* Too many open files */'],
	25 : [ 'ENOTTY', '/* Not a typewriter */'],
	26 : [ 'ETXTBSY', '/* Text file busy */'],
	27 : [ 'EFBIG', '/* File too large */'],
	28 : [ 'ENOSPC', '/* No space left on device */'],
	29 : [ 'ESPIPE', '/* Illegal seek */'],
	30 : [ 'EROFS', '/* Read-only file system */'],
	31 : [ 'EMLINK', '/* Too many links */'],
	32 : [ 'EPIPE', '/* Broken pipe */'],
	33 : [ 'EDOM', '/* Math argument out of domain of func */'],
	34 : [ 'ERANGE', '/* Math result not representable */'],
	35 : [ 'EDEADLK', '/* Resource deadlock would occur */'],
	36 : [ 'ENAMETOOLONG', '/* File name too long */'],
	37 : [ 'ENOLCK', '/* No record locks available */'],
	38 : [ 'ENOSYS', '/* Function not implemented */'],
	39 : [ 'ENOTEMPTY', '/* Directory not empty */'],
	40 : [ 'ELOOP', '/* Too many symbolic links encountered */'],
	#EAGAIN : [ 'EWOULDBLOCK', '/* Operation would block */'],
	42 : [ 'ENOMSG', '/* No message of desired type */'],
	43 : [ 'EIDRM', '/* Identifier removed */'],
	44 : [ 'ECHRNG', '/* Channel number out of range */'],
	45 : [ 'EL2NSYNC', '/* Level 2 not synchronized */'],
	46 : [ 'EL3HLT', '/* Level 3 halted */'],
	47 : [ 'EL3RST', '/* Level 3 reset */'],
	48 : [ 'ELNRNG', '/* Link number out of range */'],
	49 : [ 'EUNATCH', '/* Protocol driver not attached */'],
	50 : [ 'ENOCSI', '/* No CSI structure available */'],
	51 : [ 'EL2HLT', '/* Level 2 halted */'],
	52 : [ 'EBADE', '/* Invalid exchange */'],
	53 : [ 'EBADR', '/* Invalid request descriptor */'],
	54 : [ 'EXFULL', '/* Exchange full */'],
	55 : [ 'ENOANO', '/* No anode */'],
	56 : [ 'EBADRQC', '/* Invalid request code */'],
	57 : [ 'EBADSLT', '/* Invalid slot */'],
	59 : [ 'EBFONT', '/* Bad font file format */'],
	60 : [ 'ENOSTR', '/* Device not a stream */'],
	61 : [ 'ENODATA', '/* No data available */'],
	62 : [ 'ETIME', '/* Timer expired */'],
	63 : [ 'ENOSR', '/* Out of streams resources */'],
	64 : [ 'ENONET', '/* Machine is not on the network */'],
	65 : [ 'ENOPKG', '/* Package not installed */'],
	66 : [ 'EREMOTE', '/* Object is remote */'],
	67 : [ 'ENOLINK', '/* Link has been severed */'],
	68 : [ 'EADV', '/* Advertise error */'],
	69 : [ 'ESRMNT', '/* Srmount error */'],
	70 : [ 'ECOMM', '/* Communication error on send */'],
	71 : [ 'EPROTO', '/* Protocol error */'],
	72 : [ 'EMULTIHOP', '/* Multihop attempted */'],
	73 : [ 'EDOTDOT', '/* RFS specific error */'],
	74 : [ 'EBADMSG', '/* Not a data message */'],
	75 : [ 'EOVERFLOW', '/* Value too large for defined data type */'],
	76 : [ 'ENOTUNIQ', '/* Name not unique on network */'],
	77 : [ 'EBADFD', '/* File descriptor in bad state */'],
	78 : [ 'EREMCHG', '/* Remote address changed */'],
	79 : [ 'ELIBACC', '/* Can not access a needed shared library */'],
	80 : [ 'ELIBBAD', '/* Accessing a corrupted shared library */'],
	81 : [ 'ELIBSCN', '/* .lib section in a.out corrupted */'],
	82 : [ 'ELIBMAX', '/* Attempting to link in too many shared libraries */'],
	83 : [ 'ELIBEXEC', '/* Cannot exec a shared library directly */'],
	84 : [ 'EILSEQ', '/* Illegal byte sequence */'],
	85 : [ 'ERESTART', '/* Interrupted system call should be restarted */'],
	86 : [ 'ESTRPIPE', '/* Streams pipe error */'],
	87 : [ 'EUSERS', '/* Too many users */'],
	88 : [ 'ENOTSOCK', '/* Socket operation on non-socket */'],
	89 : [ 'EDESTADDRREQ', '/* Destination address required */'],
	90 : [ 'EMSGSIZE', '/* Message too long */'],
	91 : [ 'EPROTOTYPE', '/* Protocol wrong type for socket */'],
	92 : [ 'ENOPROTOOPT', '/* Protocol not available */'],
	93 : [ 'EPROTONOSUPPORT', '/* Protocol not supported */'],
	94 : [ 'ESOCKTNOSUPPORT', '/* Socket type not supported */'],
	95 : [ 'EOPNOTSUPP', '/* Operation not supported on transport endpoint */'],
	96 : [ 'EPFNOSUPPORT', '/* Protocol family not supported */'],
	97 : [ 'EAFNOSUPPORT', '/* Address family not supported by protocol */'],
	98 : [ 'EADDRINUSE', '/* Address already in use */'],
	99 : [ 'EADDRNOTAVAIL', '/* Cannot assign requested address */'],
	100 : [ 'ENETDOWN', '/* Network is down */'],
	101 : [ 'ENETUNREACH', '/* Network is unreachable */'],
	102 : [ 'ENETRESET', '/* Network dropped connection because of reset */'],
	103 : [ 'ECONNABORTED', '/* Software caused connection abort */'],
	104 : [ 'ECONNRESET', '/* Connection reset by peer */'],
	105 : [ 'ENOBUFS', '/* No buffer space available */'],
	106 : [ 'EISCONN', '/* Transport endpoint is already connected */'],
	107 : [ 'ENOTCONN', '/* Transport endpoint is not connected */'],
	108 : [ 'ESHUTDOWN', '/* Cannot send after transport endpoint shutdown */'],
	109 : [ 'ETOOMANYREFS', '/* Too many references: cannot splice */'],
	110 : [ 'ETIMEDOUT', '/* Connection timed out */'],
	111 : [ 'ECONNREFUSED', '/* Connection refused */'],
	112 : [ 'EHOSTDOWN', '/* Host is down */'],
	113 : [ 'EHOSTUNREACH', '/* No route to host */'],
	114 : [ 'EALREADY', '/* Operation already in progress */'],
	115 : [ 'EINPROGRESS', '/* Operation now in progress */'],
	116 : [ 'ESTALE', '/* Stale NFS file handle */'],
	117 : [ 'EUCLEAN', '/* Structure needs cleaning */'],
	118 : [ 'ENOTNAM', '/* Not a XENIX named type file */'],
	119 : [ 'ENAVAIL', '/* No XENIX semaphores available */'],
	120 : [ 'EISNAM', '/* Is a named type file */'],
	121 : [ 'EREMOTEIO', '/* Remote I/O error */'],
	122 : [ 'EDQUOT', '/* Quota exceeded */'],
	123 : [ 'ENOMEDIUM', '/* No medium found */'],
	124 : [ 'EMEDIUMTYPE', '/* Wrong medium type */'],
	125 : [ 'ECANCELED', '/* Operation Canceled */'],
	126 : [ 'ENOKEY', '/* Required key not available */'],
	127 : [ 'EKEYEXPIRED', '/* Key has expired */'],
	128 : [ 'EKEYREVOKED', '/* Key has been revoked */'],
	129 : [ 'EKEYREJECTED', '/* Key was rejected by service */'],
	130 : [ 'EOWNERDEAD', '/* Owner died */'],
	131 : [ 'ENOTRECOVERABLE', '/* State not recoverable */']
}


def smart_tooltips(sym):
	if sym == None or len(sym.tooltip()):
		return
	var = sym
	type = sym.type()
	if type.is_pointer():
		type = type.pointed_type()
		try:
			sym = sym.children()[0]
		except:
			sym = None

	if sym and type.as_class():
		tip = "{ "
		for field in sym.children():
			if len(tip) > 64:
				tip += "..."
				break;
			tip += field.name() + "=" + field.value() + " "
		tip += " }"
		var.set_tooltip(tip)	


################################################################
# 
# entry point
# 
def on_debug_symbol(sym):
	"""
	Debugger callback, this is the script's entry point
	"""

	#print "on_debug_symbol:",sym.name(), sym.type().name(),sym.is_expanding()

	if sym:
		if sym.name() == "errno":
			try:
				tip = errno[int(sym.value())]
				sym.set_tooltip(" ".join(tip))
			except KeyError:
				sym.set_tooltip("")
			except:
				pass
			return None

		# handle QString
		if sym.type().name() == "QString":
			return qstring_as_cstr(sym)

		# handle STL containers
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
			} [type](sym)
		except KeyError:
			pass

		smart_tooltips(sym)
		return sym

	return None


