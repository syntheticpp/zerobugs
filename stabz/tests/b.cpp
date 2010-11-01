#include "a.h"
#include "b.h"

B::B() : name_("class B") {}

int main()
{
    B b;
    A a(b);

    return 0;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
