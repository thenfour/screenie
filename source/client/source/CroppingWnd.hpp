//
// CroppingWnd.hpp - cropping functionality
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#ifndef SCREENIE_CROPPINGWND_HPP
#define SCREENIE_CROPPINGWND_HPP

class CCroppingWindow
	: public CWindowImpl<CCroppingWindow>
{
public:
	DECLARE_WND_CLASS("ScreenieCropperWnd")

	CCroppingWindow(util::shared_ptr<Gdiplus::Bitmap> bitmap);
	~CCroppingWindow();

	BOOL PreTranslateMessage(MSG* pMsg);

	bool HasSelection() { return m_hasSelection; }
	bool IsSelecting() { return m_selecting; }

	void ClearSelection();

	void BeginSelection(int x, int y);
	void UpdateSelection(int x, int y);
	void EndSelection(int x, int y);

	bool GetSelection(RECT& selectionRect);
	util::shared_ptr<Gdiplus::Bitmap> GetBitmapRect(const RECT& rectToCopy);

	BEGIN_MSG_MAP(CCroppingWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

	LRESULT OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
private:
	bool m_selecting;
	bool m_hasSelection;
	POINT m_selBegin;
	RECT m_selection;

	util::shared_ptr<Gdiplus::Bitmap> m_bitmap;
};

#endif