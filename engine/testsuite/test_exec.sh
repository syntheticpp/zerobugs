# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_exec.sh 205 2007-11-22 03:52:59Z root $

################################################################
# Test executing a program from the command line, and passing
# arguments to it.
################################################################
function test_exec()
{
rm -f a.out foo bar $config
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
cat > foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
int main(int argc, char* argv[])
{
    return 0;
}
---end---
#write test script (processed by the AutoTest plugin)
cat > script << '---end---'
call next
call argc
expect { 3
}
call { argv[0] }
expect { "a.out"
}
call { argv[1] }
expect { "foo"
}
call { argv[2] }
expect { "--bar"
}
call quit
---end---

#compile
rm -f a.out
build ${1:-$debug} foo.cpp

rm -f $config

# FIXME
#if ! ${path}bin/zero $opts --main a.out foo --foo -- --bar
#    then exit 1
#fi
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
        test_exec -g
    else
        test_exec -gstabs
        test_exec -gstabs+
        test_exec -gdwarf-2
    fi
}

source suffix.sh
