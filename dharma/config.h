#ifndef CONFIG_H__02AB1CCF_D57B_4681_82F7_243AA75D393F
#define CONFIG_H__02AB1CCF_D57B_4681_82F7_243AA75D393F
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: config.h 714 2010-10-17 10:03:52Z root $
//
#include "zdk/config.h"
#if (__GNUC__ >= 3)
 #ifdef __INTEL_CXXLIB_ICC
  //
  // Intel compiler with Intel C++ libraries
  //
  #include <functional>
  #include <memory>

  namespace std
  {
    // Intel C++ library does not provide this SGI extension
    template<typename T>
    struct identity : public std::unary_function<T, T>
    {
        T operator()(T val) const { return val; }
    };

    template<typename T>
    struct identity<std::auto_ptr<T> >;
  }

 #else
 //
 // GCC, or Intel Compiler with GNU C++ library
 //
  #include <ext/functional>  // SGI extensions (select2nd, etc.)
  #if HAVE_UNORDERED_MAP
    #include <unordered_map>
    #include <unordered_set>
    #define hash_map unordered_map
    #define hash_set unordered_set
  #else
    #include <ext/hash_map>
    #include <ext/hash_set>
  #endif
  #define HAVE_HASH_MAP 1

  using namespace __gnu_cxx;
#endif

#endif // __GNUC__ >= 3

#include "zdk/platform.h"

#endif // CONFIG_H__02AB1CCF_D57B_4681_82F7_243AA75D393F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
