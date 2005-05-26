//
// utility.cpp - miscellaneous utilities that don't belong elsewhere
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#include "stdafx.hpp"

#include <iomanip>
#include <wininet.h>
#include <algorithm>

#include "codec.hpp"
#include "utility.hpp"

bool MakeDestinationFilename(tstd::tstring& filename, const tstd::tstring& mimeType, const tstd::tstring& formatString)
{
	ImageCodecsEnum imageCodecs;
	Gdiplus::ImageCodecInfo* codecInfo = imageCodecs.GetCodecByMimeType(mimeType.c_str());

	if (codecInfo != NULL)
	{
		// get the system time for processing the filename
		SYSTEMTIME systemTime = { 0 };
		::GetSystemTime(&systemTime);

		tstd::tstring filenameRoot = FormatFilename(systemTime, formatString);
		tstd::tstring filenameExtension = "JPG";

		filename = LibCC::Format(TEXT("%.%")).s(filenameRoot).s(filenameExtension).Str();

		return true;
	}

	return false;
}

tstd::tstring GetUniqueTemporaryFilename()
{
	TCHAR pathBuffer[MAX_PATH] = { 0 };
	::GetTempPath(MAX_PATH, pathBuffer);

	TCHAR filenameBuffer[MAX_PATH] = { 0 };
	::GetTempFileName(pathBuffer, TEXT("SCR"), 0, filenameBuffer);

	return tstd::tstring(filenameBuffer);
}

tstd::tstring GetLastErrorString(DWORD lastError)
{
	tstd::tstring message;
	TCHAR* buffer = 0;

	::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		lastError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(PTSTR)&buffer,
		0,
		NULL 
		);

	if (buffer)
	{
		message = buffer;
		::LocalFree(buffer);
	}
	else
	{
		message = LibCC::Format(TEXT("Unknown error 0x%")).ul<16, 8, TEXT('0')>(lastError).Str();
	}

	return message;
}

tstd::tstring GetWinInetErrorString()
{
	DWORD lastError;

	DWORD bufferLength = 512;
	std::vector<tstd::tchar_t> buffer(bufferLength);
	std::fill(buffer.begin(), buffer.end(), 0);

	if (::InternetGetLastResponseInfo(&lastError, &buffer[0], &bufferLength) != TRUE)
	{
		// we only want reallocate the buffer if the call failed because
		// there wasn't enough buffer space. if something else went wrong,
		// it's not our problem.

		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			buffer.reserve(bufferLength);
			std::fill(buffer.begin(), buffer.end(), 0);

			::InternetGetLastResponseInfo(&lastError, &buffer[0], &bufferLength);
		}
	}

	return tstd::tstring(&buffer[0]);
}

bool GetStringResource(HINSTANCE instance, UINT id, tstd::tstring& stringOut)
{
	for (int bufferLength = 256; ; bufferLength *= 2)
	{
		std::vector<tstd::tchar_t> buffer(bufferLength);
		std::fill(buffer.begin(), buffer.end(), 0);

		int lengthRead = ::LoadString(instance, id, &buffer[0], bufferLength);

		if (lengthRead == 0)
			break;

		if (lengthRead < (bufferLength - 1))
		{
			stringOut = &buffer[0];
			return true;
		}
	}

	return false;
}

BOOL CALLBACK EnableChildWindowsEnumProc(HWND hwnd, LPARAM lParam)
{
	::EnableWindow(hwnd, static_cast<BOOL>(lParam));

	return TRUE;
};

BOOL EnableChildWindows(HWND parent, BOOL enable)
{
	// it's very likely that the caller did NOT want to disable every window
	// on the system... which would happen if 'parent' were NULL. (it's not fun.)
	ATLASSERT(::IsWindow(parent));

	return ::EnumChildWindows(parent, EnableChildWindowsEnumProc, static_cast<LPARAM>(enable));
}

bool GetSpecialFolderPath(tstd::tstring& path, int folder)
{
	TCHAR pathBuffer[MAX_PATH] = { 0 };

	if (SHGetSpecialFolderPath(0, pathBuffer, folder, FALSE))
	{
		path = pathBuffer;

		return true;
	}

	return false;
}

tstd::tstring GetWindowString(HWND hWnd)
{
	int length = ::GetWindowTextLength(hWnd) + 1;

	std::vector<TCHAR> buffer(length);
	::GetWindowText(hWnd, &buffer[0], length);

	return tstd::tstring(&buffer[0]);
}

tstd::tstring GetComboSelectionString(HWND hWndCombo)
{
	int curSel = ::SendMessage(hWndCombo, CB_GETCURSEL, 0, 0);

	if (curSel != CB_ERR)
	{
		// get the length of the listbox item, and add space for the null terminator
		int length = ::SendMessage(hWndCombo,
			CB_GETLBTEXTLEN, static_cast<WPARAM>(curSel), 0) + 1;

		std::vector<TCHAR> buffer(length);

		if (::SendMessage(hWndCombo, CB_GETLBTEXT, curSel,
			reinterpret_cast<LPARAM>(&buffer[0])) != CB_ERR)
		{
			return tstd::tstring(&buffer[0]);
		}
	}

	return tstd::tstring();
}

// this function assumes it is being passed a ZERO-based weekday
// number, such as that within the SYSTEMTIME structure.

tstd::tstring WeekdayNameFromNumber(unsigned int weekdayNumber)
{
	if (weekdayNumber >= 7)
		return tstd::tstring("");

	std::vector<tstd::tstring> weekdayNames(7);

	weekdayNames[0] = TEXT("Sunday");
	weekdayNames[1] = TEXT("Monday");
	weekdayNames[2] = TEXT("Tuesday");
	weekdayNames[3] = TEXT("Wednesday");
	weekdayNames[4] = TEXT("Thursday");
	weekdayNames[5] = TEXT("Friday");
	weekdayNames[6] = TEXT("Saturday");

	return weekdayNames[weekdayNumber];
}

// this function assumes it is being passed a ONE-based integer
// corresponding to a month of the year, such as that within the
// SYSTEMTIME structure.

tstd::tstring MonthNameFromNumber(unsigned int monthNumber)
{
	if (monthNumber >= 7)
		return tstd::tstring("");

	std::vector<tstd::tstring> monthNames(12);

	// i am not a fan of one-based indexing in a language where
	// everything is expressed using zero-based notation.
	//
	// decrementing monthNumber will also prevent us from having to
	// add a superfluous 'undefined' zeroth month to the array.

	monthNumber--;

	monthNames[0] = TEXT("January");
	monthNames[1] = TEXT("February");
	monthNames[2] = TEXT("March");
	monthNames[3] = TEXT("April");
	monthNames[4] = TEXT("May");
	monthNames[5] = TEXT("June");
	monthNames[6] = TEXT("July");
	monthNames[7] = TEXT("August");
	monthNames[8] = TEXT("September");
	monthNames[9] = TEXT("October");
	monthNames[10] = TEXT("November");
	monthNames[11] = TEXT("December");

	return monthNames[monthNumber];
}

tstd::tstring FormatFilename(const SYSTEMTIME& systemTime, const tstd::tstring& inputFormat)
{
	tstd::tstringstream outputStream;

	for (tstd::tstring::size_type pos = 0; pos < inputFormat.length(); ++pos)
	{
		tstd::tchar_t currentCharacter = inputFormat[pos];

		if (currentCharacter == TEXT('%'))
		{
			tstd::tchar_t nextCharacter = inputFormat[++pos];

			switch (nextCharacter)
			{
				case TEXT('m'):
					outputStream << std::setprecision(2) << systemTime.wMonth;
					break;
				case TEXT('M'):
					outputStream << MonthNameFromNumber(systemTime.wMonth);
					break;
				case TEXT('d'):
					outputStream << std::setprecision(2) << systemTime.wDay;
					break;
				case TEXT('D'):
					outputStream << WeekdayNameFromNumber(systemTime.wDayOfWeek);
					break;
				case TEXT('y'):
					outputStream << std::setw(2) << std::setfill('0') << (systemTime.wYear % 100);
					break;
				case TEXT('Y'):
					outputStream << systemTime.wYear;
					break;
				case TEXT('h'):
					outputStream << std::setw(2) << std::setfill('0') << systemTime.wHour;
					break;
				case TEXT('H'):
					outputStream << std::setw(2) << std::setfill('0') << (systemTime.wHour % 12);
					break;
				case TEXT('c'):
					if (systemTime.wHour == 0)
						outputStream << TEXT("AM");
					else
						outputStream << (systemTime.wHour >= 12) ? TEXT("PM") : TEXT("AM");
					break;
				case TEXT('i'):
					outputStream << std::setw(2) << std::setfill('0') << systemTime.wMinute;
					break;
				case TEXT('s'):
					outputStream << std::setw(2) << std::setfill('0') << systemTime.wSecond;
					break;
				default:
					// this isn't a format specifier character.
					// output it literally.

					outputStream << nextCharacter;
					break;
			}
		}
		else
		{
			// we aren't interested in doing anything with this character.
			// output it literally.

			outputStream << currentCharacter;
		}
	}

	return outputStream.str();
}

tstd::tstring tstring_tolower(const tstd::tstring& input)
{
	tstd::tstring output;

	for (tstd::tstring::size_type i = 0; i < input.length(); ++i)
		output.push_back(std::tolower(input[i], std::locale()));

	return output;
}

tstd::tstring tstring_toupper(const tstd::tstring& input)
{
	tstd::tstring output;

	for (tstd::tstring::size_type i = 0; i < input.length(); ++i)
		output.push_back(std::toupper(input[i], std::locale()));

	return output;
}