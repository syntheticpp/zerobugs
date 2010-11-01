struct X
{
    wchar_t var_wchar_t;
    X* next;
};


int main()
{
    X x1, x2;

    x1.var_wchar_t = 'A';
    x2.var_wchar_t = 'B';

    x2.next = 0;
    x1.next = &x2;

    return 0;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
