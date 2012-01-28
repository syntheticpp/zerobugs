
struct X
{
    virtual ~X() {}
    virtual int f_1(char*) { return 42; }
    virtual void f_2(int, long) {}
};

int main()
{
    X x;
    x.f_2(0, 0);

    return x.f_1("blah");
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
