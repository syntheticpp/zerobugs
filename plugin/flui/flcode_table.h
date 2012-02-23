#ifndef FLCODE_TABLE_H__D3145B5F_F5C3_4180_81B1_DBE0A33D9DF9
#define FLCODE_TABLE_H__D3145B5F_F5C3_4180_81B1_DBE0A33D9DF9
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/shared_string.h"
#include "listing.h"
#include <map>
#include <set>
#include <vector>
#include <unordered_map>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Table.H>


/**
 * Base class for custom widgets that display some sort of code listing
 * (can be source, assembly, or anything else the derived classes may want).
 */
class Fl_CodeTable : public Fl_Table
{
public:
    static const SharedStringPtr mark_arrow;
    static const SharedStringPtr mark_stop_enabled;
    static const SharedStringPtr mark_stop_disabled;

    /**
     * Associate a mark name with the pixmap data.
     */
    void set_mark_pixmap(
        SharedStringPtr     mark,
        const char* const*  pixmap );

    /**
     * Set or remove a pixmap marker at specified line.
     */
    void set_mark_at_line(
        int             line,
        SharedStringPtr mark,
        bool            setMark = true);

    /**
     * Remove all marks at all lines with given mark name
     */
    void remove_all_marks(SharedStringPtr mark);

protected:
    Fl_CodeTable(int x, int y, int w, int h, const char* label = nullptr);
    ~Fl_CodeTable();

    void draw_line_marks(int row, int x, int y);

    Fl_Color text_color() const { return FL_BLACK; }
    Fl_Color text_background() const { return FL_WHITE; }
    Fl_Color header_background() const { return FL_LIGHT2; }
    Fl_Color highlight_background() const { return FL_GRAY; }

    Fl_Color cell_background(int row, int column) const;

    Fl_Font font() const;
    int     font_size() const;
    
    /**
     * Highlight specfied line, may throw std::out_of_range.
     * Only one line can be highlighted at the time.
     */
    void highlight_line(ui::CodeListing*, int);

    int highlighted_line() const {
        return highlight_;
    }

private:
    int  highlight_; // index of current highlighted line

    // map line number to set of (optional) marks at that line
    std::unordered_map<int, std::set<SharedStringPtr> > marks_;

    // map mark name to pixmap data
    // @note: use a map to implicitly sort by name
    std::map<SharedStringPtr, std::shared_ptr<Fl_Pixmap> > pixmaps_;
};


/**
 * Custom widget that displays source code in a Table widget.
 */
class Fl_SourceTable : public Fl_CodeTable
{
public:
    Fl_SourceTable(int x, int y, int w, int h, const char* label = nullptr);

    void set_listing(ui::CodeListing* listing) {
        listing_ = listing;
    }

    void refresh(const RefPtr<Thread>&, const RefPtr<Symbol>&);

private:
    /**
     * implements custom drawing
     * @see http://seriss.com/people/erco/Fl_Table/documentation/Fl_Table.html#draw_cell
     */
    void draw_cell(TableContext, int row, int col, int x, int y, int w, int h);

    void draw_header(int col, int x, int y, int w, int h);

    virtual void resize(int x, int y, int w, int h);

    /**
     * @return the number of digits needed to print the highest line number
     */
    int  line_digits() const;

private:
    ui::CodeListing*    listing_;
    mutable int         digits_;
    size_t              maxTextLen_;
};


/**
 * Table widget that displays assembly code.
 */
class Fl_AsmTable : public Fl_CodeTable
{
public:
    Fl_AsmTable(int x, int y, int w, int h, const char* label = nullptr);

    void refresh(const RefPtr<Thread>&, const RefPtr<Symbol>&);

    void set_listing(ui::CodeListing* listing) {
        listing_ = listing;
    }

private:
    void draw_cell(TableContext, int row, int col, int x, int y, int w, int h);
    
    virtual void resize(int x, int y, int w, int h);

private:
    typedef std::vector<std::string> Columns;

    ui::CodeListing*    listing_;
    Columns             rowData_;
    int                 rowNum_;
    size_t              maxAsmLen_;
};

#endif // FLCODE_TABLE_H__D3145B5F_F5C3_4180_81B1_DBE0A33D9DF9

