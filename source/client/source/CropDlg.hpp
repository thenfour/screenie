//
// CropDlg.hpp - modal cropping dialog
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#ifndef _SCREENIE_CROPDLG_H_
#define _SCREENIE_CROPDLG_H_

#include "CroppingWnd.hpp"
#include "ZoomWnd.hpp"

class CCropDlg :
	public CDialogImpl<CCropDlg>,
	public CDialogResize<CCropDlg>
{
public:
	enum { IDD = IDD_CROPDLG };

	CCropDlg(util::shared_ptr<Gdiplus::Bitmap> bitmap)
		: m_bitmap(bitmap), m_didCropping(false), m_croppingWnd(bitmap), m_zoomWnd(bitmap) { }
	~CCropDlg() { }

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	bool GetCroppedScreenshot(util::shared_ptr<Gdiplus::Bitmap>& croppedScreenshot)
	{
		if (!m_didCropping)
			return false;

		croppedScreenshot = m_croppedBitmap;

		return true;
	}

	BEGIN_MSG_MAP(CCropDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)

		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)

		CHAIN_MSG_MAP(CDialogResize<CCropDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CCropDlg)
		DLGRESIZE_CONTROL(IDC_ZOOM, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_IMAGE, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_Y | DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		DlgResize_Init(true, true, WS_CLIPCHILDREN);

		m_croppingWnd.SubclassWindow(GetDlgItem(IDC_IMAGE));
		m_zoomWnd.SubclassWindow(GetDlgItem(IDC_ZOOM));

		return 0;
	}

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		POINT cursorPos;
		GetCursorPos(&cursorPos);

		RECT imageClientRect = { 0 };
		m_croppingWnd.GetClientRect(&imageClientRect);
		m_croppingWnd.ScreenToClient(&cursorPos);

		int scaleWidth = imageClientRect.right - imageClientRect.left;
		int scaleHeight = imageClientRect.bottom - imageClientRect.top;

		if (::PtInRect(&imageClientRect, cursorPos))
		{
			int x = int(cursorPos.x * (float(m_bitmap->GetWidth()) / scaleWidth));
			int y = int(cursorPos.y * (float(m_bitmap->GetHeight()) / scaleHeight));

			SetWindowText(LibCC::Format(TEXT("Crop Screenshot: (%, %)")).ul(x).ul(y).CStr());

			if (m_croppingWnd.IsSelecting())
				m_croppingWnd.UpdateSelection(cursorPos.x, cursorPos.y);

			POINT bitmapCursorPos = { x, y };
			m_zoomWnd.UpdateBitmapCursorPos(bitmapCursorPos);
		}
		else
		{
			SetWindowText(TEXT("Crop Screenshot"));
		}

		return 0;
	}

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 0;
	}


	LRESULT OnLButtonDown(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		POINT cursorPos = { 0 };
		GetCursorPos(&cursorPos);

		RECT imageClientRect = { 0 };
		m_croppingWnd.GetClientRect(&imageClientRect);
		m_croppingWnd.ScreenToClient(&cursorPos);

		if (PtInRect(&imageClientRect, cursorPos))
		{
			m_croppingWnd.BeginSelection(cursorPos.x, cursorPos.y);
		}

		return 0;
	}

	LRESULT OnLButtonUp(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		POINT cursorPos = { 0 };
		GetCursorPos(&cursorPos);

		RECT imageClientRect = { 0 };
		m_croppingWnd.GetClientRect(&imageClientRect);
		m_croppingWnd.ScreenToClient(&cursorPos);

		if (PtInRect(&imageClientRect, cursorPos))
		{
			m_croppingWnd.EndSelection(cursorPos.x, cursorPos.y);
		}

		return 0;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(IDOK);

		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(IDCANCEL);

		return 0;
	}

	void CloseDialog(int nVal)
	{
		if (nVal == IDOK)
		{
			RECT selectionRect = { 0 };
			if (m_croppingWnd.GetSelection(selectionRect))
			{
				m_croppedBitmap = m_croppingWnd.GetBitmapRect(selectionRect);
				m_didCropping = true;
			}
		}

		EndDialog(nVal); 
	}
private:
	bool m_didCropping;
	util::shared_ptr<Gdiplus::Bitmap> m_croppedBitmap;
	util::shared_ptr<Gdiplus::Bitmap> m_bitmap;
	CZoomWindow m_zoomWnd;
	CCroppingWindow m_croppingWnd;
};

#endif