#ifndef FRAME_REG_H__8541D364_65E1_427B_AC10_BE3FD9F103B0
#define FRAME_REG_H__8541D364_65E1_427B_AC10_BE3FD9F103B0
//
// $Id: frame_reg.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/thread_util.h"
#include "reg.h"

#define RNAME(x) #x
#define OFF(x) (size_t)&(((user_regs_struct*)0)->x)

#define REG32_(n,name) user_reg<int32_t>(thread, (name), OFF(n), r.n)

#define FREG32_(n,name,f) \
    user_reg<int32_t>(thread, (name), OFF(n), r.n, (&Frame::f))

#define REG32(n,name) regs.push_back(REG32_(n,name))
#define FRAME_REG32(n,name,f) regs.push_back(FREG32_(n,name,f))

#define REG_(n) user_reg<reg_t>(thread, RNAME(n), OFF(n), r.n)

#define FREG_(n,f) \
    user_reg<reg_t>(thread, RNAME(n), OFF(n), r.n, (&Frame::f))

#define REG(n) regs.push_back(REG_(n))
#define FRAME_REG(n,f) regs.push_back(FREG_(n,f))


template<typename T>
inline RefPtr<Register>
user_reg(const Thread& t, const char* name, off_t offs, reg_t val)
{
    return new Reg<T, REG_USER>(name, t, offs, val);
}

template<typename T>
inline RefPtr<Register>
user_reg(const Thread& t, const char* name, off_t offs, reg_t val, reg_t (Frame::*mfun)() const)
{
    if (Frame* f = thread_current_frame(&t))
    {
        return new Reg<T, REG_USER>(name, t, (size_t)-1, (f->*mfun)());
    }
    else
    {
        return user_reg<T>(t, name, offs, val);
    }
}
#endif // FRAME_REG_H__8541D364_65E1_427B_AC10_BE3FD9F103B0
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
