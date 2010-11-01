#ifndef TYPE_SELECT_H__1039340132
#define TYPE_SELECT_H__1039340132
//
// $Id: type_select.h 714 2010-10-17 10:03:52Z root $
//
// From Andrei Alexandrescu's Loki
//
template<bool, typename T, typename U>
struct TypeSelect
{
    typedef T type;
};

template<typename T, typename U>
struct TypeSelect<false, T, U>
{
    typedef U type;
};

#endif // TYPE_SELECT_H__1039340132
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
