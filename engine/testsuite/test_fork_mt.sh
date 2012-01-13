# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id: test_fork_mt.sh 522 2008-06-13 06:44:13Z root $
#
function test_fork_mt()
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

build $debug $@ foo.cpp -lpthread

#write test script for the AutoTest plugin
cat > ./script << '---end---'
call { break child }

call continue
call next
###call test.cancel
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
    #test_fork_mt -gdwarf-2 -feliminate-dwarf2-dups 
    test_fork_mt -gstabs+
    test_fork_mt -gdwarf-2
}

# run this test standalone
source suffix.sh
