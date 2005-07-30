

#ifndef SCREENIE_WELCOMEWND_HPP
#define SCREENIE_WELCOMEWND_HPP


#include "image.hpp"
#include "animbitmap.h"
#include "xversion.h"


template<bool m_nag>// if false, assume we are showing welcome.
class CWelcomeWindow : public CWindowImpl<CWelcomeWindow>
{
  static const COLORREF DarkGreen = RGB(23,43,18);
  static const COLORREF DarkRed = RGB(107,23,21);
public:
  CWelcomeWindow() :
    m_isDone(false)
  {
  }

	BEGIN_MSG_MAP(CWelcomeWindow)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkGnd)
    MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCTLCOLORSTATIC)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

    COMMAND_ID_HANDLER(IDOK, OnContinue)
	END_MSG_MAP()

  void DoSortofModal()
  {
    Create(0, 0, 0, WS_POPUP);
    ShowWindow(SW_NORMAL);
    MSG msg;
    while(GetMessage(&msg, 0, 0, 0))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if(m_isDone)
      {
        break;
      }
    }
  }

  LRESULT OnContinue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  {
    DestroyWindow();
    return 0;
  }

  // this will fake the static control into being transparent.
  LRESULT OnCTLCOLORSTATIC(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    if((HWND)lParam == m_link.m_hWnd)
    {
      bHandled = TRUE;
      const HDC hStaticDC = reinterpret_cast<HDC>(wParam);
      SetBkMode(hStaticDC, TRANSPARENT);
      return reinterpret_cast<LRESULT>( GetStockObject(NULL_BRUSH) );
    }
    return 0;
  } 

  void DrawText_(int x, int y, int w, int h, bool center, COLORREF textColor, int fontSize, int fontWeight, const tstd::tstring& text)
  {
    CFont font;
    font.CreateFont(fontSize, 0, 0, 0, fontWeight, 0, 0, 0, 0, 0, 0, 0, 0, _T("Arial"));
    HDC dc = m_offscreen.GetDC();
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, textColor);
    CRect rc(x, y, x + w, y + h);
    HGDIOBJ hOldFont = SelectObject(dc, font);
    DrawText(dc, text.c_str(), text.size(), &rc, (center ? DT_CENTER : 0) | DT_NOCLIP);
    SelectObject(dc, hOldFont);
  }

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
    // background image
    m_background = LoadImageResource(MAKEINTRESOURCE(IDR_SPLASHBACKGROUND), _T("PNG"));
    GetObject(m_background.handle, sizeof(BITMAP), &m_bmpBackground);

    m_offscreen.SetSize(m_bmpBackground.bmWidth, m_bmpBackground.bmHeight);
    m_offscreen.BlitFrom(m_background.handle, 0, 0, m_bmpBackground.bmWidth, m_bmpBackground.bmHeight);

    MoveWindow(0, 0, m_bmpBackground.bmWidth, m_bmpBackground.bmHeight, FALSE);
    CenterWindow();

    // hyperlink
    m_hyperlinkFont.CreateFont(22, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, _T("Arial"));
    CRect rc(26,246,170+26,30+246);
    m_link.Create(m_hWnd, &rc, _T(""), WS_CHILD | SS_CENTER | WS_VISIBLE, 0, IDC_HYPERLINK);
    m_link.SetFont(m_hyperlinkFont);
    m_link.SetLinkFont(m_hyperlinkFont);
    m_link.SetLabel( _T("http://screenie.net/"));

    Version v;
    v.FromFile(GetModuleFileNameX().c_str());

    // registration
    DrawText_(26, 272, 170, 30, true, DarkGreen, 17, FW_NORMAL, _T("Registered to:"));
    DrawText_(26, 292, 170, 30, true, DarkRed, 19, FW_BOLD, LibCC::Format().s(v.GetRegistrant()).Str());

    // "Thank you for installing"
    DrawText_(216, 8, 200, 30, false, DarkGreen, 18, FW_NORMAL, _T("Thank you for installing"));

    if(m_nag)
    {
      SetupNag();
    }
    else
    {
      SetupWelcome();
    }

		return 0;
	}

	LRESULT OnEraseBkGnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
  {
    bHandled = TRUE;
    return TRUE;
  }

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
  {
    bHandled = TRUE;
    return 0;
  }

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  {
    m_isDone = true;
    return 0;
  }

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
  {
    bHandled = TRUE;
    PAINTSTRUCT ps;
    BeginPaint(&ps);
    m_offscreen.Blit(ps.hdc, 0, 0);
    EndPaint(&ps);
    return 0;
  }
  
private:

  void SetupNag()
  {
    // "the follomwming features have been disabled"
    // "cropping"
    // "ftp uploading"
    // "if you like what you see...."
    // buy
    // continue
    CRect rc(0, 0, 75, 23);
    m_continue.Create(m_hWnd, &rc, _T("Continue"), WS_CHILD | WS_VISIBLE, 0, IDOK);
  }

  void SetupWelcome()
  {
    // "what do i do next?"
    DrawText_(216, 91, 200, 30, false, DarkGreen, 18, FW_NORMAL, _T("What do I do next?\r\nHit the Print Screen key on your\r\nkeyboard to take a screenshot."));

    // printscreen graphic
    m_printScreen = LoadImageResource(MAKEINTRESOURCE(IDR_PRTSCR), _T("PNG"));
    GetObject(m_printScreen.handle, sizeof(BITMAP), &m_bmpPrintScreen);
    m_offscreen.BlitFrom(m_printScreen.handle, 0, 0, m_bmpPrintScreen.bmWidth, m_bmpPrintScreen.bmHeight, 216, 136);

    // "to configure screenie, right-cilck...."
    DrawText_(216, 197, 200, 30, false, DarkGreen, 18, FW_NORMAL, _T("To configure Screenie, right-click on\r\nthe Screenie logo in your system tray."));

    // menu graphic
    m_menu = LoadImageResource(MAKEINTRESOURCE(IDR_TRAYSAMPLE), _T("PNG"));
    GetObject(m_menu.handle, sizeof(BITMAP), &m_bmpMenu);
    m_offscreen.BlitFrom(m_menu.handle, 0, 0, m_bmpMenu.bmWidth, m_bmpMenu.bmHeight, 216, 228);

    // continue
    CRect rc(324, 297, 324+75, 297+23);
    m_continue.Create(m_hWnd, &rc, _T("Continue"), WS_CHILD | WS_VISIBLE, 0, IDOK);

    // "do not show this window again"
  }


  AnimBitmap<32> m_offscreen;

	CHyperLink m_link;
  //CButton m_showSplash;
  CButton m_continue;
  //CButton m_buy;

  AutoGdiBitmap m_menu;
  BITMAP m_bmpMenu;

  AutoGdiBitmap m_printScreen;
  BITMAP m_bmpPrintScreen;

  AutoGdiBitmap m_background;
  BITMAP m_bmpBackground;

  bool m_isDone;

  CFont m_hyperlinkFont;
};


void ShowNagSplash()
{
  CWelcomeWindow<true> aoeu;
  aoeu.DoSortofModal();
}

void ShowSplashScreen()
{
  CWelcomeWindow<false> aoeu;
  aoeu.DoSortofModal();
}


#endif
