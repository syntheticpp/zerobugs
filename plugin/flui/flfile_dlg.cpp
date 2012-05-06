//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "flfile_dlg.h"
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_File_Input.H>
#include "dharma/canonical_path.h"


FlFileDialog::FlFileDialog(ui::Controller& controller)
    : FlDialog(controller, 0, 0, 500, 460, "Open File")
    , fileBrowser_(new Fl_File_Browser(20, 40, 460, 300))
    , fileInput_(new Fl_File_Input(20, 350, 460, 40))

{
    fileBrowser_->type(FL_HOLD_BROWSER);
    fileBrowser_->callback(browser_callback, this);

    // set_resizable(500, 400);

    add_ok_cancel([this] {
        //
        // TODO
        //
    });
}


void FlFileDialog::popup(
    const ui::State&    state,
    const char*         directory )
{
    fileBrowser_->load(directory);

    center();
    FlDialog::popup(state);
}


/* static */ void
FlFileDialog::browser_callback(
    Fl_Widget*  w,
    void*       data )
{
    auto browser = static_cast<Fl_File_Browser*>(w);
    int index = browser->value();
    const char* path = browser->text(index);

    static_cast<FlFileDialog*>(data)->on_browser_selection(path);
}


void FlFileDialog::on_browser_selection(const char* path)
{
    fileInput_->value(abspath(path));
}


