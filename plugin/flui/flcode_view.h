#ifndef FLCODE_VIEW_H__8EF08CB5_1507_4F24_80E2_8FF2EA8DF721
#define FLCODE_VIEW_H__8EF08CB5_1507_4F24_80E2_8FF2EA8DF721
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "flcode_table.h"
#include "flview.h"
#include "code_view.h"

#include <FL/Fl_Tabs.H>


/**
 * Implements ui::SourceView by delegating to a Fl_SourceTable.
 */
class FlSourceView : public FlView<ui::SourceView, Fl_SourceTable>
{
public:
    FlSourceView(ui::Controller&, const char* filename);

protected:
    ~FlSourceView() throw();

    virtual void update(const ui::State&);
};


/**
 * Manages multiple code views inside a Fl_Tabs widget.
 */
class FlMultiCodeView : public FlView<ui::MultiCodeView, Fl_Tabs>
{
public:
    explicit FlMultiCodeView(ui::Controller&);

protected:
    ~FlMultiCodeView() throw();

    virtual void update(const ui::State&);

    virtual ui::Layout::CallbackPtr make_callback();

    virtual RefPtr<CodeView> make_view(const Symbol&);
};

#endif // FLCODE_VIEW_H__8EF08CB5_1507_4F24_80E2_8FF2EA8DF721

