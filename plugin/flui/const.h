#ifndef CONST_H__AE04D626_16B6_44BB_86B7_6C504890A378
#define CONST_H__AE04D626_16B6_44BB_86B7_6C504890A378
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
namespace ui
{
    struct Const
    {
        static const int asm_window_size = 256;

        static const int label_height       = 25;

        static const int menubar_height     = 25 + 30 /* toolbar height == 30  */;
        static const int statbar_height     = 25;
        static const int thread_regs_width  = 250;

        // default main window dimensions
        static const int default_window_width   = 1200;
        static const int default_window_height  = 800;
    };
}


#endif // CONST_H__AE04D626_16B6_44BB_86B7_6C504890A378

