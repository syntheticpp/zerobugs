# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: common.sh 719 2010-10-22 03:59:11Z root $
#
# Variables and functions shared accross test modules
#
if [ -n "$COMMON" ]; then 
    echo "already sourced"
    echo ${BASH_SOURCE[*]}
    exit
fi
uname -a
export COMMON=1

#some of the core-related tests need a core file
#to be generated in the current dir
#echo "core.%p" > /proc/sys/kernel/core_pattern 2>/dev/null

# Workaround broken __mt_alloc
export GLIBCXX_FORCE_NEW=1
export GLIBCPP_FORCE_NEW=1

path="../../"
export PATH=$PATH:/opt/intel/cce/9.1.042/bin
if test -z $LD_LIBRARY_PATH; then
    LD_LIBRARY_PATH="."
fi
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib64
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${path}lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/intel/cce/9.1.042/lib

ARCH=`uname -m`
KVER=`uname -a | cut -f3 -d' '`

if [ $ARCH = "x86_64" ]
then
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib64/:.
fi
debug=-g
logfile=testsuite.log
outfile=testsuite.out
config=.zero/config

#opts="--no-hw-bp --ui-disable --test=script --log=$logfile --no-banner"
#opts="--ui-disable --test=script --log=$logfile --no-banner --cache-disable"
opts="--ui-disable --test=script --log=$logfile --no-banner"

compiler=g++
#compiler=/usr/lib/gcc-snapshot/bin/g++
#compiler="icc"

if [ "$compiler" = icc ]; then 
    LDFLAGS+=-i-static
fi

#GCCVER="`$compiler --version 2>&1 | head -1 | cut -f3 -d' ' | cut -f1 -d'.'`"
#GCCVER="`$compiler --version 2>&1 | head -1 | cut -f2 -d')' | cut -f1 -d'.'`"
GCCVER="`$compiler -dumpversion | cut -f1 -d'.'`"
GCC_BUGGY_STABS=`$compiler -dumpversion | grep 3.4.4`
echo $GCC_BUGGY_STABS
echo compiler=$GCCVER


#export ZERO_PLUGIN_PATH=.:${path}plugin
export ZERO_PLUGIN_PATH=../../plugin/autotest/:${path}plugin
export ZERO_CONFIG_PATH=`pwd`
export ZERO_QUICK_SHUTDOWN=false
#export ZERO_QUICK_SHUTDOWN=true

#run tests faster by not saving the settings
export ZERO_SAVE_STATE=false
#export ZERO_USE_FRAME_HANDLERS=false
#export ZERO_USE_FRAME_HANDLERS=true
export ZERO_HARDWARE_BREAKPOINTS=false
#export ZERO_CACHE_SYMBOLS=true

#export ZERO_LAZY_SYMBOLS=false
#export ZERO_ALWAYS_SCAN_VTABLES=1
#export ZERO_AUTO_RTTI=true

#export MALLOC_CHECK_=2

function suspend()
{
	echo ===== Stopping $$ =====
	kill -s STOP $$
}

################################################################
# Run the debugger in test mode
################################################################
function run_debugger()
{
	if [ "$USE_VALGRIND" = yes -o "$USE_VALGRIND" = true ]; then
		RUN_DEBUGGER="run_valgrind ${path}bin/zero"
	elif [ "$USE_INSTALLED" = yes ]; then
		RUN_DEBUGGER=/usr/local/bin/zero-bin
	else
		RUN_DEBUGGER="${path}bin/zero"
	fi
	#echo "$RUN_DEBUGGER"

	#wipe history and config so that we don't accidentally
	#pick up any breakpoints etc from a previous test case
	rm -rf .zero

	if ! $RUN_DEBUGGER $opts $@; then 
		exit 1
    fi
}

function run_valgrind()
{
    #NODLCLOSE=1 valgrind -v \
	#	--db-attach=yes 	\
	#	--leak-check=full 	\
	#	--num-callers=32 	\
	#	--db-command="../../z %p" $@
	
	vopt="--log-file=log --leak-check=full --num-callers=64"
    NODLCLOSE=1 valgrind -v $vopt $@
}

################################################################
#
################################################################
function build()
{
    if [ -n "$STL_LIB" ]; then 
		CFLAGS="$CFLAGS $STL_INCLUDE_PATH -l$STL_LIB"
    fi
    rm -f a.out 
    if ! ${compiler} $@ $CFLAGS
        then exit 1
    fi
}


function error()
{
	echo "Test failed"
	exit 1
}
#trap suspend SIGABRT SIGSEGV
trap error ERR 
