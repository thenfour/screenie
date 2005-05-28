/*
  Last updated May 18, 2005 Carl Corcoran

  (c) 2004-2005 Carl Corcoran, carl@ript.net
  http://carl.ript.net/stringformat/
  http://carl.ript.net/wp
  http://mantis.winprog.org/
  http://svn.winprog.org/personal/carl/stringformat

  All software on this site is provided 'as-is', without any express or
  implied warranty, by its respective authors and owners. In no event will
  the authors be held liable for any damages arising from the use of this
  software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
  claim that you wrote the original software. If you use this software in
  a product, an acknowledgment in the product documentation would be
  appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not
  be misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.
*/

#pragma once

// options--------------------------------------------------------------------------
//#define CCSTR_OPTION_INLINE 0
//#define CCSTR_OPTION_WIN32 1
//#define CCSTR_OPTION_AUTOCAST 1


// internal options-----------------------------------------------------------------
#undef CCSTR_NOINLINE
#undef CCSTR_INLINE
#undef CCSTR_WIN32

// External options defaults
#ifndef CCSTR_OPTION_INLINE
# define CCSTR_OPTION_INLINE 1
//# pragma message("Setting default CCSTR_OPTION_INLINE = 1")
#endif

#ifndef CCSTR_OPTION_AUTOCAST
# define CCSTR_OPTION_AUTOCAST 0
//# pragma message("Setting default CCSTR_OPTION_AUTOCAST = 1")
#endif

#ifndef CCSTR_OPTION_WIN32
# ifdef _WIN32
#   define CCSTR_WIN32// include win32 types by default if possible
//#   pragma message("Setting default CCSTR_WIN32 = 1")
# endif
#else
# if CCSTR_OPTION_WIN32 == 1
#   define CCSTR_WIN32// explicit option
//#   pragma message("Setting CCSTR_WIN32 = 1")
# endif
#endif

// Set up inline option
#ifdef _MSC_VER
# if (CCSTR_OPTION_INLINE == 1)
#   define CCSTR_INLINE __declspec(noinline)
# else
#   define CCSTR_INLINE inline
# endif
#else
# define CCSTR_INLINE inline
#endif


