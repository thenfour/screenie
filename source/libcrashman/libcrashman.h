#ifndef LIBCRASHMAN_H
#define LIBCRASHMAN_H

#include <string>
#include <map>

namespace CrashMan
{

	//////////////////////////////////////////////////////////////////////////

	typedef bool (*CrashHandler)(const wchar_t* dumpfile, EXCEPTION_POINTERS* pExInfo, void* pContext);

	void Initialize(CrashHandler handler, void* pContext = 0);
	void Uninitialize();

	//////////////////////////////////////////////////////////////////////////

	typedef std::map<std::wstring, std::wstring> StringMap;

	bool Execute(const wchar_t* handlerPath, const wchar_t* dumpfile, const StringMap& params, const StringMap& strings,
		const wchar_t* restartPath, const wchar_t* restartArgs = 0, const wchar_t* configfile = 0);

	//////////////////////////////////////////////////////////////////////////

	// these are the strings to override in the 'strings' map passed to Execute

	const wchar_t STR_URL[] = L"url";

	const wchar_t STR_VENDOR[] = L"vendor";
	const wchar_t STR_PRODUCT[] = L"product";

	const wchar_t STR_OK[] = L"ok";
	const wchar_t STR_CANCEL[] = L"cancel";

	const wchar_t STR_MAINDLG_TITLE[] = L"maindlgtitle";
	const wchar_t STR_MAINDLG_DESCRIPTION[] = L"maindlgdescription";
	const wchar_t STR_MAINDLG_DETAILS[] = L"maindlgdetails";
	const wchar_t STR_MAINDLG_SENDREPORTCHECKBOX[] = L"maindlgsendreportcheckbox";
	const wchar_t STR_MAINDLG_VIEWREPORT[] = L"maindlgviewreport";
	const wchar_t STR_MAINDLG_CLOSE[] = L"maindlgclose";
	const wchar_t STR_MAINDLG_RESTART[] = L"maindlgrestart";
	const wchar_t STR_MAINDLG_SENDREPORT[] = L"maindlgsendreport";
	const wchar_t STR_MAINDLG_SENDING[] = L"maindlgsending";

	const wchar_t STR_VIEWREPORTDLG_TITLE[] = L"viewreportdlgtitle";
	const wchar_t STR_VIEWREPORTDLG_TECHINFO[] = L"viewreportdlgtitletechnicalinfo";

	//////////////////////////////////////////////////////////////////////////

}

#endif