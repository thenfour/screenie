#ifndef LIBGRUMBLE_SNARLEVENT_H
#define LIBGRUMBLE_SNARLEVENT_H

#include "snarl.h"

#define GRUMBLE_EVENTHANDLER_WNDCLASS L"GrumbleSnarlEventHandler"
#define GRUMBLE_EVENTHANDLER_MESSAGE L"GrumbleSnarlEventHandlerMsg"

namespace Snarl
{

//  EXAMPLE
//	
// 	class MyEventHandler : public Snarl::EventHandler
// 	{
// 	public:
// 		MyEventHandler()
// 		{
// 			Create();
// 		}
// 	protected:
// 		void OnSnarlStartup()
// 		{
// 			// here is where you would register your alerts
//
//			Snarl::RegisterConfig(GetHwnd(), L"MyAppName", GetReplyMsg(), L"c:\\youricon.png");
//
//			Snarl::RegisterAlert(L"MyAppName", L"MySampleAlert");
// 		}
// 
// 		void OnSnarlShutdown()
// 		{
// 			// here is where you would 'revoke' your config
//			Snarl::RevokeConfig(GetHwnd());
// 		}
// 
// 		void OnMessageClicked(MessageID id)
// 		{
// 			// a popup was clicked
// 		}
// 
// 		void OnMessageTimeout(MessageID id)
// 		{
// 			// a popup timed out
// 		}
// 	private:
// 		// your private data, a map of ids -> info about popups, etc.
// 	}
//
//

	//////////////////////////////////////////////////////////////////////////
	// EventHandler
	//
	// Base class for handling Snarl events. Derive and implement virtuals
	//////////////////////////////////////////////////////////////////////////

	class EventHandler
	{
	public:
		enum ClickType
		{
			LeftClick,
			RightClick
		};

		EventHandler()
		{
			m_uSnarlMsg = Snarl::GetGlobalMessage();
			m_uReplyMsg = ::RegisterWindowMessageW(GRUMBLE_EVENTHANDLER_MESSAGE);

			m_hWnd = NULL;
		}

		virtual ~EventHandler()
		{
			Destroy();
		}

		UINT GetGlobalMessage() const { return m_uSnarlMsg; }
		UINT GetReplyMessage() const { return m_uReplyMsg; }

		HWND GetHwnd() const { return m_hWnd; }

		bool Create()
		{
			RegisterWndClass();

			HWND hWnd = ::CreateWindowExW(WS_EX_APPWINDOW, GRUMBLE_EVENTHANDLER_WNDCLASS, NULL,
				WS_POPUPWINDOW, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), this);

			if (hWnd != NULL)
			{
				m_hWnd = hWnd;
				return true;
			}

			return false;
		}

		void Destroy()
		{
			if (::IsWindow(m_hWnd))
				::DestroyWindow(m_hWnd);
		}
	protected:
		virtual void OnSnarlStartup() { }
		virtual void OnSnarlShutdown() { }

		virtual void OnMessageClicked(MessageID id, ClickType clicktype) { }
		virtual void OnMessageTimeout(MessageID id) { }

		virtual bool HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)
		{
			if (m_uSnarlMsg && uMsg == m_uSnarlMsg)
			{
				if (wParam == SNARL_LAUNCHED)
					OnSnarlStartup();
				else if (wParam == SNARL_QUIT)
					OnSnarlShutdown();
			}
			else if (m_uReplyMsg && uMsg == m_uReplyMsg)
			{
				// I don't know why Snarl considers a left click an 'ack' and a right click a 'click'...

				if (wParam == SNARL_NOTIFICATION_ACK)
					OnMessageClicked(static_cast<MessageID>(lParam), LeftClick);
				if (wParam == SNARL_NOTIFICATION_CLICKED)
					OnMessageClicked(static_cast<MessageID>(lParam), RightClick);
				else if (wParam == SNARL_NOTIFICATION_TIMED_OUT)
					OnMessageTimeout(static_cast<MessageID>(lParam));
			}

			return false;
		}

		// you probably won't ever need to use these
		void SetGlobalMessage(UINT uMsg) { m_uSnarlMsg = uMsg; }
		void SetReplyMessage(UINT uMsg) { m_uReplyMsg = uMsg; }
	private:
		HWND m_hWnd;
		UINT m_uSnarlMsg;
		UINT m_uReplyMsg;

		static bool RegisterWndClass()
		{
			WNDCLASSEXW wcex = { 0 };

			wcex.cbSize = sizeof(wcex);
			wcex.hInstance = GetModuleHandle(NULL);
			wcex.lpfnWndProc = WndProc;
			wcex.lpszClassName = GRUMBLE_EVENTHANDLER_WNDCLASS;

			return RegisterClassExW(&wcex) != 0;
		}

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			if (uMsg == WM_NCCREATE)
			{
				CREATESTRUCT* pCS = reinterpret_cast<CREATESTRUCT*>(lParam);
				EventHandler* pThis = reinterpret_cast<EventHandler*>(pCS->lpCreateParams);

				pThis->m_hWnd = hWnd;

				::SetPropW(hWnd, L"GrumbleThis", reinterpret_cast<HANDLE>(pThis));
			}

			EventHandler* pThis = reinterpret_cast<EventHandler*>(::GetPropW(hWnd, L"GrumbleThis"));
			if (pThis != 0)
			{
				LRESULT result;
				if (pThis->HandleMessage(uMsg, wParam, lParam, result))
					return result;
			}

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// SimpleClient
	//
	// Simple wrapper for the Snarl functions that take message/HWND params
	//////////////////////////////////////////////////////////////////////////

	class SimpleClient : public EventHandler
	{
	public:
		bool RegisterConfig(const wchar_t* appname, const wchar_t* icon)
		{
			return Snarl::RegisterConfig(GetHwnd(), appname, GetReplyMessage(), icon);
		}

		bool RevokeConfig()
		{
			return Snarl::RevokeConfig(GetHwnd());
		}

		MessageID ShowMessage(const wchar_t* title, const wchar_t* text, long timeout, const wchar_t* path, const wchar_t* soundpath = 0, const wchar_t* msgclass = 0)
		{
			return Snarl::ShowMessage(title, text, timeout, path, GetHwnd(), GetReplyMessage(), soundpath, msgclass);
		}
	};

}

#endif