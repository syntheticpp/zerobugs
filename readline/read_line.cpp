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
#include <assert.h>
#include <ctype.h>                  // for isprint()
#include <algorithm>
#include <functional>
#ifdef DEBUG
 #include <iostream>
#endif
#include "read_line.h"
#include "terminal.h"
#include "zdk/byteswap.h"
// #include "generic/auto_file.h"   // for logging
// todo: replace fprintf with boost::format

using namespace std;

// word separators in the edit buffer
const char kWordSeparators[] = " ";


void display_strings(const vector<string>& matches, FILE* file)
{
    assert(!matches.empty());

    // find the size of the widest string
    size_t cwidth = 0;

    vector<string>::const_iterator i(matches.begin());
    for (; i != matches.end(); ++i)
    {
        if (i->size() > cwidth)
        {
            cwidth = i->size();
        }
    }
    // put at least one space between columns;
    // a nice side-effect is that division by zero is also prevented
    ++cwidth;

    size_t columns = Term::screen_width(fileno(file)) / cwidth;
    if (columns == 0)
    {
        columns = 78; // default to something sane
    }

    for (size_t j(0); j != matches.size(); ++j)
    {
        if ((j % columns) == 0)
        {
            fprintf(file, "\n");
        }

        assert(matches[j].size() <= cwidth);

        const unsigned int fill = cwidth - matches[j].size();

        fprintf(file, "%s%*s", matches[j].c_str(), fill, " ");
        fflush(file);
    }
    fprintf(file, "\n");
    fflush(file);
}


/// Sets the new text on the current line erasing all previous content
/// on this line
static void reset_current_line(const string& newText, FILE* file)
{
    fprintf(file, "\r%s\033[K", newText.c_str());
    fflush(file);
}


ReadLine::ReadLine(int input, FILE* output)
    : input_(input)
    , fout_(output)
    , pos_(0)
    , autoCompleteChar_(Term::KEY_TAB)
    , autoCompleteFunc_(NULL)
{
}


ReadLine::~ReadLine()
{
}


void ReadLine::set_auto_complete_func(AutoCompleteFn autoComplete)
{
    autoCompleteFunc_ = autoComplete;
}


bool ReadLine::read(string& resultLine)
{
    Term::Attr ttyAttr(input_);

    // temporarily disable echoing and buffering
    termios tc = ttyAttr.get();
    tc.c_lflag &= ~(ECHO | ICANON);

    ttyAttr.set(tc);

    pos_ = 0;
    buffer_.clear();
    matches_.clear(); // for auto-complete

    int ch = 0;

    StringList::const_iterator histIter(history_.end());
    bool result(false);

    fprintf(fout_, "%s", prompt_.c_str());
    fflush(fout_);

    //auto_file log(fopen("log.txt", "w+"));

    bool keepLooping(true);
    do
    {
        ch = Term::readkey(input_);

        switch (ch)
        {
        case Term::KEY_ENTER:
            resultLine = buffer_;
            result = true;

            fprintf(fout_, "\n");
            keepLooping = false;
            break;

        case Term::KEY_UP:
            histIter = show_prev_history_entry(histIter);
            break;

        case Term::KEY_DOWN:
            histIter = show_next_history_entry(histIter);
            break;

        case Term::KEY_LEFT:
            move_cursor_left();
            break;

        case Term::KEY_RIGHT:
            move_cursor_right();
            break;

        case Term::KEY_DELETE:
            delete_current_char();
            break;

        case Term::KEY_BACKDEL:
        case Term::KEY_BACKSPACE:
            delete_prev_char();
            break;

        case Term::KEY_HOME:
        case Term::KEY_HOME1:
        case Term::KEY_HOME2:
            move_cursor_home();
            break;

        case Term::KEY_END:
        case Term::KEY_END1:
        case Term::KEY_END2:
            move_cursor_to_end();
            break;

        default:
            if (ch == autoCompleteChar_)
            {
                //fprintf(log.get(), "buffer=%s\n", buffer_.c_str());
                handle_auto_complete();
            }
            else
            {
                if (ch < 256 && isprint(ch))
                {
                    // printable char, insert at cursor position
                    insert_char(ch);
                }
            #if 0
                else
                {
                    clog << "code=" << hex << ch << dec << endl;
                }
            #endif

                histIter = history_.end();
            }
            break;
        }
    } while(keepLooping);

    fflush(fout_);
    return result;
}


void ReadLine::set_prompt(string const& newPrompt)
{
    prompt_ = newPrompt;
}



// History stuff
void ReadLine::add_history_entry(string const& entry)
{
    history_.push_back(entry);
}


void ReadLine::remove_history_entry(StringList::iterator i)
{
    assert(i != history_.end());
    history_.erase(i);
}



ReadLine::StringList const& ReadLine::get_history_entries() const
{
    return history_;
}



void ReadLine::move_cursor_left()
{
    if (pos_ > 0)
    {
        --pos_;

        const size_t w = Term::screen_width(fout_);

        if ((pos_ + 1 + prompt_.size()) % w)
        {
            Term::move_cursor_left(fout_);
        }
        else
        {
            Term::move_cursor_up(fout_);
            Term::move_cursor_right(fout_, w - 1);
        }
    }
}


void ReadLine::move_cursor_right()
{
    if (pos_ < buffer_.size())
    {
        ++pos_;

        const size_t w = Term::screen_width(fout_);

        if ((pos_ + prompt_.size()) % w)
        {
            Term::move_cursor_right(fout_);
        }
        else
        {
            Term::move_cursor_down(fout_);
            Term::move_cursor_left(fout_, w - 1);
        }
    }
}


void ReadLine::move_cursor_home()
{
    const size_t w = Term::screen_width(fout_);

    Term::move_cursor_up(fout_, pos_ / w);

    fprintf(fout_, "\r%s", prompt_.c_str());
    fflush(fout_);

    pos_ = 0;
}


void ReadLine::move_cursor_to_end()
{
    fprintf(fout_, "%s", buffer_.substr(pos_).c_str());
    fflush(fout_);

    pos_ = buffer_.size();
}


ReadLine::StringList::const_iterator
ReadLine::show_prev_history_entry(StringList::const_iterator it)
{
    if (it != history_.begin())
    {
        // Move up
        --it;
        buffer_ = *it;

        reset_current_line(prompt_ + buffer_, fout_);
        pos_ = buffer_.size();
    }
    return it;
}


ReadLine::StringList::const_iterator
ReadLine::show_next_history_entry(StringList::const_iterator it)
{
    if (it != history_.end() && it + 1 != history_.end())
    {
        // Move down
        ++it;
        buffer_ = *it;

        // Reset the content of the current line
        reset_current_line(prompt_ + buffer_, fout_);
        pos_ = buffer_.size();
    }
    return it;
}


void ReadLine::delete_current_char()
{
    // we should be within buffer boundaries
    if(pos_ < buffer_.size())
    {
        buffer_.erase(buffer_.begin() + pos_);
        matches_.clear();

        Term::save_cursor(fout_);
        fprintf(fout_, "%s", buffer_.substr(pos_).c_str());
        Term::clear_eol(fout_);

        const size_t w = Term::screen_width(fout_);
        // if line wrapped, clear the next line
        if (((buffer_.size() + prompt_.size()) % w) == 0)
        {
            // carriage return, then clear to end of line
            fprintf(fout_, "\n\033[K");
        }
        Term::restore_cursor(fout_);
    }
}


void ReadLine::delete_prev_char()
{
    if (pos_ > 0)
    {
        buffer_.erase(buffer_.begin() + --pos_);
        matches_.clear();

        const size_t w = Term::screen_width(fout_);
        if (pos_ && ((pos_ + 1 + prompt_.size()) % w) == 0)
        {
            Term::move_cursor_up(fout_);
            Term::move_cursor_right(fout_, w - 1);
            Term::save_cursor(fout_);
        }
        else
        {
            Term::move_cursor_left(fout_);
            Term::save_cursor(fout_);
        }
        fprintf(fout_, "%s", buffer_.substr(pos_).c_str());
        Term::clear_eol(fout_);

        // if line wrapped, clear the next line
        if (((buffer_.size() + prompt_.size()) % w) == 0)
        {
            // carriage return, then clear to end of line
            fprintf(fout_, "\n\033[K");
        }

        Term::restore_cursor(fout_);
    }
}


// Insert a character in the buffer at cursor position
void ReadLine::insert_char(int ch)
{
    if (pos_ < buffer_.size())
    {
        buffer_.insert(pos_, 1, ch);
        matches_.clear();

        Term::save_cursor(fout_);
        fprintf(fout_, "%s", buffer_.substr(pos_++).c_str());

        Term::restore_cursor(fout_);

        const size_t w = Term::screen_width(fout_);
        if ((pos_ + prompt_.size()) % w)
        {
            Term::move_cursor_right(fout_);
        }
        else
        {
            Term::move_cursor_down(fout_);
            Term::move_cursor_left(fout_, w - 1);
        }
    }
    else
    {
        buffer_ += ch;
        putc(ch, fout_);
        fflush(fout_);
        ++pos_;
    }
}


void ReadLine::handle_auto_complete()
{
    // Determine the beginning and end of the current word
    const size_t index = buffer_.find_last_of(kWordSeparators);
    size_t beg = 0;

    if (index != string::npos)
    {
        beg += index + 1;
    }

    // TODO: fix the end when the cursor is in the middle of the buffer
    size_t end(buffer_.size());
    string matchWord(buffer_, beg, end - beg);

    if (complete_word(matchWord, beg))
    {
        // We have a match
        buffer_.replace(beg, end - beg, matchWord);
        pos_ = buffer_.size();

        fprintf(fout_, "\r%s%s", prompt_.c_str(), buffer_.c_str());
        fflush(fout_);
    }
}


/**
 * Find the longest common string in sorted list
 */
static string longest_match(const vector<string>& matches)
{
    string s;

    if (!matches.empty())
    {
        for (size_t n = 0; s.empty(); ++n)
        {
            vector<string>::const_iterator i = matches.begin();
            for (char c = (*i++)[n]; i != matches.end(); ++i)
            {
                if ((*i)[n] != c)
                {
                    s = i->substr(0, n);
                    break;
                }
            }
        }
    }
    return s;
}


/**
 * Tries to find a match for the specified word
 * @return true if a match was found
 */
bool ReadLine::complete_word(string& word, size_t pos)
{
    using namespace ReadLineLookup;

    if (!autoCompleteFunc_)
    {
        return false;
    }
    if (matches_.empty())
    {
        // Call the generator to get matches
        autoCompleteFunc_(word.c_str(), buffer_, matches_);
    }
    // unique set
    StringSet tmp(matches_.begin(), matches_.end());
    ResultPair range = find_matches(tmp, word);

    matches_.assign(range.first, range.second);

    if (matches_.empty())
    {
        return false;
    }

    if (matches_.size() == 1)
    {
        word = matches_.front() /* + " " */;
        matches_.clear();
        return true;
    }
    else
    {
        // We have more than one match

        // Term::save_cursor(fout_);

        // todo: dynamically compute the limit based on the window size
        static const size_t kMaxDisplayEntries = 100;

        bool showEntries = true;

        if (matches_.size() > kMaxDisplayEntries)
        {
            // We have too many entries; ask if we should display all
            // todo: use boost::format
            fprintf(fout_, "\nThere are %lu possibilities. Display all? (y/n)",
                    static_cast<long unsigned>(matches_.size()));
            fflush(fout_);

            do
            {
                int key = Term::readkey(input_);

                if (key  == 'y' || key == 'Y')
                {
                    showEntries = true;
                    break;
                }
                else if (key == 'n' || key == 'N')
                {
                    showEntries = false;
                    break;
                }
            } while(true);
        }

        // Term::restore_cursor(fout_);

        if (showEntries)
        {
            display_strings(matches_, fout_);
        }
        if (!matches_.empty())
        {
            // find the longest common string
            // (matches are sorted)
            // vector<string>::reverse_iterator ri = matches_.rbegin();
            if (word.size() < matches_.front().size())
            {
                const size_t size = word.size();
                word = longest_match(matches_);

                buffer_.replace(pos, size, word);
                pos_ = buffer_.size();
            }
        }

        // Show the prompt again with the typed word
        fprintf(fout_, "\n%s%s", prompt_.c_str(), buffer_.c_str());
        fflush(fout_);
    }

    return false;
}



namespace ReadLineLookup
{
    /**
     * Predicate used by find_matches
     */
    struct tokens_match : public binary_function<string, string, bool>
    {
        bool operator()(string const& token,
            string const& entry) const
        {
            if (entry.size() < token.size()) return false;

            return (entry.compare(0, token.size(), token) == 0);
        }
    };


    ResultPair find_matches(StringSet const& set, string const& text)
    {
        ResultPair result(set.end(), set.end());

        // Search where the matches start
        result.first = set.lower_bound(text);
        if (result.first != set.end())
        {
            // See where the matching tokens end
            result.second = find_if(result.first, set.end(),
                not1(bind1st(tokens_match(), text)));
        }
        return result;
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
