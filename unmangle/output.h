// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: output.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

/**
 * Wraps some extra logic around an output string, inserts
 * commas, double-colons and spaces where needed.
 */
struct Output
{
#if defined(USE_STD_CONTAINERS)
    typedef std::vector<size_t> pos_stack_type;
#else
    typedef nuts::fixed_stack<size_t, 256> pos_stack_type;
#endif
    static const size_t npos = string_type::npos;

    Output() : delim_(0), spaceOnce_(false), substIndex_(npos)
    {
    }
    template<typename U>
    void append(const U* s, size_t len = npos)
    {
        // todo: support appending non-CharT
    }
    void append(const CharT* s, size_t len = npos)
    {
        if (s)
        {
            size_t off = npos;
            // s points inside the output buffer?
            // prepare for possible buffer reallocation
            if (s >= buf_.data() && s < buf_.data() + buf_.size())
            {
                 off = s - buf_.data();
            }
            if (delim_)
            {
                update_pos();
                NUTS_ASSERT(delim_[0] && delim_[1] && !delim_[2]);
                buf_.append(delim_, 2);
                delim_ = 0;
            }
            else if (*s == '>')
            {
                if (size_t size = buf_.size())
                {
                    if (buf_.data()[--size] == '>')
                    {
                        buf_.append(" ", 1);
                    }
                }
            }
            else if (*s == '<' && spaceOnce_)
            {
                buf_.append(" ", 1);
            }
/* #ifdef CPLUS_DEMANGLE_LEGACY_COMPAT
            if ((*s == '*') || (*s == '&'))
            {
                if (size_t size = buf_.size())
                {
                    const CharT* buf = buf_.data();
                    --size;
                    if (buf[size] != '*' && buf[size] != ' ')
                    {
                        buf_.append(" ", 1);
                    }
                }
            }
#endif // CPLUS_DEMANGLE_LEGACY_COMPAT */
            spaceOnce_ = false;
            if (off != npos)
            {
                NUTS_ASSERT(off <= buf_.size());
                s = buf_.data() + off;
            }
            buf_.append(s, len);
            if (len) { substIndex_ = npos; }
        }
    }
    /// Append unsigned number in base ten
    void append(size_t n)
    {
    #ifdef CPLUS_DEMANGLE_COMPAT
        if (boolAlpha_)
        {
            if (n) append("true", 4); else append("false", 5);
            boolAlpha_ = false;
            return;
        }
    #endif // CPLUS_DEMANGLE_COMPAT
        size_t i = n, d = 1;
        while (i /= 10) { d *= 10; } // count digits
        do
        {
            if (d)
            {
                i = (n / d);
                n %= d;
            }
            else
            {
                i = n % 10;
            }
            CharT c = CharT('0') + i;
            buf_.append(&c, 1);
        } while (d /= 10);
        substIndex_ = npos;
    }
    template<typename T> void append(const T& val)
    {
        if (!val.empty())
        {
            append(val.data(), val.size());
        }
    }
    CharT* detach(size_t* len)
    {
        if (this->empty())
        {
            if (len) *len = 0;
            return 0;
        }
        if (len) *len = buf_.size();
        // detach and trim extra memory
        return buf_.detach(true);
    }

    const CharT* data() const { return buf_.data(); }

    size_t size() const { return buf_.size(); }

    bool empty() const { return buf_.empty(); }

    void rewind(size_t n) { buf_.rewind(n); }

    CharT operator[](size_t i) { return buf_[i]; }

    substring_type substr(size_t pos, size_t size = npos) const
    {
        if (size == npos)
        {
            size = (buf_.size() >= pos) ? buf_.size() - pos : 0;
        }
        return substring_type(buf_, pos, size);
    }
    const CharT* delim_;// 2-char separator, either "::" or ", "
    union
    {
        bool spaceOnce_;// for operator<, << followed by template args
        bool boolAlpha_;
    };
    size_t substIndex_; // index in dictionary of most
                        // recently used substitution
    /**
     * A pointer inside the output buffer that is automatically
     * updated to skip over comma and double-colon delimiters.
     */
    class Position
    {
    public:
        explicit Position(Output& out) : out_(out)
        { out.positions_.push_back(out.size()); }
        ~Position() { out_.positions_.pop_back(); }
         operator size_t() const { return out_.positions_.back(); }
    private:
        Position(const Position&);
        Position& operator=(const Position&);
        Output& out_;
    };
    friend class Position;
private:
    /// Called before a delimiter is copied to the output buffer,
    /// so that the pointers into the buffer are updated.
    void update_pos()
    {
        pos_stack_type::iterator i = positions_.begin();
        pos_stack_type::iterator end = positions_.end();
        for (; i != end; ++i)
        {
            if (*i == size()) (*i) += 2;
        }
    }
private:
    Output(const Output&);
    Output& operator=(const Output&);
    string_type buf_;   // output buffer
    pos_stack_type positions_;
};
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4

