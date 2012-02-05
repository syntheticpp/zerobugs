#ifndef LOG_H__3922A7F3_578B_44B5_B11B_A94B7AD185AF
#define LOG_H__3922A7F3_578B_44B5_B11B_A94B7AD185AF
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/export.h"
#include "zdk/mutex.h"
#include <iosfwd>

#define dbgout(n)   Log::Level(__FILE__, __LINE__, (n))


namespace Log
{
    static const int ALWAYS = -1;

    // support std:: manipulators
    typedef std::ostream& (*Manipulator)(std::ostream&);


    // Initialize (open) logfile. Unlink if too large.
    void ZDK_EXPORT init(const char* path);
    void ZDK_EXPORT close();

    // Verbosity level
    void ZDK_EXPORT set_verbosity(int v);
    int ZDK_EXPORT  verbosity();

    std::ostream& ZDK_EXPORT stream();

    /**
     * Write to log if verbosity level (or higher) is set.
     */
    class ZDK_LOCAL Level
    {
    public:
        Level(
            const char* file,
            size_t      line,
            int         n);

        ~Level();

        Level& operator<<(const char* s);

        template<typename T> inline Level& operator<<(const T& arg)
        {
            if (n_ < Log::verbosity())
            {
                Log::stream() << arg;
            }
            return *this;
        }

        Level& operator<<(Manipulator manipulator);
        
        operator std::ostream& ();

    private:
        int n_; // log level 
    };

} // namespace

#endif // LOG_H__3922A7F3_578B_44B5_B11B_A94B7AD185AF

