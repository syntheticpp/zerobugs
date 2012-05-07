//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "flfile_dlg.h"
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_File_Input.H>

#include "dharma/canonical_path.h"
#include "dharma/syscall_wrap.h"

#include <iostream>
using namespace std;


FlFileDialog::FlFileDialog(ui::Controller& controller)
    : FlDialog(controller, 0, 0, 500, 460, "Open File")
    , fileBrowser_(new Fl_File_Browser(20, 40, 460, 300))
    , fileInput_(new Fl_File_Input(20, 350, 460, 40))
    , isDoubleClick_(false)
{
    fileBrowser_->type(FL_HOLD_BROWSER);
    fileBrowser_->callback(browser_callback, this);
    fileInput_->callback(file_callback, this);
    fileInput_->down_box(FL_THIN_UP_BOX);

    //TODO: make sure it gets resized properly
    // set_resizable(500, 400);

    add_ok_cancel([] {
    });
}


void FlFileDialog::load(string&& dir)
{
    if (!dir.empty() && dir[dir.length()-1] != '/')
    {
        dir += '/';
    }

    directory_.swap(dir);
    fileBrowser_->load(directory_.c_str());
}


void FlFileDialog::popup(
    const ui::State&    state,
    const char*         directory )
{
    assert(directory);
    load(abspath(directory));

    center();
    FlDialog::popup(state);
}


/* static */ void
FlFileDialog::browser_callback(
    Fl_Widget*  w,
    void*       data )
{
    assert(w);
    assert(data);

    auto browser = static_cast<Fl_File_Browser*>(w);
    int index = browser->value();

    if (const char* path = browser->text(index))
    {
        reinterpret_cast<FlFileDialog*>(data)->on_browser_selection(path);
    }
}


/* static */ void
FlFileDialog::file_callback(
    Fl_Widget*  w,
    void*       data )
{
    assert(w);
    assert(data);

    auto input = static_cast<Fl_File_Input*>(w);
    if (const char* path = input->value())
    {
        struct stat stbuf = { 0 };
        if (stat(path, &stbuf) == 0)
        {
            reinterpret_cast<FlFileDialog*>(data)->set_browser_selection(path);
        }
    }
}


void FlFileDialog::on_browser_selection(const char* path)
{
    assert(path);

    string fullpath(CanonicalPath(directory_ + path));

    if (sys::is_dir(fullpath.c_str()))
    {
        enable_ok(false);

        if (isDoubleClick_)
        {
            fileInput_->value(fullpath.c_str(), fullpath.length());
            load(move(fullpath));
        }
    }
    else
    {
        selected_.swap(fullpath);
        fileInput_->value(selected_.c_str(), selected_.length());
        enable_ok(true);
    }
}


void FlFileDialog::set_browser_selection(const char* path)
{
    assert(path);

    if (sys::is_dir(path))
    {
        enable_ok(false);
        load(string(path));
    }
    else
    {
        selected_ = path;
        enable_ok(true);
    }
}


int FlFileDialog::handle(int eventType)
{
    isDoubleClick_ = false;

    if (eventType == FL_PUSH && Fl::event_inside(fileBrowser_))
    {
        if (Fl::event_clicks() == 1)
        {
            isDoubleClick_ = true;
        }
    }

    return 0;   // don't hide event from dialog window
}


void FlFileDialog::close_impl()
{
    selected_.clear();
}

