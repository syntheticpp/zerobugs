#ifndef FLFILE_DIALOG_H__C47A015E_A95D_47A4_874F_799450A47AFE
#define FLFILE_DIALOG_H__C47A015E_A95D_47A4_874F_799450A47AFE
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "fldialog.h"

class Fl_File_Browser;
class Fl_File_Input;


class FlFileDialog : public FlDialog
{
public:
    explicit FlFileDialog(ui::Controller& controller);

    void popup(
        const ui::State&    state,
        const char*         directory );

private:
    static void browser_callback(
        Fl_Widget*      w,
        void*           data );

    void on_browser_selection( const char* );
    void load(std::string&&);

private:
    Fl_File_Browser*    fileBrowser_;
    Fl_File_Input*      fileInput_;
    std::string         directory_;
};

#endif // FLFILE_DIALOG_H__C47A015E_A95D_47A4_874F_799450A47AFE

