# vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
# $Id$
#
################################################################
# Test calling overloaded operators in the debugged program
################################################################
function test_oper_call()
{
echo ----- ${FUNCNAME}${1:-$debug} ----- >> $logfile
rm -f $config a.out
cat > ./foo.cpp << '---end---'
////////////////////////////////////////////////////////////////
// foo.cpp
struct Fu
{
    int i_;

    explicit Fu(int i = 0) : i_(i) {}

    Fu& operator++() { ++i_; return *this; }
    
    Fu operator++(int) { int i = i_++; return Fu(i); }

    const Fu* operator->() const { return this; }

    const Fu& operator*() const { return *this; }

    int operator[](unsigned n) { return 42 + n; }

    float operator()(short x) { return x * 3.14; }
};

int main()
{
    Fu f;
    ++f;
    f++;
    f->i_;
    *f;
    f[3];
    f(2);
}
---end---

#write test script (processed by the AutoTest plugin)
cat > ./script << '---end---'
echo ##### test_oper_call #####
call { exec a.out }
call { break main }
call continue
call next
call next
call ++f
call f->i_
expect { 1
}
call ++f
call f->i_
expect { 2
}
call (*f).i_
expect { 2
}
call f[0]
expect { 42
}
call f[-1]
expect { 41
}
call f[3]
expect { 45
}
call f(2)
expect { 6.28
}
call quit
---end---

build ${1:-$debug} foo.cpp
run_debugger
}

function run()
{
    source common.sh
	if [ "$compiler" = "icc" ]
	then test_oper_call -g
	else
		test_oper_call -gdwarf-2
		test_oper_call -gstabs+
	fi
}

source suffix.sh
