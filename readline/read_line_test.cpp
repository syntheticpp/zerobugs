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
#include <algorithm>
#include "read_line.h"
#include "terminal.h"

using namespace std;

namespace
{
    bool InitCommandSet( ReadLineLookup::StringSet& commands )
    {
        commands.insert("bibi");
        commands.insert("Bubu");
        commands.insert("biba");
        commands.insert("bibanul");
        commands.insert("baba");
        commands.insert("barza");
        return true;
    }

} // End anonymous namespace


void MatchGenerator(const char* text, const string&, vector<string>& matches)
{
/*
    using namespace ReadLineLookup;

    static StringSet commands;

    InitCommandSet( commands );

    // Lookup the text in the commands
    ResultPair range = find_matches(commands, text);

    // Copy the results to the destination
    matches.clear();
    copy( range.first, range.second, back_inserter(matches) );
 */
        matches.push_back("bibi");
        matches.push_back("Bubu");
        matches.push_back("biba");
        matches.push_back("bibanul");
        matches.push_back("baba");
        matches.push_back("barza");
        matches.push_back("barba");
        matches.push_back("barca");
        matches.push_back("quit");
}

int main()
{
    ReadLine reader;

    reader.set_auto_complete_func( MatchGenerator );
    reader.set_prompt( "prompt >" );
    //reader.set_prompt( ">" );

    bool loop( false );
    string buffer;

    do
    {
        loop = reader.read( buffer );

        while (!buffer.empty())
        {
            size_t n = buffer.size() - 1;
            if (buffer[n] == ' ')
            {
                buffer.erase(n);
            }
            else break;
        }

        if ( !buffer.empty() )
        {
            reader.add_history_entry( buffer );
        }

        if (buffer == "quit") break;

        if (buffer == "list")
        {
            system("ls -l");
        }

        unsigned int x = 0, y = 0;
        Term::get_cursor_pos(stdout, x, y);

        fprintf(stderr, "[%d,%d]\n", x, y);
    }while( loop );

    return 0;
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
