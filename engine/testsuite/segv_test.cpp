/* Test Case for stepping into signal handlers
 */
 
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <iostream>

static jmp_buf ret_from_segv;

static void ignore_segv(int)
{
    std::clog << __func__ << std::endl;
    longjmp (ret_from_segv, 1);
    //siglongjmp (ret_from_segv, 1);
}

void set_handler()
{
    sighandler_t old = signal( SIGSEGV, ignore_segv );
    std::clog << __func__ << ": old=" << (void*)old << std::endl;
    std::clog << "new handler=" << (void*)&ignore_segv << std::endl;
}

int main()
{
#if 1
    struct sigaction sa = { 0 };
    sa.sa_handler = ignore_segv;
    sa.sa_flags = SA_NODEFER | SA_RESTART;
    //sigaction( SIGSEGV, &sa, NULL );
#else
    set_handler();
#endif
    if (setjmp( ret_from_segv ) == 0)
    {
        *(int*)1 = 1; 
    }

    std::clog << __func__ << std::endl;
    return 0;
}
