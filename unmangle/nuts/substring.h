#ifndef SUBSTRING_H__32184D6B_690E_48E7_82B0_4206F59E8EE5
#define SUBSTRING_H__32184D6B_690E_48E7_82B0_4206F59E8EE5
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

#include <algorithm>
#include <iosfwd>
#include <iterator>
#include <string>
#include "nuts/assert.h"

namespace nuts
{
    /**
     * Holds a pointer to another string object, the index
     * where the substring begins, and its length. For use with
     * containers, to minimize memory usage.
     * @note The essential assumption is that the "master" string
     * is in scope for the entire life-time of the substring.
     */
    template<
        typename S,
        typename CharT = typename S::value_type,
        typename CTraits = std::char_traits<CharT> >
    class substring
    {
    public:
        typedef typename S::const_iterator const_iterator;
        typedef size_t size_type;

        substring(const S& s, size_t pos, size_t len)
            : str_(&s), pos_(pos), len_(len)
        {
            NUTS_ASSERT(!len  || ((pos <= s.size()) && (pos + len <= s.size())));
        }
        substring() : str_(0), pos_(0), len_(0)
        { }
        const_iterator begin() const { return str_->begin() + pos_; }
        const_iterator end() const { return str_->begin() + pos_ + len_; }
        const CharT* data() const { return str_->data() + pos_; }
        const S* str() const { return str_; }
        size_type pos() const { return pos_; }
        size_type size() const { return len_; }
        bool empty() const { return (len_ == 0); }

        CharT operator[](size_t i) const
        {
            NUTS_ASSERT(i < len_);
            return (*str_)[pos_ + i];
        }
        template<typename U, typename C, typename T>
        bool less(const substring<U, C, T>& other) const
        {
            if ((str_ == other.str_)
                && (pos_ == other.pos_)
                && (len_ == other.len_))
            {
                return false;
            }
            return std::lexicographical_compare(
                this->begin(), this->end(),
                other.begin(), other.end());
        }
        void print(std::ostream& out) const
        {
            std::copy(this->begin(), this->end(),
                std::ostream_iterator<CharT>(out, ""));
        }
        template<typename U>
        bool equal(const substring<U, CharT, CTraits>& other) const
        {
            if (len_ != other.len_)
            {
                return false;
            }
            if ((str_ == other.str_) && (pos_ == other.pos_))
            {
                return true;
            }
            return CTraits::compare(data(), other.data(), len_) == 0;
        }
        void clear() { pos_ = len_ = 0; }
    private:
        const S* str_;
        size_t pos_, len_;
    };

    template<typename T>
    bool operator<(const substring<T>& lhs, const substring<T>& rhs)
    { return lhs.less(rhs); }

    template<typename T>
    bool operator==(const substring<T>& lhs, const substring<T>& rhs)
    {
        NUTS_ASSERT(lhs.equal(rhs) == rhs.equal(lhs));
        return lhs.equal(rhs);
     }

    template<typename T>
    bool operator!=(const substring<T>& lhs, const substring<T>& rhs)
    { return !(lhs == rhs); }

    template<typename T, typename CharT, typename Traits>
    std::ostream&
    operator<<(std::ostream& out, const substring<T, CharT, Traits>& ss)
    {
        ss.print(out);
        return out;
    }
}
#endif // SUBSTRING_H__32184D6B_690E_48E7_82B0_4206F59E8EE5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
