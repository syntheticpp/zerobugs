#ifndef HOTKEY_H__D77F7887_1919_407F_91EA_C0F9C0538798
#define HOTKEY_H__D77F7887_1919_407F_91EA_C0F9C0538798
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include <memory>

namespace ui
{
    class HotKey
    {
    protected:
        HotKey() { }

        virtual int to_int() const = 0;

    public:
        operator int() const {
            return to_int();
        }

        static std::unique_ptr<HotKey> alt(char key);
        static std::unique_ptr<HotKey> ctrl(char key);
        static std::unique_ptr<HotKey> fn(int f);
    };
};


#endif // HOTKEY_H__D77F7887_1919_407F_91EA_C0F9C0538798

