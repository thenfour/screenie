#ifndef LIBGRUMBLE_SNARL_H
#define LIBGRUMBLE_SNARL_H

namespace Snarl
{

	typedef long MessageID;
	static const MessageID MESSAGE_ID_INVALID = -1;

	enum : int
	{
		SNARL_LAUNCHED = 1,
		SNARL_QUIT = 2
	};

	enum : int
	{
		SNARL_NOTIFICATION_CLICKED = 32,
		SNARL_NOTIFICATION_TIMED_OUT = 33,
		SNARL_NOTIFICATION_ACK = 34
	};

	static const int SNARL_STRING_LENGTH = 1024;

	enum SNARL_COMMANDS
	{
		SNARL_SHOW = 1,
		SNARL_HIDE,
		SNARL_UPDATE,
		SNARL_IS_VISIBLE,
		SNARL_GET_VERSION,
		SNARL_REGISTER_CONFIG_WINDOW,
		SNARL_REVOKE_CONFIG_WINDOW,
		SNARL_REGISTER_ALERT,
		SNARL_REVOKE_ALERT,
		SNARL_REGISTER_CONFIG_WINDOW_2,
		SNARL_GET_VERSION_EX,
		SNARL_EX_SHOW = 0x20,

		// Grumble stuff
		SNARL_GET_GRUMBLE_INFO = 0x573
	};

	struct SNARLSTRUCT
	{
		SNARL_COMMANDS cmd;
		long id;
		long timeout;
		long lngData2;
		char title[SNARL_STRING_LENGTH];
		char text[SNARL_STRING_LENGTH];
		char icon[SNARL_STRING_LENGTH];
	};

	struct SNARLSTRUCTEX
	{
		SNARL_COMMANDS cmd;
		long id;
		long timeout;
		long lngData2;
		char title[SNARL_STRING_LENGTH];
		char text[SNARL_STRING_LENGTH];
		char icon[SNARL_STRING_LENGTH];
		char snarlClass[SNARL_STRING_LENGTH];
		char extra[SNARL_STRING_LENGTH];
		char extra2[SNARL_STRING_LENGTH];
		long reserved1;
		long reserved2;
	};

	namespace impl
	{

		template<typename T>
		LRESULT SendCopyData(HWND hWnd, T& data);

		template<typename T>
		LRESULT SendSnarlData(T& data);

		inline int UTF16ToUTF8(const wchar_t* widestr, UINT widelen, char* mstr, UINT mlen);

	}



	//
	// ShowMessage()
	// Show a notification message.
	//
	// title - the message title
	// text - the message text (the long one)
	// icon - path to an icon for this message. ICO, PNG, JPG, GIF, or BMP formats are supported by both Snarl and Grumble.
	// timeout - the number of seconds to leave the message on screen. Grumble ignores this.
	// hWndReply - the window to send uReplyMsg to after the message popup has been clicked or it has timed out
	// uReplyMsg - the message to send to hWndReply; see above
	// soundpath - path to a sound for this message; use NULL for none. Grumble ignores this.
	// msgclass - the 'class' of your notification; use NULL for none. the same value you would pass to RegisterAlert()
	//
	// Returns the unique integer ID of the popup message.
	//

	inline MessageID ShowMessage(const wchar_t* title, const wchar_t* text, long timeout, const wchar_t* iconpath, HWND hWndReply, UINT uReplyMsg,
		const wchar_t* soundpath = 0, const wchar_t* msgclass = 0)
	{
		SNARLSTRUCTEX snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_EX_SHOW;
		snstruct.timeout = timeout;
		snstruct.lngData2 = reinterpret_cast<long>(hWndReply);
		snstruct.id = uReplyMsg;

		impl::UTF16ToUTF8(title, lstrlenW(title), snstruct.title, SNARL_STRING_LENGTH);
		impl::UTF16ToUTF8(text, lstrlenW(text), snstruct.text, SNARL_STRING_LENGTH);
		impl::UTF16ToUTF8(iconpath, lstrlenW(iconpath), snstruct.icon, SNARL_STRING_LENGTH);

		if (msgclass)
			impl::UTF16ToUTF8(msgclass, lstrlenW(msgclass), snstruct.snarlClass, SNARL_STRING_LENGTH);

		MessageID id = static_cast<MessageID>(impl::SendSnarlData(snstruct));

		return id;
	}

	//
	// UpdateMessage()
	// Change the properties of an existing popup message.
	//
	// id - the ID of the existing message
	// title - 
	// text - 
	// iconpath - 
	//
	// Returns success.
	//

	inline bool UpdateMessage(MessageID id, const wchar_t* title, const wchar_t* text, const wchar_t* iconpath)
	{
		SNARLSTRUCTEX snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_UPDATE;
		snstruct.id = id;

		impl::UTF16ToUTF8(title, lstrlenW(title), snstruct.title, SNARL_STRING_LENGTH);
		impl::UTF16ToUTF8(text, lstrlenW(text), snstruct.text, SNARL_STRING_LENGTH);
		impl::UTF16ToUTF8(iconpath, lstrlenW(iconpath), snstruct.icon, SNARL_STRING_LENGTH);

		return impl::SendSnarlData(snstruct) != 0;
	}

	//
	// HideMessage()
	// Hides an existing popup message.
	//
	// id - the ID of the existing message
	//
	// Returns success.
	//

	inline bool HideMessage(MessageID id)
	{
		SNARLSTRUCT snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_HIDE;
		snstruct.id = id;

		return impl::SendSnarlData(snstruct) != 0;
	}

	//
	// HideMessage()
	// Hides an existing popup message.
	//
	// id - the ID of the existing message
	//
	// Returns success.
	//

	inline bool IsMessageVisible(MessageID id)
	{
		SNARLSTRUCT snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_IS_VISIBLE;
		snstruct.id = id;

		return impl::SendSnarlData(snstruct) != 0;
	}

	//
	// RegisterConfig()
	// Register your application with Snarl/Grumble.
	//
	// hWndApp - the window handle of *your* application. If you don't have one, use NULL.
	// appname - the name of your application
	// uReplyMsg - the message to be sent to hWndApp
	// icon - path to an icon for your application
	//
	// Returns success.
	//

	inline bool RegisterConfig(HWND hWndApp, const wchar_t* appname, UINT uReplyMsg, const wchar_t* icon)
	{
		SNARLSTRUCT snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_REGISTER_CONFIG_WINDOW_2;
		snstruct.lngData2 = reinterpret_cast<long>(hWndApp);
		snstruct.id = uReplyMsg;

		impl::UTF16ToUTF8(appname, lstrlenW(appname), snstruct.title, SNARL_STRING_LENGTH);
		impl::UTF16ToUTF8(icon, lstrlenW(icon), snstruct.icon, SNARL_STRING_LENGTH);

		return impl::SendSnarlData(snstruct) != 0;
	}

	//
	// RevokeConfig()
	// Properly unregisters your application with Snarl/Grumble.
	//
	// hWndApp - the window handle of your application. should be the same value passed to RegisterConfig
	//
	// Returns success.
	//

	inline bool RevokeConfig(HWND hWndApp)
	{
		SNARLSTRUCT snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_REVOKE_CONFIG_WINDOW;
		snstruct.lngData2 = reinterpret_cast<long>(hWndApp);

		return impl::SendSnarlData(snstruct) != 0;
	}

	//
	// RegisterAlert()
	// Registers an alert with Snarl/Grumble. This way, the user can selectively enable/disable the alerts.
	//
	// appname - the name of your application. This should be the same as in RegisterConfig()
	// msgclass - a description of the 'type' of this message
	//
	// Returns success.
	//

	inline bool RegisterAlert(const wchar_t* appname, const wchar_t* msgclass)
	{
		SNARLSTRUCT snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_REGISTER_ALERT;

		impl::UTF16ToUTF8(appname, lstrlenW(appname), snstruct.title, SNARL_STRING_LENGTH);
		impl::UTF16ToUTF8(msgclass, lstrlenW(msgclass), snstruct.text, SNARL_STRING_LENGTH);

		return impl::SendSnarlData(snstruct) != 0;
	}

	//
	// GetSnarlWindow()
	// Gets the handle of the Snarl (or Grumble) window.
	//
	// Returns the window handle.
	//

	inline HWND GetSnarlWindow()
	{
		return ::FindWindowW(NULL, L"Snarl");
	}

	//
	// GetGlobalMessage()
	// Gets the ID for the system-wide Snarl message that is broadcast to all windows
	//
	// Returns the message ID.
	//

	inline UINT GetGlobalMessage()
	{
		return ::RegisterWindowMessageW(L"SnarlGlobalMessage");
	}

	//////////////////////////////////////////////////////////////////////////

	enum ServiceType
	{
		ServiceTypeNone,
		ServiceTypeSnarl,
		ServiceTypeGrumble
	};

	//
	// GetCurrentServiceType()
	// Determines whether Snarl, Grumble, or neither, is running.
	//
	// Returns one of ServiceType
	//

	inline ServiceType GetCurrentServiceType()
	{
		SNARLSTRUCT snstruct;
		ZeroMemory(&snstruct, sizeof(snstruct));

		snstruct.cmd = SNARL_GET_GRUMBLE_INFO;

		LRESULT result = impl::SendSnarlData(snstruct);

		return ServiceTypeSnarl;
	}


	namespace impl
	{

		//////////////////////////////////////////////////////////////////////////
		// helper stuff the outside doesn't need access to
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

		inline int UTF16ToUTF8(const wchar_t* widestr, UINT widelen, char* mstr, UINT mlen)
		{
			CPINFO cpinfo = { 0 };
			GetCPInfo(CP_UTF8, &cpinfo);

			UINT length = ::WideCharToMultiByte(CP_UTF8, 0, widestr, widelen, mstr, mlen, 0, 0);
			mstr[min(length, mlen - 1)] = 0;

			return length;
		}

	}

}

#endif