static void func();

int main()
{
    func();
}

void func()
{
    const char* this_func = "func";
    const char* __this_func = "func";

    const char*& r_str = __this_func;
    void* ptr = 0;
    int v_int = 0;
    int& r_int = v_int;

    unsigned int v_unsigned_int = 0;
    char vchar = 0;
    unsigned char v_unsigned_char = 0;
    long vlong_int = 0;
    unsigned long vlong_unsigned_int = 0;

    short v_short_int = 0;
    unsigned short v_short_unsigned_int = (++v_short_int)++;

    bool v_bool;

    v_int = ++vchar = v_unsigned_char = ++v_bool;
    v_unsigned_int = ++vlong_int = ++vlong_unsigned_int;

    long double v_long_double = 0;
    double v_double = 0;
    float v_float;

    v_long_double = ++v_double = v_float++;

#if __GNUC__ >= 3
    // --- test the const volatile qualifiers
    volatile int v_volatile_int = 0;

    v_volatile_int++;

    const char v_const_char = 'c';
    const volatile unsigned int v_const_volatile_unsigned_int =
        v_const_char + 1;
#endif
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
