//
// CroppingWnd.hpp - cropping functionality
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#ifndef SCREENIE_CROPPINGWND_HPP
#define SCREENIE_CROPPINGWND_HPP


#include "animbitmap.h"


class ICroppingWindowEvents
{
public:
  virtual void OnCroppingSelectionChanged() = 0;
  virtual void OnCroppingPositionChanged(int x, int y) = 0;
};


class CCroppingWindow :
  public CWindowImpl<CCroppingWindow>,
  public ICroppingWindowEvents
{
  static const int g_CropBorder = 15;
public:
	//DECLARE_WND_CLASS("ScreenieCropperWnd")
  static ATL::CWndClassInfo& GetWndClassInfo()
  {
    static ATL::CWndClassInfo wc =
	  {
		  { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, StartWindowProc,
		    0, 0, NULL, NULL, NULL, NULL, NULL, _T("ScreenieCropperWnd"), NULL },
		  NULL, NULL, IDC_ARROW, TRUE, 0, _T("")
	  };
    wc.m_lpszCursorID = IDC_CROSS;
    wc.m_bSystemCursor = TRUE;
	  return wc;
  }

  // ICroppingWindowEvents methods
  void OnCroppingSelectionChanged() { }
  void OnCroppingPositionChanged(int x, int y) { }

	CCroppingWindow(util::shared_ptr<Gdiplus::Bitmap> bitmap, ICroppingWindowEvents* pNotify);
	~CCroppingWindow();

	BOOL PreTranslateMessage(MSG* pMsg);

	bool HasSelection() { return m_hasSelection; }

	void ClearSelection();

	void BeginSelection(int x, int y);// in IMAGE coords
	void UpdateSelection(int x, int y);// in IMAGE coords

  // gets the selection in IMAGE coords
	bool GetSelection(RECT& selectionRect);
	util::shared_ptr<Gdiplus::Bitmap> GetBitmapRect(const RECT& rectToCopy);

	BEGIN_MSG_MAP(CCroppingWindow)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	END_MSG_MAP()

	LRESULT OnSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);

  CPoint ClientToImageCoords(CPoint x);
  CPoint ImageToClient(CPoint p);

  LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	bool m_hasSelection;
	
  POINT m_selBegin;
	float m_lastSelectionPointX;// used for "slowing down" selection boxes...
	float m_lastSelectionPointY;
  CPoint m_lastSelectionCursorPos;// client coords
  bool m_selectionEntrancy;// to prevent re-entrancy with mouse handlers.

	RECT m_selectionOrg;// 2005-05-30 carl : changed this from screen coords to picture coords.

  void GetScreenSelection(RECT& rc);

  void DrawSelectionBox(HDC dc);

	util::shared_ptr<Gdiplus::Bitmap> m_bitmap;
  AnimBitmap<32> m_dibOriginal;
  AnimBitmap<32> m_dibStretched;
  AnimBitmap<32> m_dibOffscreen;

  ICroppingWindowEvents* m_notify;
};

#endif

