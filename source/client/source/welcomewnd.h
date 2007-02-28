

#ifndef SCREENIE_WELCOMEWND_HPP
#define SCREENIE_WELCOMEWND_HPP


#include "image.hpp"
#include "animbitmap.h"
#include "xversion.h"
#include "screenshotoptions.hpp"
#include "checkbox.h"


template<bool m_nag>// if false, assume we are showing welcome.
class CWelcomeWindow : public CWindowImpl< CWelcomeWindow<m_nag> >
{
  static const COLORREF DarkGreen = RGB(23,43,18);
  static const COLORREF DarkRed = RGB(107,23,21);
  static const int IDC_ShowSplash = 1000;

  static const int ButtonWidth = 116;
  static const int ButtonHeight = 39;
  static const int LeftPane = 242;// the welcome screen is sorta divided in 2 panes.  this is the width of the left one
  static const int TotalWidth = 521;
  static const int RightPane = TotalWidth - LeftPane;

public:
  CWelcomeWindow(ScreenshotOptions& opt) :
    m_isDone(false),
    m_options(opt)
  {
#ifdef CRIPPLED
    m_next = 0;
#endif
  }

	BEGIN_MSG_MAP(CWelcomeWindow)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkGnd)
    MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCTLCOLORSTATIC)
    
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

    COMMAND_ID_HANDLER(IDC_BUY, OnBuy)
    COMMAND_ID_HANDLER(IDOK, OnContinue)
	END_MSG_MAP()

  void GoForIt(CWelcomeWindow<false>* next = 0)
  {
#ifdef CRIPPLED
    m_next = next;
#endif
    if(m_nag)
    {
      Create(0, 0, _T("Screenie Demo"), WS_POPUP);
    }
    else
    {
      Create(0, 0, _T("Screenie Welcome"), WS_POPUP);
    }
    ShowWindow(SW_NORMAL);
  }

  void EnsureDestroyed()
  {
    if(IsWindow())
    {
      DestroyWindow();
    }
  }

  LRESULT OnBuy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  {
    m_link.Navigate();
    return 0;
  }
  LRESULT OnContinue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  {
    if(!m_nag)
    {
      m_options.ShowSplash(!m_showSplash.IsChecked());
    }
#ifdef CRIPPLED
    if(m_next)
    {
      m_next->GoForIt(0);
    }
#endif
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

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
    // setup the default GUI stuff

    // background image
    AutoGdiBitmap background;
    background = LoadBitmapResource(MAKEINTRESOURCE(IDR_SPLASHBACKGROUND), _T("PNG"));
    GetObject(background.handle, sizeof(BITMAP), &m_bmpBackground);

    m_offscreen.SetSize(m_bmpBackground.bmWidth, m_bmpBackground.bmHeight);
    m_offscreen.BlitFrom(background.handle, 0, 0, m_bmpBackground.bmWidth, m_bmpBackground.bmHeight);

    MoveWindow(0, 0, m_bmpBackground.bmWidth, m_bmpBackground.bmHeight, FALSE);
    CenterWindow();

    // hyperlink
    m_hyperlinkFont.CreateFont(24, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, _T("Arial"));
    CRect rc(0,290,LeftPane, 30 + 290);
    m_link.Create(m_hWnd, &rc, _T(""), WS_CHILD | SS_CENTER | WS_VISIBLE, 0, IDC_HYPERLINK);
    m_link.SetFont(m_hyperlinkFont);
    m_link.SetLinkFont(m_hyperlinkFont);
    m_link.SetLabel(_T("http://screenie.net/"));
    m_link.SetHyperLink(_T("http://screenie.net/"));

    Version v;
    v.FromFile(GetModuleFileNameX().c_str());

//     // registration
//     DrawText_(0, 337, LeftPane, 30, true, DarkGreen, 19, FW_NORMAL, _T("Registered to:"));
//     DrawText_(0, 362, LeftPane, 422-362, true, DarkRed, 21, FW_BOLD, LibCC::Format().s(v.GetRegistrant()).Str(), true);

    // "Thank you for installing" is now on the graphic
    // DrawText_(LeftPane, 10, RightPane, 30, false, DarkGreen, 25, FW_NORMAL, _T("Thank you for installing"));

    // set up images for the continue button.
    m_continueImages.Create(ButtonWidth, ButtonHeight, ILC_COLOR32, 3, 0);
    int normal = m_continueImages.Add(LoadBitmapResource(MAKEINTRESOURCE(IDR_CONTINUE_NORMAL), _T("BIN")).handle);
    int pushed = m_continueImages.Add(LoadBitmapResource(MAKEINTRESOURCE(IDR_CONTINUE_PUSHED_HOVER), _T("BIN")).handle);
    m_continue.SetImageList(m_continueImages.m_hImageList);
    m_continue.SetImages(normal, pushed);

      SetupWelcome();

		return 0;
    }
  
private:

  void SetupWelcome()
  {
    // "what do i do next?"
	// DrawText_(LeftPane, 106, RightPane, 30, false, DarkGreen, 18, FW_NORMAL, _T("Instructions:"));
    DrawText_(LeftPane, 150, RightPane, 30, false, DarkGreen, 18, FW_NORMAL, _T("Hit the Print Screen key on your\r\nkeyboard to take a screenshot."));

    // printscreen graphic
    AutoGdiBitmap printScreen = LoadBitmapResource(MAKEINTRESOURCE(IDR_PRTSCR), _T("BIN"));
    BITMAP bmpPrintScreen;
    GetObject(printScreen.handle, sizeof(BITMAP), &bmpPrintScreen);
    m_offscreen.BlitFrom(printScreen.handle, 0, 0, bmpPrintScreen.bmWidth, bmpPrintScreen.bmHeight, LeftPane, 193);

    // "to configure screenie, right-cilck...."
    DrawText_(LeftPane, 255, RightPane, 30, false, DarkGreen, 18, FW_NORMAL, _T("To configure Screenie, right-click on\r\nthe Screenie logo in your system tray."));

    // menu graphic
    AutoGdiBitmap menu = LoadBitmapResource(MAKEINTRESOURCE(IDR_TRAYSAMPLE), _T("BIN"));
    BITMAP bmpMenu;
    GetObject(menu.handle, sizeof(BITMAP), &bmpMenu);
    m_offscreen.BlitFrom(menu.handle, 0, 0, bmpMenu.bmWidth, bmpMenu.bmHeight, LeftPane, 300);

    CRect rc;

    // continue
    rc.SetRect(LeftPane + ((RightPane - ButtonWidth) / 2), 379, LeftPane + ((RightPane - ButtonWidth) / 2) + ButtonWidth, 379 + ButtonHeight);
    m_continue.Create(m_hWnd, &rc, _T("Continue"), WS_CHILD | WS_VISIBLE, 0, IDOK);

    // "do not show this window again"
    rc.SetRect(10,422,10+22,426+22);
    m_showSplash.Create(m_hWnd, &rc, _T(""), WS_CHILD | WS_VISIBLE | BS_CHECKBOX, 0, IDC_ShowSplash);
    DrawText_(10+20+6, 426, LeftPane, 30, false, DarkGreen, 14, FW_NORMAL, _T("Do not show this window next time Screenie starts"));

    m_showSplash.SetCheck(true);

    m_showSplash.SetChecked(IDR_CHECKED, _T("BIN"));
    m_showSplash.SetCheckedHover(IDR_CHECKED, _T("BIN"));
    m_showSplash.SetCheckedDown(IDR_CHECKED, _T("BIN"));
    m_showSplash.SetUnchecked(IDR_UNCHECKED, _T("BIN"));
    m_showSplash.SetUncheckedHover(IDR_UNCHECKED, _T("BIN"));
    m_showSplash.SetUncheckedDown(IDR_UNCHECKED, _T("BIN"));
  }

  void DrawText_(int x, int y, int w, int h, bool center, COLORREF textColor, int fontSize, int fontWeight, const tstd::tstring& text, bool wordBreak = false)
  {
    CFont font;
    font.CreateFont(fontSize, 0, 0, 0, fontWeight, 0, 0, 0, 0, 0, 0, 0, 0, _T("Arial"));
    HDC dc = m_offscreen.GetDC();
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, textColor);
    CRect rc(x, y, x + w, y + h);
    HGDIOBJ hOldFont = SelectObject(dc, font);
    DrawText(dc, text.c_str(), text.size(), &rc, (center ? DT_CENTER : 0) | DT_NOCLIP | (wordBreak ? DT_WORDBREAK : 0));
    SelectObject(dc, hOldFont);
  }

  std::tr1::shared_ptr<Gdiplus::Image> LoadPNGResource(UINT id)
  {
    return std::tr1::shared_ptr<Gdiplus::Image>(ImageFromResource(_Module.GetResourceInstance(), MAKEINTRESOURCE(id), _T("PNG")));
  }

  BITMAP m_bmpBackground;

#ifdef CRIPPLED
  CWelcomeWindow<false>* m_next;// when the user closes the nag, this one is the welcome screen.
#endif

  AnimBitmap<32> m_offscreen;

	CHyperLink m_link;
  CFont m_hyperlinkFont;

  CheckboxButton m_showSplash;

  CImageList m_continueImages;
  CBitmapButton m_continue;

  CImageList m_buyImages;
  CBitmapButton m_buy;

  bool m_isDone;

  ScreenshotOptions& m_options;
};



#endif
