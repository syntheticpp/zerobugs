// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "dharma/system_error.h"
#include "zdk/argv_util.h"
#include "autotest.h"


using namespace std;
using namespace boost;


void query_plugin(InterfaceRegistry* registry)
{
    registry->update(DebuggerPlugin::_uuid());
}


int32_t query_version(int32_t* minor)
{
    if (minor) *minor = ZDK_MINOR;
    return ZDK_MAJOR;
}

Plugin* create_plugin(uuidref_t iid)
{
    Plugin* plugin = 0;

    if (uuid_equal(DebuggerPlugin::_uuid(), iid))
    {
        plugin = new AutoTest();
    }
    return plugin;
}


static string logname()
{
    ostringstream ss;

    ss << "testlog." << getpid();
    return ss.str();
}


void trim_trailing_spaces(string& s)
{
    while (!s.empty())
    {
        size_t n = s.size() - 1;

        if (s[n] == ' ')
        {
            s.erase(n);
        }
        else
        {
            break;
        }
    }
}



AutoTest::AutoTest()
    : debugger_(0)
    , passedCount_(0)
    , execCount_(0)
    , cmdCount_(0)
    , debug_(false)
    , verbose_(false)
{
    cout << "========== AutoTest Plugin Loaded =========" << endl;
}


AutoTest::~AutoTest() throw()
{
    cout << "========== AutoTest Plugin UnLoading =========" << endl;
}


void AutoTest::release()
{
    delete this;
}


bool AutoTest::initialize(Debugger* debugger, int* ac, char*** av)
{
    debugger_ = debugger;

    const char* logFileName = NULL;
    string filename;

BEGIN_ARG_PARSE(ac, av)
    ON_ARGV("--test=", filename)
        {
            assert(!filename.empty());
            read_script(filename);
            cmdCount_ = commands_.size();
        }
    ON_ARGV("--log=", logFileName)
        {
            log_.open(logFileName, ios::out | ios::app);
        }
    ON_ARG("--debug-test")
        {
            debug_ = true;
        }
    ON_ARG("--log-verbose")
        {
            verbose_ = true;
        }
END_ARG_PARSE

    if (!logFileName)
    {
        log_.open(logname().c_str());
    }
    if (!log_)
    {
        throw runtime_error("could not open file: " + string(logFileName));
    }
    return true;
}


void AutoTest::shutdown()
{
    print_stats();
}


void AutoTest::register_streamable_objects(ObjectFactory*)
{
}


void AutoTest::on_table_init(SymbolTable*)
{
}


void AutoTest::on_table_done(SymbolTable*)
{
}


void AutoTest::on_attach(Thread*)
{
}


void AutoTest::on_detach(Thread* thread)
{
    if (thread == 0)
    {
        // detached from all threads
    }
}


bool AutoTest::on_message(const char* msg, Debugger::MessageType type, Thread*, bool)
{
    if (type == Debugger::MSG_ERROR)
    {
        cerr << "*** Critical error: " << msg << endl;
    }
    return false;
}


bool AutoTest::on_event(Thread* thread, EventType)
{
    if (!commands_.empty())
    {
        boost::shared_ptr<Command> cmd = commands_.front();

        if (cmd->input() == "quit") // hack!
        {
            ++execCount_;
            ++passedCount_;
            debugger_->quit();
            return false;
        }
        if (cmd->input() == "test.prompt")
        {
            return false;
        }
        if (cmd->input() == "test.cancel")
        {
            commands_.clear();
            cout << output_ << endl;
            return false;
        }
        else
        {
            ostringstream out;
            bool result = false;

            if (cmd->is_pending())
            {
                commands_.pop_front();
                result = cmd->compare(out, false, thread);
            }
            else
            {
                result = cmd->run(out, debugger_, thread);
                if (cmd->is_pending())
                {
                    return true;
                }
                else
                {
                    commands_.pop_front();
                }
                //clog << "*** " << boolalpha << result << endl;
            }
            if (result)
            {
                ++passedCount_;
            }
            ++execCount_;

            clog << out.str();
            log_ << out.str() << flush;
            if (verbose_)
            {
                log_ << cmd->output();
            }
            output_ += cmd->output();
            if (!cmd->assign_to().empty())
            {
                string out = cmd->output();

                //trim_trailing_spaces(out);
                // clog << "\"" << out << "\"" << endl;
                vars_[cmd->assign_to()] = out;
            }

            if (!result && debug_)
            {
                // same as an explicit test.cancel commamnd
                commands_.clear();
                cout << output_ << endl;
                return false;
            }
        }
        return true;
    }
    return false;
}


void AutoTest::on_program_resumed()
{
}


void AutoTest::on_insert_breakpoint(volatile BreakPoint*)
{
}


void AutoTest::on_remove_breakpoint(volatile BreakPoint*)
{
}


bool AutoTest::on_progress(const char*, double, word_t)
{
    return true;
}


static void
parse_error(const string& filename, size_t line, const string& token)
{
    cerr << filename << ':' << line << ": unexpected `" << token << "'" << endl;
    _exit(-1);
}


#define BEGIN_COMMAND() do { \
    state = CALL_BEGIN; \
    cmd.clear(); \
    if (command) \
        commands_.push_back(command); \
    boost::shared_ptr<Command>(new Command(vars_)).swap(command);\
    assert(command.get()); \
    command->assign_to(var); \
    var.clear(); } while (0)

#define SET_EXPECT() set_expect(arch, state, command, expect)

namespace
{
    enum State
    {
        START,
        CALL_BEGIN,
        CALL_BLOCK,
        CALL_END,
        EXPECT_BEGIN,
        EXPECT_BLOCK,
        EXPECT_END,
        ECHO,
        ECHO_END,
        VAR_BEGIN,
    };

    enum Arch
    {
        ARCH_ALL,
        ARCH_32,
        ARCH_64
    };
}


static void set_expect(State& state, string& expect)
{
    state = EXPECT_END;
    trim_trailing_spaces(expect);
}



static void
set_expect(Arch& arch, State& state, boost::shared_ptr<Command> command, string& expect)
{
    set_expect(state, expect);

    assert(command);
    switch (arch)
    {
    case ARCH_ALL:
        command->set_expect(expect);
        break;

    case ARCH_32:
        command->set_expect32(expect);
        break;

    case ARCH_64:
        command->set_expect64(expect);
        break;
    }
    arch = ARCH_ALL;
}



/**
 * Read and parse a script file, throw runtime_error
 * on unexpected tokens
 */
void AutoTest::read_script(const string& filename)
{
    ifstream script(filename.c_str());

    script.unsetf(ios::skipws); // do not skip blank spaces

    size_t line = 1;

    bool   newLine = true, comment = false;
    string token, cmd, expect, echo, var;
    State  state = START;
    Arch   arch = ARCH_ALL;

    boost::shared_ptr<Command> command;

    for (char c = 0; (script >> c); )
    {
        if (!isspace(c))
        {
            if (newLine && c == '#')
            {
                comment = true;
            }
            if (!comment)
            {
                token += c;
            }
            newLine = false;
            continue;
        }

        assert(isspace(c));

        if (token.empty())
        {
            if (!comment)
            {
                if (state == ECHO)
                {
                    echo += c;
                }
                else if (state == EXPECT_BLOCK)
                {
                    expect +=c;
                }
            }
        }
        else
        {
            switch (state)
            {
            case EXPECT_END:
                if (token == "or")
                {
                    state = EXPECT_BEGIN;
                    arch = ARCH_ALL;
                    break;
                }
            /* todo:
                else if (token == "expect-i386")
                {
                    state = EXPECT_BEGIN;
                    arch = ARCH_32;
                    //expect.clear();
                }
                else if (token == "expect-x64")
                {
                    state = EXPECT_BEGIN;
                    arch = ARCH_64;
                    //expect.clear();
                }
            */
                else if (command)
                {
                    commands_.push_back(command);
                    command.reset();
                }
                // fallthru

            case START:
                var.clear();
                // fallthru

            case ECHO_END:
                if (token == "call")
                {
                    BEGIN_COMMAND();
                }
                else if (token == "echo")
                {
                    state = ECHO;
                    echo.clear();
                }
                else
                {
                    var = token;
                    state = VAR_BEGIN;
                }
                break;

            case VAR_BEGIN:
                if (token == "=" )
                {
                    BEGIN_COMMAND();
                }
                else
                {
                    parse_error(filename, line, token);
                }
                break;

            case CALL_BEGIN:
                if (token == "{" || token == "(")
                {
                    state = CALL_BLOCK;
                }
                else
                {
                    state = CALL_END;
                    assert(command);
                    command->set_input(token);
                }
                break;

            case CALL_BLOCK:
                if (token == "}" || token == ")")
                {
                    state = CALL_END;
                    assert(command);
                    command->set_input(cmd);
                }
                else
                {
                    if (!cmd.empty() && cmd[cmd.size() - 1] != '\n')
                    {
                        cmd += ' ';
                    }
                    cmd += token;
                }
                break;

            case CALL_END:
                if (token == "call")
                {
                    BEGIN_COMMAND();
                }
                else if (token == "expect")
                {
                    state = EXPECT_BEGIN;
                    arch = ARCH_ALL;
                    expect.clear();
                }
                else if (token == "expect-i386")
                {
                    state = EXPECT_BEGIN;
                    arch = ARCH_32;
                    expect.clear();
                }
                else if (token == "expect-x64")
                {
                    state = EXPECT_BEGIN;
                    arch = ARCH_64;
                    expect.clear();
                }
                else if (token == "echo")
                {
                    assert(command);
                    commands_.push_back(command);
                    command.reset();

                    state = ECHO;
                }
                else
                {
                    var = token;
                    state = VAR_BEGIN;
                }
                break;

            case EXPECT_BEGIN:
                if (token == "{" || token == "(")
                {
                    state = EXPECT_BLOCK;
                    expect.clear();
                }
                else
                {
                    SET_EXPECT();
                }
                break;

            case EXPECT_BLOCK:
                if (token == "}" || token == ")")
                {
                    SET_EXPECT();
                }
                else
                {
                    expect += token;
                    expect += c;
                }
                break;

            case ECHO:
                echo += token;
                echo += c;
                break;
            }
        }

        if (c == '\n')
        {
            ++line;
            newLine = true;

            if (!comment)
            {
                switch (state)
                {
                case ECHO:
                    state = ECHO_END;

                    if (!command)
                    {
                        boost::shared_ptr<Command>(new Command(vars_)).swap(command);
                    }
                    command->add_echo(echo);
                    echo.clear(); // echo arguments end at end of line
                    break;

                default: // silence off compiler warning
                    break;
                }
            }
            comment = false;
        }
        token.clear();
    } // end for

    if (command)
    {
        commands_.push_back(command);
    }
}


void AutoTest::print_stats()
{
    ostringstream out;

    out << "*** AutoTest Results ***\n";
    out << "Commands: " << cmdCount_ << endl;
    out << "Executed: " << execCount_ << endl;
    out << "Passed  : " << passedCount_ << endl;

    log_ << out.str() << flush;
    clog << out.str() << flush;

    if (passedCount_ != cmdCount_)
    {
        _exit(1);
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
