//
// CroppingWnd.hpp - cropping functionality
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#ifndef SCREENIE_CROPPINGWND_HPP
#define SCREENIE_CROPPINGWND_HPP


#include "animbitmap.h"


class CCroppingWindow
	: public CWindowImpl<CCroppingWindow>
{
  static const int g_CropBorder = 15;
public:
	DECLARE_WND_CLASS("ScreenieCropperWnd")

	CCroppingWindow(util::shared_ptr<Gdiplus::Bitmap> bitmap);
	~CCroppingWindow();

	BOOL PreTranslateMessage(MSG* pMsg);

	bool HasSelection() { return m_hasSelection; }
	bool IsSelecting() { return m_selecting; }

	void ClearSelection();

	void BeginSelection(int x, int y);// in SCREEN coords
	void UpdateSelection(int x, int y);// in SCREEN coords
	void EndSelection(int x, int y);// in SCREEN coords

  // gets the selection in IMAGE coords
	bool GetSelection(RECT& selectionRect);
	util::shared_ptr<Gdiplus::Bitmap> GetBitmapRect(const RECT& rectToCopy);

	BEGIN_MSG_MAP(CCroppingWindow)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

	LRESULT OnSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);

  CPoint ScreenToImageCoords(CPoint x);

private:
	bool m_selecting;
	bool m_hasSelection;
	POINT m_selBegin;
	RECT m_selectionOrg;// 2005-05-30 carl : changed this from screen coords to picture coords.

  void GetScreenSelection(RECT& rc);

  void DrawSelectionBox(HDC dc);

	util::shared_ptr<Gdiplus::Bitmap> m_bitmap;
  AnimBitmap<32> m_dibOriginal;
  AnimBitmap<32> m_dibStretched;
  AnimBitmap<32> m_dibOffscreen;
};

#endif

