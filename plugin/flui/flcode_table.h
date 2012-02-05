#ifndef FLCODE_TABLE_H__D3145B5F_F5C3_4180_81B1_DBE0A33D9DF9
#define FLCODE_TABLE_H__D3145B5F_F5C3_4180_81B1_DBE0A33D9DF9
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include <FL/Fl_Table.H>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include "dharma/sarray.h"


/**
 * Custom widget that displays source code in a Table widget.
 * TODO: is FlTextTable a better name?
 */
class FlCodeTable : public Fl_Table
{
public:
    FlCodeTable(int x, int y, int w, int h, const char* label = nullptr);

    void read_file(const char* filename);

    /**
     * Highlight specfied line, may throw std::out_of_range.
     * Only one line can be highlighted at the time.
     */
    void highlight_line(int);

    void set_mark_pixmap(
        const std::string&  mark,
        const char* const*  pixmapData );
    
    void set_mark_at_line(int, const std::string& mark, bool = true);

    int highlighted_line() const { return highlight_; }

private:
    /**
     * implements custom drawing
     * @see http://seriss.com/people/erco/Fl_Table/documentation/Fl_Table.html#draw_cell
     */
    virtual void draw_cell(
        TableContext,
        int row,
        int col,
        int x, 
        int y, 
        int w, 
        int h);

    void draw_header(int col, int x, int y, int w, int h);
    void draw_line_marks(int row, int x, int y);

    /**
     * @return the number of digits needed to print the highest line number
     */
    int  line_digits() const;

    Fl_Color text_color() const { return FL_BLACK; }
    Fl_Color text_background() const { return FL_WHITE; }
    Fl_Color header_background() const { return FL_LIGHT2; }
    Fl_Color highlight_background() const { return FL_GRAY; }

private:
    // Store the file as a vector of lines of text.
    std::vector<std::string>    lines_;
    std::string                 filename_;
    Fl_Font                     font_;
    int                         fontSize_;
    int                         highlight_;
    mutable int                 digits_;

    std::unordered_map<int, std::set<std::string> > marks_;
    std::unordered_map<std::string, SArray> pixmaps_;
};

#endif // FLCODE_TABLE_H__D3145B5F_F5C3_4180_81B1_DBE0A33D9DF9

