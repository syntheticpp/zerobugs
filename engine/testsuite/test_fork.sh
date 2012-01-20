# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_fork.sh 512 2008-06-02 03:33:02Z root $
#
function test_fork()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f a.out $config
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
void child(int x)
{
    std::cout << x << " fork=" << getpid() << std::endl;
    sleep(1);
}
int main()
{
    pid_t pid = getpid();
    std::cout << "pid=" << pid << std::endl;

    if ((pid = fork()) == 0)
    {
        child(42);
    }
    else
    {
        std::cout << "PID=" << pid << std::endl;  
        waitpid(pid, 0, 0);
    }
    return 0;
}
---end---

build $debug $@ foo.cpp

#write test script for the AutoTest plugin
cat > ./script << '---end---'
call { break child }

call continue
call next
call { eval x }
expect { 42
}
call { show threads /count }
expect { 2
}
call quit
---end---

#finally, run the test
run_debugger --main --trace-fork a.out
}

################################################################
# Run this test standalone
################################################################
function run()
{
    source common.sh
    #test_fork -gdwarf-23 -feliminate-dwarf2-dups 
    test_fork -gdwarf-2
    test_fork -gstabs+
}

# run this test standalone
source suffix.sh
