#ifndef FSTREAM_H__A34E8A7A_C77E_4DF3_8228_3E3AC10A89F5
#define FSTREAM_H__A34E8A7A_C77E_4DF3_8228_3E3AC10A89F5

// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "cstream.h"


CLASS FileStream : public CStream
{
public:
    explicit FileStream(const char* fname, bool readOnly = false, unsigned = __WORDSIZE);

    virtual ~FileStream() throw();

    void truncate();

    size_t size() const;

    loff_t position() const;

    virtual size_t write_object(const char*, const Streamable*);

private:
    auto_fd fd_;
};

#endif // FSTREAM_H__A34E8A7A_C77E_4DF3_8228_3E3AC10A89F5
