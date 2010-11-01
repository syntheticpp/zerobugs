#! /bin/bash
# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_32on64.sh 379 2008-03-04 10:11:11Z root $
# Automated tests for the debugger engine
# Copyright (c) 2004, 2006 Cristian L. Vlasceanu
#
source common.sh
source test_add_mod.sh
source test_array.sh
source test_arg.sh
source test_anon_struct.sh
source test_attach.sh
source test_attach2.sh
source test_bits.sh
source test_bitfields.sh
source test_break.sh
source test_brkpnt.sh
source test_cast.sh
source test_cond_break.sh
source test_const.sh
source test_darray.sh
source test_dl_core.sh
source test_dso.sh
source test_env.sh
source test_exec.sh
source test_expr.sh
source test_expr_lazy.sh
source test_frame.sh
source test_inherit.sh
source test_interface.sh
source test_macro.sh
source test_member.sh
source test_namespace.sh
source test_ptr_call.sh
source test_recursive.sh
source test_ret_fun.sh
source test_ret_obj.sh
source test_ret_val.sh
source test_strcmp.sh
source test_core.sh
source test_core_env.sh
source test_lookup.sh
source test_nested_struct.sh
source test_rtti.sh
source test_dwarf_reg.sh
source test_fork.sh
source test_forkexec.sh
source test_fun_call.sh
source test_oper_call.sh
source test_segv.sh
source test_segv_2.sh
source test_sigtrap.sh
source test_shared_ptr.sh
source test_static_const.sh
source test_symbol_map.sh
source test_threads.sh
source test_throw.sh
source test_virt_call.sh
source test_watch.sh

declare -a TESTCASE
#set compiler:=${compiler} -m32
export CFLAGS="$CFLAGS -m32"

################################################################
#
################################################################
function add_test()
{
	TESTCASE[${#TESTCASE[@]}]=$1
}
################################################################
#
################################################################
function add_common_test()
{
	stabs_plus=0 #requires -gstabs+?
	case $1 in 
	*,stabs+) stabs_plus=1;; #test requires -gstabs+
	esac
	T=${1/,stabs+/}
	if [ "$compiler" = "icc" ]
		then add_test "$T -g"
	else 
		if [ $stabs_plus = 0 ]
			then add_test "$T -gstabs"
		fi
		add_test "$T -gstabs+"
		add_test "$T -gdwarf-2"
		#add_test "$T -gdwarf-2 -feliminate-dwarf2-dups"
	fi
}
################################################################
#
################################################################
function run_tests()
{
	rm -f $outfile
	rm -f $logfile
	touch $outfile
	sz=${#TESTCASE[@]}	#size of test array
	n=1					#count of tests executed so far
	perc=0
	monitor=0
	if [ -n "$DISPLAY" ]
	then xterm -title "Test Suite Monitor" \
		-bg black -fg green -e tail -f $outfile &
		monitor=`jobs -p`
	fi
	for TEST in "${TESTCASE[@]}"
	do
		echo "Completed: ${n}/${sz}	[${perc}%]	Running: $TEST"

		if $TEST >> $outfile 2>&1
			then :		#success
			else echo -e "\\nTest failed\n"; exit 1
		fi
		let n=$n+1
		let perc=`expr \( $n \* 100 \) / $sz`
	done
	let n=$n-1
	if [ "$monitor" -gt 0 ]; then
		kill $monitor
	fi
	echo -e "\\n$n tests done."
}


################################################################
# Common tests, each may be run several times with different
# compiler options.
# Tests marked with stabs+ do not work with -gstabs, they
# require -gstabs+
################################################################
COMMON_TESTS=(
	test_add_mod
	test_array,stabs+
	test_arg
	test_bits,stabs+
	test_bitfields
	test_break
    test_brkpnt
	test_cast,stabs+
    test_cond_break
	test_cstyle_cast
	test_const
	test_core_env
	test_darray
	test_dl_core
	test_env
	test_exec
	test_expr_eval,stabs+
    test_expr_lazy
	test_frame
	test_function_call,stabs+
	test_inherit,stabs+
	test_interface,stabs+
	test_lookup
	test_nested_struct
	test_member_cast
	test_member_data
	test_namespace,stabs+
	test_oper_call,stabs+
	test_ptr_call,stabs+
	test_recursive
	test_ret_fun
	test_ret_obj
	test_ret_val
	test_strcmp
	test_segv
	test_segv_2
	test_sigtrap
	test_shared_ptr,stabs+
	test_symbol_map
	test_threads,stabs+
# known to fail with GCC 2.95 
#	test_virt_call,stabs+

# this test creates problems on Virtual PC
#	test_watchpoint,stabs+
)
LINUX_TESTS=(
	test_rtti
	test_rtti_with_templates
)
LINUX26_TESTS=(
	#test_attach
	#test_attach2
	#test_frame
	test_fork
	test_forkexec 
)

for T in "${COMMON_TESTS[@]}"
do
	add_common_test $T
done
if test $GCCVER -gt 2
	then :
	    add_common_test test_anon_struct
		add_common_test test_core
		add_common_test test_dso
		add_common_test test_throw
		add_common_test test_virt_call,stabs+
fi 
if test `uname` = "Linux"
then
	for T in "${LINUX_TESTS[@]}"
	do
		add_common_test $T
	done
	case $KVER in
	2.6*) for T in "${LINUX26_TESTS[@]}"
			do 
				add_common_test $T
			done;;
	*) echo kernel $KVER not likely to support ptrace options;;
	esac
fi

if [ "$compiler" = "icc" ]
	then 
		add_test "test_macro -g3"
		add_test "test_static_const -g"
	else 
		add_test "test_macro -gdwarf-23"

        if test $GCCVER -gt 2
		then 
            add_test "test_static_const -gdwarf-2"
        fi
fi


#todo
#add_test test_dwarf_reg -O0
#add_test test_dwarf_reg -O1

run_tests
