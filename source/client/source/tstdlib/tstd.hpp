#ifndef _TSTD_HPP_
# define _TSTD_HPP_

// NOTE: In Visual Studio, if compiling with unicode, you must compile with the option that
// makes wchar_t a native type (the correct behaviour) as opposed to a typedef to short.
// On the command line, this setting is /Zc:wchar_t. In the IDE, this is under C/C++|Language
// as "Treat wchar_t as built-in type".

// namespace tstd is intended to be like std, except with extended definitions and utilities
// centered around use of std::* classes specialized on character types where
// the primary type is decided by a preprocessor macro (_UNICODE).
// Its behaviour is intended to be parallel to that of tchar.h, but it does not include
// tchar.h and does not rely on any of tchar.h's behaviour to work.
// Only the _UNICODE macro is checked, but for proper behaviour in win32, defining both
// _UNICODE and UNICODE may be necessary.

// if _NOWCHAR is defined, pretty much everything in here becomes a noop and uses only
// char, and never wchar_t, regardless of whether _UNICODE is defined.

// This header also defines the macro _ST, which is like the tchar.h _T and _TEXT. This is
// only necessary for portability to non-windows platforms, and perhaps later this header
// will attempt to emulate the tchar.h behaviour of _T, but a way will have to be devised
// to prevent it from conflicting (may not actually be difficult).

namespace tstd
{
#  if defined(_UNICODE) && !defined(_NOWCHAR)
	typedef wchar_t tchar_t;
#   define _TT(a) L##a
#  else
	typedef char   tchar_t;
#   define _TT(a) a
#  endif
}

#endif
