#ifndef LANG_H__CCB06792_F4A5_46BF_A4BC_DDE07E20EF45
#define LANG_H__CCB06792_F4A5_46BF_A4BC_DDE07E20EF45
//
// $Id: lang.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

enum Language
{
    LANG_C89                     = 0x0001,
    LANG_C                       = 0x0002,
    LANG_Ada83                   = 0x0003,
    LANG_C_plus_plus             = 0x0004,
    LANG_Cobol74                 = 0x0005,
    LANG_Cobol85                 = 0x0006,
    LANG_Fortran77               = 0x0007,
    LANG_Fortran90               = 0x0008,
    LANG_Pascal83                = 0x0009,
    LANG_Modula2                 = 0x000a,
    LANG_Java                    = 0x000b, /* DWARF3 */
    LANG_C99                     = 0x000c, /* DWARF3 */
    LANG_Ada95                   = 0x000d, /* DWARF3 */
    LANG_Fortran95               = 0x000e, /* DWARF3 */
    LANG_PLI                     = 0x000f, /* DWARF3 */
    LANG_ObjC                    = 0x0010, /* DWARF3f */
    LANG_ObjC_plus_plus          = 0x0011, /* DWARF3f */
    LANG_UPC                     = 0x0012, /* DWARF3f */
    LANG_D                       = 0x0013, /* DWARF3f */
    LANG_lo_user                 = 0x8000,
    LANG_Mips_Assembler          = 0x8001, /* MIPS   */
    LANG_Upc                     = 0x8765, /* UPC, use
                                        LANG_UPC instead. */
/* ALTIUM extension */
    LANG_ALTIUM_Assembler        = 0x9101,  /* ALTIUM */
};
#endif // LANG_H__CCB06792_F4A5_46BF_A4BC_DDE07E20EF45
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
