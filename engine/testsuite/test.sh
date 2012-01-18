#! /bin/bash
# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test.sh 720 2010-10-28 06:37:54Z root $
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
source test_double_reg.sh
source test_dso.sh
source test_dso_debug.sh
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
source test_path_map.sh
source test_ptr_call.sh
source test_recursive.sh
source test_ret_fun.sh
source test_ret_obj.sh
source test_ret_val.sh
source test_core.sh
source test_core_env.sh
source test_lookup.sh
source test_longjmp.sh
source test_nested_struct.sh
source test_rtti.sh
source test_dwarf_reg.sh
source test_fork.sh
source test_fork_mt.sh
source test_forkexec.sh
source test_forkexec_mt.sh
source test_fun_call.sh
source test_oper_call.sh
source test_ref_ptr.sh
source test_segv.sh
source test_segv_2.sh
source test_sigtrap.sh
source test_shadow.sh
source test_shared_ptr.sh
source test_static.sh
source test_static_const.sh
source test_step.sh
source test_strcmp.sh
source test_symbol_map.sh
source test_threads.sh
source test_threads_stop.sh
source test_throw.sh
source test_vector.sh
source test_virt_call.sh
source test_watch.sh

declare -a TESTCASE

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
    buggy_stabs=1
    if test -z "$GCC_BUGGY_STABS"; then
        buggy_stabs=0
    fi    
    T=$1
    while true; do
        case $T in 
        *,no-ppc*)
            if test "$ARCH" = "ppc"; then
                return
            fi
            T=${T/,no-ppc*/}
            ;;
        *,stabs+*) #test does require -gstabs+
            stabs_plus=1; 
            T=${T/,stabs+*/}
            ;;
        *) break
            ;;
        esac
    done
    if test "$compiler" = "icc"; then
        add_test "$T -g"
    else 
        if test $buggy_stabs = 0; then
                if test $stabs_plus = 0; then 
                    #okay to add simple stabs test
                    add_test "$T -gstabs"
                fi
                add_test "$T -gstabs+"
        fi;
        add_test "$T -gdwarf-2"

        #add_test "$T -gdwarf-2 -feliminate-dwarf2-dups"
    fi
}
################################################################
#
################################################################
function run_tests()
{
    ldd ${path}plugin/zdwarf.so | grep libdwarf

    rm -f $outfile
    rm -f $logfile
    touch $outfile
    sz=${#TESTCASE[@]}  #size of test array
    let n=1             #count of tests executed so far
    let perc=0
    let monitor=0
    if [ -n "$DISPLAY" ]; then 
        xterm -title "Test Suite Monitor" \
            -bg black -fg green -e tail -f $outfile&
        #monitor=`jobs -p`
        monitor=$!
    fi
    
    for TEST in "${TESTCASE[@]}"
    do
        echo "Completed: ${n}/${sz} [${perc}%]      Running: $TEST"
        echo "Completed: ${n}/${sz} [${perc}%]      Running: $TEST" >> $outfile

        if ($TEST >> $outfile 2>&1)
            then :
            else echo -e "\\nTest failed\n"; exit 1
        fi
        let n=$n+1
        let perc=`expr \( $n \* 100 \) / $sz`
    done
    let n=$n-1

    if test "$monitor" -gt 0; then
        kill $monitor
    fi
    echo -e "\\n$n tests done."
    #killall -9 a.out zero 2>/dev/null
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
    test_arg,stabs+,no-ppc
    test_bits,stabs+
    test_bitfields
    test_break
    test_brkpnt
    test_cast,stabs+,no-ppc
    test_cond_break
    test_cstyle_cast
# known to fail with GCC 2.95, moved to GCCVER -gt 2 section
#   test_core
    test_const
    test_core_env
    test_darray
    test_dl_core
    test_env
    test_exec
    test_expr_eval,stabs+
    test_expr_lazy,no-ppc
    test_frame
    test_function_call,stabs+,no-ppc
    test_inherit,stabs+,no-ppc
#broken on 4.3.0
#   test_interface,stabs+
    test_lookup
	test_longjmp
#Fails with DWARF / GCC 3.2.2, GCC 3.3.5
    test_nested_struct
    test_member_cast
    test_member_data
    test_namespace,stabs+,no-ppc
    test_oper_call,stabs+,no-ppc
    test_path_map
    test_ptr_call,stabs+,no-ppc
    test_recursive

#   test_ref_ptr,stabs+,no-ppc
    test_ret_fun
    test_ret_obj,no-ppc
    test_ret_val,no-ppc
#    test_strcmp,no-ppc
	test_step_over
    test_shadow,stabs+,no-ppc
    test_shared_ptr,stabs+,no-ppc
    test_symbol_map
    test_segv
    test_segv_2
    test_sigtrap,no-ppc
    test_threads,stabs+,no-ppc
    test_threads_stop,stabs+,no-ppc
#   test_threads_static,stabs+,no-ppc
# known to fail with GCC 2.95 
#   test_virt_call,stabs+
    test_vector

    # NOTE this test may create problems on Virtual PC
    test_watchpoint,stabs+,no-ppc
)
LINUX_TESTS=(
    test_rtti,no-ppc
    test_rtti_with_templates,no-ppc
)
LINUX26_TESTS=(
    #test_attach
    #test_attach2

    test_fork
    test_fork_mt
    test_forkexec 
	test_forkexec_mt 
)

for T in "${COMMON_TESTS[@]}"
do
    add_common_test $T
done
if test $GCCVER -gt 2; then
    add_common_test test_anon_struct
    add_common_test test_core,no-ppc
    add_common_test test_virt_call,stabs+,no-ppc
fi 
if test $GCCVER -gt 3; then
    add_common_test test_dso,no-ppc
    add_common_test test_dso_debug,no-ppc
    add_common_test test_throw,no-ppc
    add_common_test test_nested_struct
fi
if test `uname` = "Linux"
then
    for T in "${LINUX_TESTS[@]}"
    do
        add_common_test $T
    done
    case $KVER in
    2.6*|3.*) for T in "${LINUX26_TESTS[@]}"
            do 
                add_common_test $T
            done;;
    *) echo kernel $KVER not likely to support ptrace options;;
    esac
fi

if [ "$compiler" = "icc" ]; then 
    :
#note: fails with icc on x86_64
       add_test "test_macro -g3"
       add_test "test_static_const -g"

    else 
		# GCC 4.5 does not support dwarf-23, commenting out for now

        #if test "$ARCH" != "ppc"; then
        #    add_test "test_macro -gdwarf-23"
        #fi

    if test $GCCVER -gt 2; then 
        add_test "test_static -gdwarf-2"
        #add_test "test_static_const -gdwarf-2"
            if test "$ARCH" != "ppc"; then
        add_test "test_double_reg -gdwarf-2"
            fi
    fi
fi


add_test test_interface -gdwarf-2

#todo
#add_test test_dwarf_reg -O0
#add_test test_dwarf_reg -O1

run_tests
