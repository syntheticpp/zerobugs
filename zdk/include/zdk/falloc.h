#ifndef FALLOC_H__90076409_62B2_4A33_8E6B_D94FACC9405D
#define FALLOC_H__90076409_62B2_4A33_8E6B_D94FACC9405D
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

#include "zdk/export.h"
#include <algorithm>
#include <cassert>
#ifdef DEBUG
 #include <iostream>
#endif
#include <boost/limits.hpp>

#if _MSC_VER && defined(max)
 #undef max // conflicts with std::numeric_limits<T>::max()
#endif

#ifndef __GNUC__
 #define __attribute__(a)
#endif

#if defined (__linux__)
 #define USE_FALLOC 1
#endif

/**
 * Default base allocator
 */
struct ZDK_LOCAL Malloc
{
    static void* alloc(size_t size)
    { return malloc(size); }

    static void dealoc(void* ptr)
    {
        if (ptr)
        {
            free(ptr);
        }
    }

    static void* alloc_aligned(size_t size, size_t alignment)
    {
        void* ptr = 0;
    #ifdef USE_FALLOC
        if (posix_memalign(&ptr, alignment, size) != 0)
        {
            assert(!ptr);
        }
    #endif
        return ptr;
    }

    static void dealoc_aligned(void* ptr) { dealoc(ptr); }

    static size_t max_size()
    {
        return std::numeric_limits<size_t>::max();
    }
};


#define DECREMENT_AVAIL(a,b)
#define INCREMENT_AVAIL(a,b)
#define CHECK_AVAIL(a,b,m) assert((a)==(m))

namespace Detail
{
    template<size_t S, typename T = size_t, typename U = size_t>
    class ZDK_LOCAL __attribute__((packed)) Block
    {
        T chunks_;
        U avail_; // used just as a block signature
        union
        {
            char   user_; // first byte of used memory
            Block* next_; // for free list of blocks
        };

    public:
    #ifdef USE_FALLOC
        enum { max_size = S - sizeof(T) - sizeof(U) };
    #else
        enum { max_size = 0 };
    #endif
        Block() : chunks_(0), avail_(max_size)
        { }

        inline size_t avail() const { return avail_; }

        inline size_t chunks() const { return chunks_; }

        /**
         * Allocate chunks of bytes out of this block
         */
        inline void* alloc(size_t bytes, char*& boundary)
        {
            void* result = boundary;
            DECREMENT_AVAIL(avail_, bytes);
            ++chunks_;
            boundary += bytes;
            return result;
        }

        /**
         * Dealocate bytes previously allocated from this block.
         * @return true if the block is entirely free (i.e. there
         * are no chunks in use)
         */
        inline bool dealoc(void* ptr, size_t bytes, char*& boundary)
        {
            assert(chunks_);
            CHECK_AVAIL(avail_, bytes, max_size);
            if (--chunks_ == 0)
            {
                return true;
            }
            if (ptr == boundary - bytes)
            {
                boundary = (char*)ptr;
                INCREMENT_AVAIL(avail_, bytes);
            }
            return false;
        }

        inline char* user_area() { return &user_; }

        inline Block* prepend_to(Block* head)
        {
            next_ = head;
            return this;
        }

        inline static Block* pop(Block*& head)
        {
            Block* tmp = head;
            if (head)
            {
                head = head->next_;
            }
            return tmp;
        }
    };
}


/**
 * Fast allocator. Favors use cases where there are a lot
 * of allocations and very few dealocations.
 * Larger blocks are allocated, from which smaller chunks
 * are given to the clients by the allocate() method.
 * Allocation / dealocation complexity is O(1)
 */
template<size_t S, typename B = Malloc>
class ZDK_LOCAL Falloc : public B
{
    // non-copyable, non-assignable
    Falloc(const Falloc&);
    Falloc& operator=(const Falloc&);

    typedef Detail::Block<S> block_type;

    size_t      usedBlks_;
    block_type* freeBlks_;
    block_type* current_;
    char*       free_;  // ptr to free mem in current blk
    bool        drop_;

    void reset_block() { current_ = 0, free_ = 0; }

    void dealoc_free_blocks()
    {
        while (block_type* tmp = block_type::pop(freeBlks_))
        {
            B::dealoc_aligned(tmp);
        }
    }

public:
    static size_t max_size() { return B::max_size(); }

    Falloc()
        : usedBlks_(0)
        , freeBlks_(0)
        , current_(0)
        , free_(0)
        , drop_(false)
    { }

    ~Falloc()
    {
        if (!drop_)
        {
            dealoc_free_blocks();
        }
    }

    void drop() { drop_ = true; }

    inline size_t free_bytes() const
    {
        return current_ ? S - (free_ - (char*)current_) : 0;
    }

    inline size_t used_bytes() const
    {
        return usedBlks_ * block_type::max_size - free_bytes();
    }

    /**
     * If there's enough memory in the current block, just
     * bump forward the boundary between used and free mem.
     * If not enough memory, request another block using the
     * base allocator.
     */
    void* allocate(size_t size)
    {
        assert(size);
        void* result = 0;
        if (size > block_type::max_size)
        {
            return B::alloc(size); // default allocation
        }
        else
        {
            // have room in current block?
            if (free_bytes() < size)
            {
                if ((current_ = block_type::pop(freeBlks_)) == 0)
                {
                    if (void* ptr = B::alloc_aligned(S, S))
                    {
                        current_ = new (ptr) block_type;
                    }
                    else
                    {
                        return 0;
                    }
                }
            /*#ifdef DEBUG
                else
                {
                    std::clog << "recycled: " << current_ << '\n';
                }
            #endif */
                assert(current_);
                CHECK_AVAIL(current_->avail(), 0, block_type::max_size);
                assert(current_->chunks() == 0);

                free_ = current_->user_area();
                ++usedBlks_;
            }
            result = current_->alloc(size, free_);

            assert(std::distance((char*)current_, free_) + free_bytes() == S);
        }
        return result;
    }

    inline static block_type* align(void* p)
    {
        size_t n = reinterpret_cast<size_t>(p);
        return reinterpret_cast<block_type*>(n & ~(S - 1));
    }

    /**
     * Find the block that owns the memory, and add the size
     * to the count of available bytes. Once the available
     * bytes amount to the max size of the block, drop the block.
     */
    void dealocate(void* p, size_t size)
    {
        if (drop_)
        {
            return;
        }
        if (!size || !p || (size > block_type::max_size))
        {
            B::dealoc(p);
        }
        else
        {
            assert(usedBlks_);
            block_type* block = align(p);

            if (block->dealoc(p, size, free_))
            {
                if (block == current_)
                {
                    reset_block();
                }
                freeBlks_ = block->prepend_to(freeBlks_);
                --usedBlks_;
            }
            assert(free_ >= (char*)current_);
        }
    }
};

#endif // FALLOC_H__90076409_62B2_4A33_8E6B_D94FACC9405D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
