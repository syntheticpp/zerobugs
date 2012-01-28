#ifndef LOCATION_H__1891B6AA_E341_450D_98B9_EDB37ED73A33
#define LOCATION_H__1891B6AA_E341_450D_98B9_EDB37ED73A33
//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <dwarf.h>
#include <libdwarf.h>
#include <boost/utility.hpp>
#include "interface.h"

namespace Dwarf
{
	template<Dwarf_Half> class LocationAttrT;

	/**
	 * Evaluates addressing operations.
	 */
	CLASS Location : boost::noncopyable
	{
		friend class LocationAttr;

	public:
		static Dwarf_Addr eval(Dwarf_Debug handle,
                               Dwarf_Addr pc, // program count
							   Dwarf_Addr frameBase,
							   Dwarf_Addr moduleBase,
							   const Dwarf_Locdesc*,
							   bool& isValue);
		virtual ~Location();

		/**
		 * Select and evaluate one location expression.
		 * @return the result of the evaluation.
		 * @param frameBase the base address of the stack frame
		 * @param moduleBase the address where the program or
		 * shared object containing the expression is loaded.
		 * The moduleBase address is needed to adjust the operands.
		 * of DW_OP_addr
		 * @param pc program counter; when several expressions
		 * are present, the one to be evaluated is the one that
		 * has the range [low_pc, high_pc) that contains PC.
		 */
		virtual Dwarf_Addr eval(
			Dwarf_Addr frameBase,
			Dwarf_Addr moduleBase,
			Dwarf_Addr unitBase,
			Dwarf_Addr pc) const;

		bool is_register(Dwarf_Addr pc, Dwarf_Addr unitBase) const;

		/**
		 * @return true if the DWARF expression represents the
		 * actual value of the object, rather than its location.
		 */
		bool is_value() const { return isValue_; }

	protected:
		Location(Dwarf_Debug dbg, Dwarf_Attribute);

	private:
		Dwarf_Debug dbg_;
		Dwarf_Locdesc** list_;
		Dwarf_Signed size_;

		// DWARF-4 DW_OP_stack_value
		// the DWARF expression represents the actual value of
		// the object, rather than its location
		mutable bool isValue_;
	};


	CLASS VTableElemLocation : public Location
	{
	public:
		VTableElemLocation(Dwarf_Debug dbg, Dwarf_Attribute);

	protected:
		virtual Dwarf_Addr eval(
			Dwarf_Addr frameBase,
			Dwarf_Addr moduleBase,
			Dwarf_Addr unitBase,
			Dwarf_Addr pc) const;
	};
} // namespace Dwarf

#endif // LOCATION_H__1891B6AA_E341_450D_98B9_EDB37ED73A33
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
