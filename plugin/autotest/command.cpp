//
//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <unistd.h>
#include <iostream>
#include <sys/fcntl.h>
#include "zdk/zero.h"
#include "command.h"


using namespace std;

void trim_trailing_spaces(string& s);

static void subst(string& str, const Command::Var& vars)
{
    //clog << "\"" << str << "\"" << endl;
    //trim_trailing_spaces(str);
    size_t count = 0;

    for (size_t n, p = 0; p < str.size(); )
    {
        n = str.find('$', p);
        if (n >= str.length())
        {
            //clog << __func__ << ": no vars in " << str << endl;
            break;
        }
        p = str.find_first_of(" \t\n", n + 1);
        if (p != str.npos)
        {
            p -= n;
        }
        string v = str.substr(n, p);
    #ifdef DEBUG
        clog << __func__ << ": '" << v << "'\n";
    #endif
        Command::Var::const_iterator i = vars.find(v.substr(1));
        if (i != vars.end())
        {
            str.erase(n, p);
            str.insert(n, i->second);

            p = n + i->second.length();
            ++count;
        }
        else if (n != str.npos)
        {
            p = n + 1;
        }
    }
    //clog << "\"" << str << "\"" << endl;
/* #ifdef DEBUG
    if (count)
        clog << __func__ << "=" << str << endl;
#endif */
}


const string& Command::output()
{
    auto_file f(fdopen(fifo_.output(), "r"));
    for (int c = 0; f.get() && ((c = fgetc(f.get())) != EOF); )
    {
        output_ += (char)c;
    }
  /*
    if (!output_.empty() && output_[output_.size() - 1] == '\n')
    {
        output_.erase(output_.size() - 1);
    }
  */
    return output_;
}


bool Command::run(ostream& log, Debugger* debugger, Thread* thread)
{
    subst(echo_, var_);
    log << echo_;
    bool error = false;
    bool res = false;
    {   // redirect scope
        auto_ptr<Redirect> redirect(new Redirect(STDOUT_FILENO, fifo_.input()));

        if (!input_.empty()) try
        {
            subst(input_, var_);
            res = debugger->command(input_.c_str(), thread);
        }
        catch (exception& e)
        {
            //cerr << "error: " << e.what() << endl;
            error = true;
            cout << e.what() << endl;
        }
        cout << flush;

        if (res)
        {
            // Some commands -- particularly "eval" may need the debugged
            // program to run and the result comes in on the next event;
            // hence the need for all this pending business.
            // NOTE that the limitation here is that we don't work with
            // expressions that need more than one event cycle to  complete
            // (such as fun(a,b) + fun(b,c) -- which evaluates 2 function calls).
            pending_ = true;
            redirect_ = redirect;
        }
    }
    debugger->resume(res);

    if (!res)
    {
        res = compare(log, error, thread);
    }
    if (!res && !error)
    {
        debugger->command("list", thread);
        debugger->command("frame", thread);
        // debugger->command("show regs /debug", thread);
    }
    return res;
}


bool
Command::compare(ostream& log, bool exceptionCaught, Thread* thread)
{
    bool result = compare(log, expectAll_, exceptionCaught);

    if (thread && thread->is_32_bit())
    {
        if (!expect32_.empty())
        {
            result &= compare(log, expect32_, exceptionCaught);
        }
        return result;
    }
    if (__WORDSIZE == 64)
    {
        if (!expect64_.empty())
        {
            result &= compare(log, expect64_, exceptionCaught);
        }
    }

    return result;
}


bool Command::compare(ostream& log,
                      vector<string>& expect,
                      bool exceptionCaught)
{
    vector<string>::iterator i = expect.begin();
    for (; i != expect.end(); ++i)
    {
        subst(*i, var_);
    }
    bool result = expect.empty();

    output();

    for (i = expect.begin(); i != expect.end(); ++i)
    {
        if (*i == output_)
        {
            result = true;
            break;
        }
    }
    if (exceptionCaught && expect.empty())
    {
        result = false;
    }
    if (!result)
    {
        log << "*** TEST FAILED: " << input_ << endl;
        log << "*** Expected:" << endl;

        for (i = expect.begin(); i != expect.end(); ++i)
        {
            log << "\"" << *i << "\"\n";
        }
        log << "*** Output was:\n\"" << output_ << "\""<< endl;
    }
    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
