// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
// ZDK headers
#include "zdk/argv_util.h"
#include "zdk/check_ptr.h"
#include "flcode_view.h"
#include "fledit_breakpoint_dlg.h"
#include "fleval_dlg.h"
#include "flbreakpoint_view.h"
#include "flvar_view.h"
#include "flmain_window.h"
#include "flmenu.h"
#include "flpack_layout.h"
#include "flstack_view.h"
#include "fltoolbar.h"
#include "flui.h"

// C++ headers
#include <cassert>
#include <thread>

// fltk headers
#include <FL/Fl.H>

// properties
#define WINDOW_TITLE    "ZeroBUGS"

using namespace std;


/**
 * Advertise the interfaces supported by this plugin
 */
void query_plugin(InterfaceRegistry* registry)
{
    registry->update(DebuggerPlugin::_uuid());
}

////////////////////////////////////////////////////////////////
int32_t query_version(int32_t* minor)
{
    if (minor) *minor = ZDK_MINOR;
    return ZDK_MAJOR;
}


////////////////////////////////////////////////////////////////
Plugin* create_plugin(uuidref_t iid)
{
    Plugin* plugin = 0;

    if (uuid_equal(DebuggerPlugin::_uuid(), iid))
    {
        plugin = new Flui();
    }
    return plugin;
}


////////////////////////////////////////////////////////////////
Flui::Flui()
    : ui::Controller()
    , window_(nullptr)
{
}


////////////////////////////////////////////////////////////////
Flui::~Flui()
{
    delete window_;
}


////////////////////////////////////////////////////////////////
void Flui::run()
{
    ui::Controller::run();
}


////////////////////////////////////////////////////////////////
void Flui::release()
{
    delete this;
}


////////////////////////////////////////////////////////////////
/**
 * Parse command line and other initializing stuff
 */
bool Flui::initialize(

    Debugger*   debugger,
    int*        argc,
    char***     argv )
{
    if (!ui::Controller::initialize( debugger, argc, argv ))
    {
        return false;
    }

    if (argc)
    {
        args_.push_back( (*argv)[0] );
    }

    // can worry about cmd line params later...

    /*BEGIN_ARG_PARSE(argc, argv)
    ON_ARG("--foo")
        {
            cout << __func__ << ": foon";
        }
    ON_ARG("--bar")
        {
            cout << __func__ << ": barn";
        }
    END_ARG_PARSE */
    return true;
}


#define WINDOW_COORD(wnd,f) (wnd ? wnd->parent()->f() : 0)

////////////////////////////////////////////////////////////////
int Flui::x() const
{
    return WINDOW_COORD(window_, x);
}

////////////////////////////////////////////////////////////////
int Flui::y() const
{
    return WINDOW_COORD(window_, y);
}

////////////////////////////////////////////////////////////////
int Flui::w() const
{
    return WINDOW_COORD(window_, w);
}

////////////////////////////////////////////////////////////////
int Flui::h() const
{
    return WINDOW_COORD(window_, h);
}


////////////////////////////////////////////////////////////////
RefPtr<ui::BreakPointView> Flui::init_breakpoint_view()
{
    return new FlBreakPointView(*this);
}


////////////////////////////////////////////////////////////////
RefPtr<ui::CodeView> Flui::init_code_view()
{
    return new FlMultiCodeView(*this);
}


////////////////////////////////////////////////////////////////
RefPtr<ui::Layout> Flui::init_layout()
{
    assert(window_);

    auto layout = new FlPackLayout(*this, 0, 0, w(), h());

    window_->end();
    window_->show();

    return layout;
}


////////////////////////////////////////////////////////////////
RefPtr<ui::VarView> Flui::init_locals_view()
{
    return new FlLocalsView(*this);
}


////////////////////////////////////////////////////////////////
RefPtr<ui::StackView> Flui::init_stack_view()
{
    return new FlStackView(*this);
}


static void (*default_callback)(Fl_Widget*, void*);

////////////////////////////////////////////////////////////////
static void close_callback(Fl_Widget* w, void* data)
{
    if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape)
    {
        return;
    }
    default_callback(w, data);
}


////////////////////////////////////////////////////////////////
void Flui::init_main_window(int x, int y, int w, int h)
{
    assert(window_ == nullptr);

    // Kludge picked up from
    // http://comments.gmane.org/gmane.comp.lib.fltk.devel/571

    // "The X double buffer has a nasty bug that results in large black flickering
    // when resizing the window. This is due to the window manager resizing the
    // window before the app can get around to drawing the off-screen buffer. One
    // fix is to nest two windows and only double-buffer the inner one [...]"

    auto appWnd = new Fl_Window(x, y, w, h, WINDOW_TITLE);
    appWnd->resizable(appWnd);
    appWnd->size_range(500, 500);

    // install custom callback to prevent Escape from closing
    // the main window (http://www.fltk.org/doc-1.1/Fl_Widget.html#Fl_Widget.callback)
    default_callback = appWnd->callback();
    appWnd->callback(close_callback);

    window_ = new FlMainWindow(*this, 0, 0, w, h);
    window_->resizable(window_);
    window_->end();
    appWnd->end();
    appWnd->show();
    window_->begin();
}


////////////////////////////////////////////////////////////////
RefPtr<ui::PopupMenu> Flui::init_contextual_menu()
{
    return new FlPopupMenu(*this);
}


////////////////////////////////////////////////////////////////
RefPtr<ui::CompositeMenu> Flui::init_menu()
{
    assert(window_);
    return new FlMenuBar(*this, window_->w(), 25);
}


////////////////////////////////////////////////////////////////
RefPtr<ui::Toolbar> Flui::init_toolbar()
{
    assert(window_);
    return new FlToolbar(*this, window_->w(), 30);
}


////////////////////////////////////////////////////////////////
void Flui::lock()
{
    Fl::lock();
}


////////////////////////////////////////////////////////////////
void Flui::unlock()
{
    Fl::unlock();
    Fl::awake();
}


////////////////////////////////////////////////////////////////
void Flui::notify_ui_thread()
{
    Fl::awake();
}


////////////////////////////////////////////////////////////////
int Flui::wait_for_event()
{
    return Fl::wait();
}


////////////////////////////////////////////////////////////////
const char* Flui::description() const
{
    return "Fltk-based user interface";
}


////////////////////////////////////////////////////////////////
const char* Flui::copyright() const
{
    return "";
}


////////////////////////////////////////////////////////////////
//
// Utility for constructing and popping up singleton dialogs
//
template<typename T, typename... Args>
static void popup(
    ui::Controller& controller,
    once_flag&      f,
    Args...         args)
{
    static T* dialog = nullptr;
    //
    // initialize dialog lazily
    //
    call_once(f, [&dialog, &controller] {
        Fl_Group::current(nullptr);
        dialog = new T(controller);
    });

    controller.set_current_dialog(dialog);
    dialog->popup(controller.state(), args...);
}


////////////////////////////////////////////////////////////////
void Flui::show_edit_breakpoint_dialog(addr_t addr)
{
    static once_flag once;
    popup<FlEditBreakPointDlg>(*this, once, addr);
}


////////////////////////////////////////////////////////////////
void Flui::show_eval_dialog()
{
    static once_flag once;
    popup<FlEvalDialog>(*this, once);
}


////////////////////////////////////////////////////////////////
void Flui::error_message(const std::string&)
{
    // TODO
}

