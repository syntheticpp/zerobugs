#ifndef BASIC_STRING_H__6E4205DA_542D_4149_9F1C_B27E7FA31192
#define BASIC_STRING_H__6E4205DA_542D_4149_9F1C_B27E7FA31192
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
#include <string>
#include "nuts/assert.h"
#include "nuts/memory.h"

namespace nuts
{
    template<
        typename CharT,
        typename T = std::char_traits<CharT>,
        typename A = default_alloc,
        size_t min_alloc_size = 256>
    class string
    {
        typedef T ctraits; // shorthand for char_traits

    public:
        typedef CharT value_type;
        typedef const CharT* const_iterator;
        typedef A alloc;

        static const size_t npos = (size_t)-1;

        string() : buf_(0), size_(0), capacity_(0) { }

        /**
         * @note copy ctor transfers ownership
         */
        string(string& s) : size_(s.size_), capacity_(s.capacity_)
        {
            buf_ = s.detach();
        }
        ~string() throw()
        {
            if (buf_)
            {
                NUTS_ASSERT(capacity_);
                A::dealocate(buf_ /*, capacity_ * sizeof(CharT) */);
        #if DEBUG
                buf_ = (CharT*)0xDeadCaca;
       #endif
            }
        }
        CharT operator[](size_t i) const
        {
            NUTS_ASSERT(i <= size_);
            return buf_[i];
        }
        const_iterator begin() const { return buf_; }

        const_iterator end() const { return buf_ + size_; }

        const CharT* data() const { return buf_; }

        const CharT* c_str() const { return buf_; }

        size_t size() const { return size_; }

        bool empty() const { return size_ == 0; }

        size_t capacity() const { return capacity_; }

        void clear() { size_ = 0; }

        string& swap(string& other) throw()
        {
            std::swap(buf_, other.buf_);
            std::swap(size_, other.size_);
            std::swap(capacity_, other.capacity_);
            return *this;
        }
        string& operator=(string& other)
        {
            string tmp(other);
            return this->swap(tmp);
        }
        void append(const CharT* chars, size_t size = npos)
        {
            if (chars)
            {
                if (size == npos)
                {
                    size = ctraits::length(chars);
                }
                if (size)
                {
                    CharT* oldptr = 0;
                    NUTS_ASSERT(capacity_ >= size_);
                    NUTS_ASSERT(size <= ctraits::length(chars));

                    if (capacity_ - size_ <= size)
                    {
                        oldptr = reallocate_to_fit_extra(size);
                    }
                    NUTS_ASSERT(capacity_ >= size_ + size + 1);

                    ctraits::copy(buf_ + size_, chars, size); // append
                    size_ += size;
                    buf_[size_] = CharT(); // null-end the string
                    A::dealocate(oldptr);
                }
            }
        }
        template<typename S>
        void append(const S& s)
        {
            append(s.data(), s.size());
        }
        void prepend(const CharT* chars, size_t size)
        {
            if (size)
            {
                NUTS_ASSERT(chars);
                CharT* oldPtr = 0;
                if (capacity_ - size_ <= size)
                {
                    oldPtr = reallocate_to_fit_extra(size, chars);
                }
                else
                {
                    NUTS_ASSERT(capacity_ >= size_ + size + 1);
                    ctraits::move(buf_ + size, buf_, size_);
                    ctraits::copy(buf_, chars, size);
                    size_ += size;
                    buf_[size_] = CharT();
                }
                A::dealocate(oldPtr);
            }
        }
        void rewind(size_t n)
        {
            NUTS_ASSERT(n <= size_);
            size_ -= n;
            buf_[size_] = CharT();
        }
        CharT* detach(bool trim = false)
        {
            CharT* buf = buf_;
            if (trim)
            {
                assert(capacity_ >= size_);
                buf = static_cast<CharT*>(A::reallocate(buf, size_ + 1));
            }
            buf_ = 0;
            size_ = capacity_ = 0;
            return buf;
        }
    private:
        CharT* allocate(size_t capacity)
        {
            const size_t nbytes = capacity * sizeof(CharT);
            if (nbytes < capacity) // wrapped around?
            {
                throw std::bad_alloc();
            }
            CharT* buf = static_cast<CharT*>(A::allocate(nbytes));
            NUTS_ASSERT(buf); // expect allocate to throw bad_alloc
            return buf;
        }
        CharT* clone(size_t capacity)
        {
            NUTS_ASSERT(capacity >= size_ + 1);
            CharT* buf = allocate(capacity);
            ctraits::copy(buf, buf_, size_);
            buf[size_] = CharT();
            return buf;
        }
        CharT*
        reallocate_to_fit_extra(size_t size, const CharT* prefix = 0)
        {
            NUTS_ASSERT(size);
            NUTS_ASSERT(capacity_ - size_ <= size);
         #if 1
            size_t cap = std::max((size_ + size + 1) << 1, min_alloc_size);
            if (cap <= capacity_) // capacity wrapped around?
            {
                cap = size_ + size + 1;
                if (cap <= capacity_)
                {
                    throw std::bad_alloc(); // exceeded size_t
                }
            }
         #else
            size_t cap = std::max((size_ + size + 1), min_alloc_size);
            NUTS_ASSERT(cap >= capacity_);
         #endif
            CharT* buf = 0;
            if (prefix)
            {
                buf = allocate(cap);
            #if DEBUG
                memset(buf, 'k', cap);
            #endif
                ctraits::copy(buf, prefix, size);
                ctraits::copy(buf + size, buf_, size_);
                size_ += size;
                buf[size_] = CharT();
            }
            else
            {
                buf = clone(cap);
            }
            std::swap(buf_, buf);
            // std::swap(capacity_, cap);
            capacity_ = cap;
            // A::dealocate(buf);
            return buf;
        }
    private:
        CharT* buf_;
        size_t size_;
        size_t capacity_;
    };

    template<typename CharT, typename T>
    string<CharT>& operator+=(string<CharT>& os, const T& arg)
    {
        return os.append(arg);
    }
#if (__GNUC__ >= 3)
    template<typename CharT>
    string<CharT>& operator+=(string<CharT>& os, const CharT* arg)
    {
        return os.append(arg);
    }
#endif
}
#endif // BASIC_STRING_H__6E4205DA_542D_4149_9F1C_B27E7FA31192
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
