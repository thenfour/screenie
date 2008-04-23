#include <windows.h>

#include "snarl.h"
#include "snarldefs.h"

namespace Snarl
{

	//////////////////////////////////////////////////////////////////////////
	// helper stuff
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	LRESULT SendCopyData(HWND hWnd, T& data)
	{
		COPYDATASTRUCT cds = { 0 };

		cds.dwData = 2;
		cds.cbData = sizeof(data);
		cds.lpData = &data;

		DWORD_PTR result;
		::SendMessageTimeout(hWnd, WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&cds), SMTO_ABORTIFHUNG, 100, &result);

		return result;
	}

	template<typename T>
	LRESULT SendSnarlData(T& data)
	{
		HWND hWndSnarl = GetSnarlWindow();
		if (::IsWindow(hWndSnarl))
			return SendCopyData(hWndSnarl, data);

		return 0;
	}

	int UTF16ToUTF8(const wchar_t* widestr, UINT widelen, char* mstr, UINT mlen)
	{
		CPINFO cpinfo = { 0 };
		GetCPInfo(CP_UTF8, &cpinfo);

		int length = ::WideCharToMultiByte(CP_UTF8, 0, widestr, widelen, mstr, mlen, 0, 0);
		mstr[min(length, mlen - 1)] = 0;

		return length;
	}

	//////////////////////////////////////////////////////////////////////////
	// implementation
	//////////////////////////////////////////////////////////////////////////

	MessageID ShowMessage(const wchar_t* title, const wchar_t* text, long timeout, const wchar_t* iconpath, HWND hWndReply, UINT uReplyMsg,
		const wchar_t* soundpath, const wchar_t* msgclass)
	{
		SNARLSTRUCTEX snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_EX_SHOW;
		snstruct.timeout = timeout;
		snstruct.lngData2 = reinterpret_cast<long>(hWndReply);
		snstruct.id = uReplyMsg;

		UTF16ToUTF8(title, lstrlenW(title), snstruct.title, SNARL_STRING_LENGTH);
		UTF16ToUTF8(text, lstrlenW(text), snstruct.text, SNARL_STRING_LENGTH);
		UTF16ToUTF8(iconpath, lstrlenW(iconpath), snstruct.icon, SNARL_STRING_LENGTH);
		UTF16ToUTF8(msgclass, lstrlenW(msgclass), snstruct.snarlClass, SNARL_STRING_LENGTH);

		MessageID id = static_cast<MessageID>(SendSnarlData(snstruct));

		return id;
	}

	bool UpdateMessage(MessageID id, const wchar_t* title, const wchar_t* text, const wchar_t* iconpath)
	{
		SNARLSTRUCTEX snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_UPDATE;
		snstruct.id = id;

		UTF16ToUTF8(title, lstrlenW(title), snstruct.title, SNARL_STRING_LENGTH);
		UTF16ToUTF8(text, lstrlenW(text), snstruct.text, SNARL_STRING_LENGTH);
		UTF16ToUTF8(iconpath, lstrlenW(iconpath), snstruct.icon, SNARL_STRING_LENGTH);

		return SendSnarlData(snstruct) != 0;
	}

	bool HideMessage(MessageID id)
	{
		SNARLSTRUCT snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_HIDE;
		snstruct.id = id;

		return SendSnarlData(snstruct) != 0;
	}

	bool IsMessageVisible(MessageID id)
	{
		SNARLSTRUCT snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_IS_VISIBLE;
		snstruct.id = id;

		return SendSnarlData(snstruct) != 0;
	}

	bool RegisterConfig(HWND hWndApp, const wchar_t* appname, UINT uReplyMsg, const wchar_t* icon)
	{
		SNARLSTRUCT snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_REGISTER_CONFIG_WINDOW_2;
		snstruct.lngData2 = reinterpret_cast<long>(hWndApp);
		snstruct.id = uReplyMsg;

		UTF16ToUTF8(appname, lstrlenW(appname), snstruct.title, SNARL_STRING_LENGTH);
		UTF16ToUTF8(icon, lstrlenW(icon), snstruct.icon, SNARL_STRING_LENGTH);

		return SendSnarlData(snstruct) != 0;
	}

	bool RevokeConfig(HWND hWndApp)
	{
		SNARLSTRUCT snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_REVOKE_CONFIG_WINDOW;
		snstruct.lngData2 = reinterpret_cast<long>(hWndApp);

		return SendSnarlData(snstruct) != 0;
	}

	bool RegisterAlert(const wchar_t* appname, const wchar_t* msgclass)
	{
		SNARLSTRUCT snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_REGISTER_ALERT;

		UTF16ToUTF8(appname, lstrlenW(appname), snstruct.title, SNARL_STRING_LENGTH);
		UTF16ToUTF8(msgclass, lstrlenW(msgclass), snstruct.text, SNARL_STRING_LENGTH);

		return SendSnarlData(snstruct) != 0;
	}

	HWND GetSnarlWindow()
	{
		return ::FindWindow(NULL, L"Snarl");
	}

	ServiceType GetCurrentServiceType()
	{
		SNARLSTRUCT snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_GET_GRUMBLE_INFO;

		LRESULT result = SendSnarlData(snstruct);

		return ServiceTypeSnarl;
	}

}