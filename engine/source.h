#ifndef SOURCE_H__6D1ABB97_A1DC_45EA_8FFE_D64876D7DC36
#define SOURCE_H__6D1ABB97_A1DC_45EA_8FFE_D64876D7DC36
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

#include <iosfwd>
#include <string>
#include <vector>

/**
 * A class to show source file listing when in text mode
 */
class SourceListing
{
public:
    explicit SourceListing(const std::string&);

    ~SourceListing();

    const char* name() const { return name_.c_str(); }

    /**
     * @return the last visible line in the onscreen listing
     */
    size_t current_line() const { return currentLine_; }

    void set_current_line(size_t);

    /**
     * @return the line in the source that corresponds
     * to the current value of the program counter
     */
    size_t symbol_line() const { return symbolLine_; }

    void set_symbol_line(size_t line) { symbolLine_ = line; }

    size_t list(std::ostream&, size_t howMany = 5) const;

    bool empty() const { return lines_.empty(); }

private:
    std::string name_;
    size_t currentLine_;
    size_t symbolLine_;

    std::vector<std::string> lines_;
};

#endif // SOURCE_H__6D1ABB97_A1DC_45EA_8FFE_D64876D7DC36
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
