//
// utility.hpp - miscellaneous utility functions
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#ifndef SCREENIE_UTILITY_HPP
#define SCREENIE_UTILITY_HPP

// for std::vector
#include <vector>

// for tstd::tstring
#include "tstdlib/tstring.hpp"

bool MakeDestinationFilename(tstd::tstring& filename,
	const tstd::tstring& mimeType, const tstd::tstring& formatString);

tstd::tstring GetUniqueTemporaryFilename();
tstd::tstring GetWinInetErrorString();

tstd::tstring GetLastErrorString(DWORD lastError);

bool GetStringResource(HINSTANCE instance, UINT id, tstd::tstring& stringOut);

BOOL EnableChildWindows(HWND parent, BOOL enable);

bool GetSpecialFolderPath(tstd::tstring& path, int folder);
tstd::tstring GetUniqueFilename(tstd::tstring path);

tstd::tstring GetWindowString(HWND hwnd);
tstd::tstring GetComboSelectionString(HWND hWndCombo);

tstd::tstring FormatFilename(const SYSTEMTIME& systemTime, const tstd::tstring& inputFormat);

tstd::tstring tstring_tolower(const tstd::tstring& input);
tstd::tstring tstring_toupper(const tstd::tstring& input);

#endif