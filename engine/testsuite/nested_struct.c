struct Employee
{
    long code;

    struct
    {
        const char* first;
        const char* last;
    } /* name */;

    unsigned age;
};


int main(int argc, char* argv[])
{
    struct Employee emp;

    //emp.name.first = "John";
    //emp.name.last = "Doe";
    emp.first = "John";
    emp.last = "Doe";
    emp.code = 123;
    emp.age = 35;

    return 0;
}

    
