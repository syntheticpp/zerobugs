#ifndef FLCODE_TABLE_H__D3145B5F_F5C3_4180_81B1_DBE0A33D9DF9
#define FLCODE_TABLE_H__D3145B5F_F5C3_4180_81B1_DBE0A33D9DF9
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include <FL/Fl_Table.H>
#include <string>
#include <vector>

/**
 * Display source code in a Table widget.
 */
class FlCodeTable : public Fl_Table
{
public:
    FlCodeTable(int x, int y, int w, int h, const char* label = nullptr);

    void read_file(const char* filename);

protected:
    virtual void draw_cell(
        TableContext,
        int row,
        int col,
        int x, 
        int y, 
        int w, 
        int h);

private:
    // Store the file as a vector of lines of text.
    std::vector<std::string>    lines_;
    std::string                 filename_;
};

#endif // FLCODE_TABLE_H__D3145B5F_F5C3_4180_81B1_DBE0A33D9DF9
