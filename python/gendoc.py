import zero
import pydoc
import sys

def on_init(debugger):
	f = open("zero.html", "w")
	f.write("<html><title>Zero Python Documentation</title><body>")
	doc = pydoc.HTMLDoc()
	f.write(doc.docmodule(zero))
	f.write("</body></html>")
	f.close()
	sys.exit(0)
