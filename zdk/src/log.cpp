//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/log.h"
#include "zdk/mutex.h"
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <fstream>
#include <iomanip>
#include <iostream>

using namespace std;

static Mutex*           log_mutex = nullptr;
static int              log_verbosity = 0;
static ostream*         log_out = nullptr;
static bool             log_prefix = true;

////////////////////////////////////////////////////////////////
static off_t get_log_max_size()
{
    off_t max_size = 4096 * 1024;

    if (const char* size = getenv("ZERO_MAX_LOGSIZE"))
    {
        max_size = strtoul(size, 0, 0);
    }
    return max_size;
}

////////////////////////////////////////////////////////////////
static string file_err(

    string func,
    int         err,
    const char* op,
    const char* path )
{
    string error(func);

    error += ": failed to ";
    error += op;
    error += " file '";
    error += path;
    error += "': ";
    error += strerror(err);

    return error;
}

////////////////////////////////////////////////////////////////
static void write_prefix(

    ostream&        out,
    const char*     file,
    size_t          line )
{
    struct timeval tv = { 0 };
    gettimeofday(&tv, NULL);

    struct tm tinfo = { 0 };
    char timestr[100] = { 0 };

    strftime(
        timestr, sizeof(timestr) - 1,
        "%H:%M:%S",
        localtime_r(&tv.tv_sec, &tinfo));

    // print time with milliseconds
    out << timestr << '.' << setiosflags(ios::left);
    out << setfill('0') << setw(3) << tv.tv_usec / 1000;
    out << resetiosflags(ios::left);

    out << " [" << setfill(' ') << setw(24) << file << ':';
    out << setw(5) << line << "]: ";
}

////////////////////////////////////////////////////////////////
void Log::init(const char* path)
{
    log_mutex = new Mutex;
    // truncate file if too large
    struct stat s = { 0 };

    if (stat(path, &s) == 0 && s.st_size > get_log_max_size())
    {
        if (unlink(path) < 0)
        {
            auto msg = file_err(__func__, errno, "remove", path);
            throw runtime_error(msg);
        }
    }

    log_out = new ofstream(path, ios::app);

    if (log_out->fail())
    {
        auto msg = file_err(__func__, errno, "open", path);
        delete log_out;
        log_out = nullptr;

        throw runtime_error(msg);
    }
    else
    {
        write_prefix(*log_out, __FILE__, __LINE__);
        *log_out << "--- init ---" << std::endl;
    }
}

////////////////////////////////////////////////////////////////
// Expected to be called from main thread, when debugger instance
// is destructed.
void Log::close()
{
    if (log_out)
    {
        write_prefix(*log_out, __FILE__, __LINE__);
        *log_out << "--- close ---\n";
        log_out->flush();
    }

    delete log_out;
    log_out = nullptr;

    delete log_mutex;
    log_mutex = nullptr;
}

////////////////////////////////////////////////////////////////
ostream& Log::stream()
{
    if (!log_out)
    {
        throw logic_error("Log not initialized.");
    }
    return *log_out;
}

////////////////////////////////////////////////////////////////
Log::Level::Level(const char* path, size_t line, int n) : n_(n)
{
    if (log_mutex)
    {
        log_mutex->enter();
    }

    if (n_ < Log::verbosity())
    {
        if (log_prefix)
        {
            write_prefix( Log::stream(), basename(path), line );
            log_prefix = false;
        }
    }
}

////////////////////////////////////////////////////////////////
Log::Level::~Level()
{
    if (log_mutex)
    {
        log_mutex->leave();
    }
#if 0
    if (n_ < Log::verbosity())
    {
        assert(log_prefix);
    }
#endif
}

////////////////////////////////////////////////////////////////
Log::Level& Log::Level::operator<<(Log::Manipulator manipulator)
{
    if (n_ < Log::verbosity())
    {
        if (manipulator == static_cast<Manipulator>(endl))
        {
            Log::stream() << "\n";
            log_prefix = true;
        }
        else
        {
            manipulator(Log::stream());
        }
    }
    return *this;
}

////////////////////////////////////////////////////////////////
void Log::set_verbosity(int n)
{
    log_verbosity = n;
}

////////////////////////////////////////////////////////////////
int Log::verbosity()
{
    return log_verbosity;
}

