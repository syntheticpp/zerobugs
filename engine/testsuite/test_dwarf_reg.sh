# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$
#
# Test DWARF registers

function test_dwarf_reg()
{
echo ----- ${FUNCNAME}${debug} ----- >> $logfile
rm -f a.out $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
int fun(int a, int b, int c)
{
    if (a > 100)
    {
        register int d = a + b + c;
        return d / 2;
    }
    return 0;
}
int main()
{
    int a[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    int b = 1000;

    for (int i = 0; b > 100; ++i)
    {
        b = fun(b, b / 2, a[i]);
    }
}
---end---

#if [ "$compiler" = "icc" ]
#then
#    build $@ -g foo.cpp
#else
#    build $@ -gdwarf-2 foo.cpp
#fi
build $@ -gdwarf-2 foo.cpp

#write test script for the AutoTest plugin
cat > ./script << '---end---'
call { exec a.out }
call { break main }

call continue
call next
call next
call next
call next

call { eval i }
expect { 0
}
call { eval b }
expect { 1000
}
call step
call next
call next
call next
call { eval d }
expect { 1501
}
call { eval &d }
expect { NULL
}
call quit
---end---
#finally, run the test
run_debugger
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh

    if [ "$compiler" = "icc" ]
    then    
       	echo todo: icc test 
    else
        test_dwarf_reg -O0
        test_dwarf_reg -O1
    fi
}

# run this test standalone
source suffix.sh
