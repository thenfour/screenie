#include <windows.h>

#include "libcrashman.h"
#include "../handler/exception_handler.h"

#include <sstream>

namespace CrashMan
{

	google_breakpad::ExceptionHandler* g_pHandler = 0;
	CrashHandler g_CrashHandlerFn = 0;

	bool BreakpadFilterCallback(void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion)
	{
		return true;
	}

	static bool BreakpadMinidumpCallback(const wchar_t* dump_path, const wchar_t* minidump_id, void* context,
		EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool succeeded)
	{
		if (g_CrashHandlerFn == 0)
			return false;

		std::wstring filename = dump_path;
		filename += L"\\";
		filename += minidump_id;
		filename += L".dmp";

		return g_CrashHandlerFn(filename.c_str(), exinfo, context);
	}

	void Initialize(CrashHandler handler, void* pContext)
	{
		wchar_t temppath[MAX_PATH];
		::GetTempPath(MAX_PATH, temppath);

		g_CrashHandlerFn = handler;
		g_pHandler = new google_breakpad::ExceptionHandler(temppath, BreakpadFilterCallback, BreakpadMinidumpCallback, pContext, google_breakpad::ExceptionHandler::HANDLER_ALL);
	}

	void Uninitialize()
	{
		delete g_pHandler;
	}

	bool Execute(const wchar_t* handlerPath, const wchar_t* dumpfile, const StringMap& params, const StringMap& strings,
		const wchar_t* restartPath, const wchar_t* restartArgs, const wchar_t* configfile)
	{
		std::wstringstream strm;

		strm << L"/dump \"" << dumpfile << L"\"";
		if (restartArgs != 0)
			strm << L" /restart \"\"" << restartPath << L"\" " << restartArgs << "\"";
		else
			strm << L" /restart \"" << restartPath << L"\"";

		for (StringMap::const_iterator i = params.begin(); i != params.end(); ++i)
			strm << L" /param \"" << i->first << L"=" << i->second << L"\"";

		for (StringMap::const_iterator i = strings.begin(); i != strings.end(); ++i)
			strm << L" /str \"" << i->first << L"=" << i->second << L"\"";

		if (configfile != 0)
			strm << L" /config " << configfile;

		PROCESS_INFORMATION pi = { 0 };
		STARTUPINFO si = { 0 };

		si.cb = sizeof(si);
		si.wShowWindow = SW_SHOW;
		si.dwFillAttribute = STARTF_USESHOWWINDOW;

		if (::CreateProcess(handlerPath, (LPWSTR)strm.str().c_str(), NULL, NULL, FALSE, 0, 0, 0, &si, &pi))
		{
			::CloseHandle(pi.hProcess);
			::CloseHandle(pi.hThread);

			return true;
		}

		return false;
	}

}