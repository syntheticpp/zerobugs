# run this test standalone
if test $# -gt 0; then
	if test $1 = "--run"; then 
		run $@
	elif test $1 = "--run32"; then
		CFLAGS+=-m32 run $@
	else 
		echo $1
	fi
fi
