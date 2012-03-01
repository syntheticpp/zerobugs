#ifndef FLVIEW_H__2BBFE1FF_5D50_475B_87DD_3A119541D3BE
#define FLVIEW_H__2BBFE1FF_5D50_475B_87DD_3A119541D3BE
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "view.h"

class Fl_Group;
class Fl_Widget;

void insert(Fl_Widget&, ui::LayoutCallback&);


template<typename W>
class FlViewImpl : public W
{
public:
    typedef W widget_type;

    template<typename... Args>
    FlViewImpl(Args... args) : W(args...)
    { }
};


struct FlViewBase
{
    virtual Fl_Widget* base_widget() = 0;
};


/**
 * Implements interface T in terms of a widget W.
 */
template<typename T, typename W>
class FlView : public T, public FlViewBase
{
public:
    template<typename... Args>
    FlView(ui::Controller& controller, Args... args)
        : T(controller)
        , impl_(new FlViewImpl<W>(args...))
    { }

    void resize(int x, int y, int w, int h) {
        base_widget()->resize(x, y, w, h);
    }

    W* widget() {
        assert(impl_);
        return impl_;
    }

    const W* widget() const {
        assert(impl_);
        return impl_;
    }

    Fl_Widget* base_widget() {
        return widget();
    }

protected:
    typedef FlView<T, W> base_type;

    virtual void insert_self(ui::LayoutCallback& cb) {
        insert(*impl_, cb);
    }
private:
    FlViewImpl<W>* impl_;
};

#endif // FLVIEW_H__2BBFE1FF_5D50_475B_87DD_3A119541D3BE

