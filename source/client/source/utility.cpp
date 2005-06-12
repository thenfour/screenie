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

#include "screenshotdestination.hpp"
#include "internet.hpp"

using namespace LibCC;

bool MakeDestinationFilename(tstd::tstring& filename, const SYSTEMTIME& systemTime, const tstd::tstring& mimeType, const tstd::tstring& formatString)
{
	ImageCodecsEnum imageCodecs;
	Gdiplus::ImageCodecInfo* codecInfo = imageCodecs.GetCodecByMimeType(mimeType.c_str());

	if (codecInfo != NULL)
	{
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
      case 'a':
        outputStream << ((systemTime.wHour >= 12) ? _T("PM") : _T("AM"));
        break;

				case TEXT('m'):
          outputStream << Format().ui<10,2>(systemTime.wMonth).Str();
					break;
				case TEXT('M'):
					outputStream << MonthNameFromNumber(systemTime.wMonth);
					break;
				case TEXT('d'):
          outputStream << Format().ui<10,2>(systemTime.wDay).Str();
					break;
				case TEXT('D'):
					outputStream << WeekdayNameFromNumber(systemTime.wDayOfWeek);
					break;
				case TEXT('y'):
					outputStream << Format().ui<10,2>(systemTime.wYear % 100).Str();
					break;
				case TEXT('Y'):
					outputStream << Format().ui<10,4>(systemTime.wYear).Str();
					break;
				case TEXT('h'):
					outputStream << Format().ui<10,2>(systemTime.wHour).Str();
					break;
				case TEXT('H'):
					outputStream << Format().ui<10,2>(systemTime.wHour % 12).Str();
					break;
				case TEXT('c'):
					if (systemTime.wHour == 0)
						outputStream << TEXT("AM");
					else
						outputStream << (systemTime.wHour >= 12) ? TEXT("PM") : TEXT("AM");
					break;
				case TEXT('i'):
          outputStream << Format().ui<10,2>(systemTime.wMinute).Str();
					break;
				case TEXT('s'):
          outputStream << Format().ui<10,2>(systemTime.wSecond).Str();
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


LibCC::Result UploadFTPFile(ScreenshotDestination& dest, const tstd::tstring& localFile, const tstd::tstring& remoteFileName, DWORD bufferSize, UploadFTPFileProgressProc_T pProc, void* pUser)
{
  // open / login
	WinInetHandle internet = ::InternetOpen("Screenie v1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (internet.handle == NULL)
	{
    return LibCC::Result(E_FAIL, LibCC::Format("Failed to connect to the FTP site.  Technical info: [InternetOpen] [gle: %] [igle: %]").gle().s(InternetGetLastResponseInfoX()).Str());
	}

	WinInetHandle ftp = ::InternetConnect(internet.handle,
		dest.ftp.hostname.c_str(), dest.ftp.port,
		dest.ftp.username.c_str(), dest.ftp.DecryptPassword().c_str(),
		INTERNET_SERVICE_FTP, 0, 0);
  if (ftp.handle == NULL)
	{
    return LibCC::Result(E_FAIL, LibCC::Format("Failed to connect to the FTP site.  Technical info: [InternetConnect] [gle: %] [igle: %]").gle().s(InternetGetLastResponseInfoX()).Str());
	}

  if (!::FtpSetCurrentDirectory(ftp.handle, dest.ftp.remotePath.c_str()))
	{
    return LibCC::Result(E_FAIL, LibCC::Format("Failed to set the current directory remotely.  Technical info: [FtpSetCurrentDirectory] [gle: %] [igle: %]").gle().s(InternetGetLastResponseInfoX()).Str());
	}

  // open the remote file
  WinInetHandle remoteFile = ::FtpOpenFile(ftp.handle, remoteFileName.c_str(), GENERIC_WRITE, FTP_TRANSFER_TYPE_BINARY, 0);
  if(remoteFile.handle == NULL)
  {
    return LibCC::Result(E_FAIL, LibCC::Format("Failed to open the remote file for writing.  Technical info: [FtpOpenFile] [gle: %] [igle: %]").gle().s(InternetGetLastResponseInfoX()).Str());
  }

  // open the local file.
  Win32Handle hFile = CreateFile(localFile.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  if(!LibCC::IsValidHandle(hFile.val))
  {
    return LibCC::Result(E_FAIL, LibCC::Format("Unable to open the temp file for reading.  Technical info: [CreateFile] [gle: %]").gle().Str());
  }

  LibCC::Blob<BYTE> buffer(bufferSize);
  DWORD total = GetFileSize(hFile.val, 0);
  DWORD progress = 0;
  for(;;)
  {
    // read in
    DWORD br;
    BOOL ret = ReadFile(hFile.val, buffer.GetBuffer(), bufferSize, &br, 0);
    if(FALSE == ret)
    {
      return LibCC::Result(E_FAIL, LibCC::Format("Error reading from the temp file.  Technical info: [ReadFile] [gle: %]").gle().Str());
    }
    if(br == 0)
    {
      break;
    }

    // write out
    DWORD bw;
    if(FALSE == InternetWriteFile(remoteFile.handle, buffer.GetBuffer(), br, &bw))
    {
      return LibCC::Result(E_FAIL, LibCC::Format("Error writing to the remote file.  Technical info: [InternetWriteFile] [gle: %] [igle: %]").gle().s(InternetGetLastResponseInfoX()).Str());
    }

    if(bw != br)
    {
      return LibCC::Result(E_FAIL, LibCC::Format("Error writing to the remote file.  Technical info: [InternetWriteFile] [gle: %] [igle: %]").gle().s(InternetGetLastResponseInfoX()).Str());
    }

    progress += bw;

    if(pProc)
    {
      if(!pProc(progress, total, pUser))
      {
        return LibCC::Result(E_FAIL, LibCC::Format("User canceled.").Str());
      }
    }
  }

	return S_OK;
}


