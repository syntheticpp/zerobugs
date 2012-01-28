//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sstream>
#include "dharma/canonical_path.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "dharma/system_error.h"
#include "msg.h"
#include "remote_io.h"
#if DEBUG
 #include <iostream>
#endif

using namespace std;
using namespace RPC;


struct ZDK_LOCAL RemoteFile
{
    int     file_;  // descriptor, as returned by open()
    word_t  pos_;   // current position in file
    string  name_;
};

typedef ext::hash_map<int, RemoteFile> FileTable;
static FileTable fileTable;


static int rio_open(const vector<uint8_t>& data)
{
    const char* filename = reinterpret_cast<const char*>(&data[0]);
    int file = open(filename, O_RDONLY);
    if (file < 0)
    {
        throw SystemError(filename);
    }
    RemoteFile remoteFile = { file, 0, filename };
    fileTable[file] = remoteFile;
    return file;
}


static FileTable::iterator find(int file, const char* func)
{
    FileTable::iterator i = fileTable.find(file);
    if (i == fileTable.end())
    {
        throw SystemError(EBADF, func);
    }
    return i;
}


static word_t rio_read(int file, vector<uint8_t>& data)
{
    FileTable::iterator i = find(file, __func__);

    file = i->second.file_;

    const word_t bytesRead = read(file, &data[0], data.size());
    if (bytesRead < 0)
    {
        throw SystemError("read: " + i->second.name_);
    }
    data.resize(bytesRead);
    // update the current position in file
    if ((i->second.pos_ = lseek(file, 0, SEEK_CUR)) < 0)
    {
        throw SystemError("lseek: " + i->second.name_);
    }
    return bytesRead;
}


static void rio_close(int file)
{
    FileTable::iterator i = find(file, __func__);
    if (close(i->second.file_) < 0)
    {
        throw SystemError("close: " + i->second.name_);
    }
#if DEBUG
    clog << __func__ << ": " << file << "=" << i->second.name_ << " ok." << endl;
#endif
    fileTable.erase(i);
}


static void rio_read_link(vector<uint8_t>& data)
{
#if 1 // this may produce a relative path
    string path(reinterpret_cast<const char*>(&data[0]));
    data.resize(PATH_MAX + 1);
    ssize_t result = readlink(path.c_str(), (char*)&data[0], PATH_MAX);
    if (result < 0)
    {
        throw SystemError(__func__ + (": " + path));
    }
    data.resize(result + 1);
    data[result] = 0;
#else
    string path = canonical_path(reinterpret_cast<const char*>(&data[0]));
    data.assign(path.c_str(), path.c_str() + path.size() + 1);
#endif
}


static void rio_seek(vector<uint8_t>& data)
{
    if (data.size() != sizeof(word_t) * 3)
    {
        throw invalid_argument(__func__);
    }

    word_t* p = reinterpret_cast<word_t*>(&data[0]);
    word_t file = *p++;
    word_t offs = *p++;
    word_t whence = *p;

    FileTable::iterator i = find(file, __func__);

    file = i->second.file_;

    if (lseek(file, offs, whence) < 0)
    {
        throw SystemError(__func__);
    }
    // update current position
    i->second.pos_ = lseek(file, 0, SEEK_CUR);
}


bool RemoteIO::dispatch(InputStream&, OutputStream& out)
{
    const word_t operation = value<rio_op>(*this);
    switch (operation)
    {
    case RIO_OPEN:
        value<rio_file>(*this) = rio_open(value<rio_data>(*this));
        break;

    case RIO_READ:
        rio_read(value<rio_file>(*this), value<rio_data>(*this));
        break;

    case RIO_READ_LINK:
        rio_read_link(value<rio_data>(*this));
        break;

    case RIO_SEEK:
        rio_seek(value<rio_data>(*this));
        break;

    case RIO_CLOSE:
        rio_close(value<rio_file>(*this));
        break;

    default:
        {
            ostringstream err;
            err << "Invalid remote I/O operation: " << operation;
            throw runtime_error(err.str());
        }
        break;
    }
    out.write_object(name(), this); // write back response message
    return true;
}

