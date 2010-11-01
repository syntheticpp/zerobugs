#pragma once
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

class Mutex
{
public:
	void enter();
    void leave() throw();
    bool leave(std::nothrow_t);

    bool trylock();

    void assert_locked() volatile;
};

