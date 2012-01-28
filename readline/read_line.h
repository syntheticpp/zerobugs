#ifndef READ_LINE_H__648102C3_5100_414C_94D7_754D9904986D
#define READ_LINE_H__648102C3_5100_414C_94D7_754D9904986D
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

#include "zdk/config.h"
#include <stdio.h>
#ifdef HAVE_UNISTD_H
 #include <unistd.h>
#endif
#include <string>
#include <vector>
#include <set>
#include "zdk/export.h"


class ZDK_LOCAL ReadLine
{
public:
    /**
     * Construct a line reader tied to the specifed input and output
     * files. It would've been more elegant and C++ correct to specify
     * I/O streams here, but problem is that I have to do some ioctl()
     * calls, and the C++ standard does not support associating file
     * descriptors with streams; it is not that bad, though. File I/O
     * and std::cout seem to play together much better than mixing
     * C/C++ I/O with ncurses.
     */
    explicit ReadLine(int input = STDIN_FILENO, FILE* output = stdout);

    ~ReadLine();

    typedef std::vector<std::string> StringList;

    /**
     * AutoComplete generator function prototype
     * @param word[in] - the current word in the input buffer
     * @param line[in] - the line typed so far
     * @param matches[out] - the matches for the specified text
     * @result true if at least one match was found.
     * @note the auto-complete function may used the current
     * word and line as hints.
     */
    typedef void (*AutoCompleteFn)(const char* word,
                                   const std::string& line,
                                   StringList& matches);

    void set_auto_complete_char(int ch) { autoCompleteChar_ = ch; }

    void set_auto_complete_func(AutoCompleteFn autoComplete);

    /**
     * Reads a line from the keyboard
     * Stops when ENTER is pressed or interrupt is received
     * @param resultLine[out] - characters typed by te user
     * @result true if it succeeded to read a line
     */
    bool read(std::string& resultLine);

    void set_prompt(const std::string& newPrompt );

    const std::string& get_prompt() const { return prompt_; }


    void add_history_entry(const std::string& entry);

    void remove_history_entry(StringList::iterator);

    void clear_history() { history_.clear(); }

    const StringList& get_history_entries() const;

private:
    ReadLine(const ReadLine&);
    ReadLine& operator=(const ReadLine&);

    void move_cursor_left();

    void move_cursor_right();

    void move_cursor_home();

    void move_cursor_to_end();

    StringList::const_iterator show_prev_history_entry(StringList::const_iterator);

    StringList::const_iterator show_next_history_entry(StringList::const_iterator);

    void delete_current_char();

    void delete_prev_char();

    /**
     * Inserts a character in the buffer at cursor position
     */
    void insert_char(int);

    /**
     * Tries to find a match for the specified word
     * @result true if a match was found
     */
    bool complete_word(std::string& word, size_t pos);

    void handle_auto_complete();

    bool is_in_buffer(int pos) const;

    int input_;
    FILE* fout_;

    size_t pos_;    // current position in line
    int autoCompleteChar_;
    AutoCompleteFn autoCompleteFunc_;
    std::string prompt_;
    std::string buffer_;
    StringList history_;
    StringList matches_;
};


// Utility functions for matching keywords
namespace ReadLineLookup
{
    typedef std::set<std::string> StringSet;

    typedef std::pair<StringSet::const_iterator,
                      StringSet::const_iterator> ResultPair;

    /**
     * Searches for a partial match in the specified set
     * @param set[in] - the set of keywords where we should search
     * @param text[in] - the word we are trying to match against the set
     * @result pair of iterators that specifies the range in the set where
     * text matches the keywords
     */
    ResultPair ZDK_LOCAL find_matches(const StringSet& set, const std::string& text);
}


void display_strings(const std::vector<std::string>&, FILE*);

#endif // READ_LINE_H__648102C3_5100_414C_94D7_754D9904986D

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
