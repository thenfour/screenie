//
// CropDlg.hpp - modal cropping dialog
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#ifndef _SCREENIE_CROPDLG_H_
#define _SCREENIE_CROPDLG_H_

#include "CroppingWnd.hpp"
#include "ZoomWnd.hpp"
#include "destinationDlg.hpp"
#include "image.hpp"

#include "ScreenshotOptions.hpp"

class CCropDlg :
	public CDialogImpl<CCropDlg>,
	public CDialogResize<CCropDlg>
{
public:
	enum { IDD = IDD_CROPDLG };

	CCropDlg(util::shared_ptr<Gdiplus::Bitmap> bitmap, ScreenshotOptions& options) :
    m_bitmap(bitmap),
    m_didCropping(false),
    m_croppingWnd(bitmap),
    m_zoomWnd(bitmap.get()),
    m_options(options),
    m_hIconSmall(0),
    m_hIcon(0)
  {
  }
	~CCropDlg()
  {
    if(m_hIcon) DeleteObject(m_hIcon);
    if(m_hIconSmall) DeleteObject(m_hIconSmall);
  }

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
		COMMAND_ID_HANDLER(IDC_CONFIGURE, OnConfigure)    

		CHAIN_MSG_MAP(CDialogResize<CCropDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CCropDlg)
		DLGRESIZE_CONTROL(IDC_ZOOM, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_IMAGE, DLSZ_SIZE_X | DLSZ_SIZE_Y)

    DLGRESIZE_CONTROL(IDC_CONFIGURE, DLSZ_MOVE_Y | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_Y | DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
	  // set icons
    if(m_hIcon) DeleteObject(m_hIcon);
    if(m_hIconSmall) DeleteObject(m_hIconSmall);
	  m_hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SCREENIE), 
		  IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	  SetIcon(m_hIcon, TRUE);
	  m_hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SCREENIE), 
		  IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	  SetIcon(m_hIconSmall, FALSE);

    DlgResize_Init(true, true, WS_CLIPCHILDREN);

    // Load window placement settings.
    if(m_options.HaveCroppingPlacement())
    {
      SetWindowPlacement(&m_options.GetCroppingPlacement());
    }

		m_croppingWnd.SubclassWindow(GetDlgItem(IDC_IMAGE));
    BOOL temp;
    m_croppingWnd.OnSize(0,0,0,temp);
    m_croppingWnd.InvalidateRect(0);
		m_zoomWnd.SubclassWindow(GetDlgItem(IDC_ZOOM));
    m_zoomWnd.OnSize(0,0,0,temp);

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
      SetCursor(LoadCursor(0, IDC_CROSS));

      CPoint pos = m_croppingWnd.ScreenToImageCoords(CPoint(cursorPos.x,cursorPos.y));

			SetWindowText(LibCC::Format(TEXT("Crop Screenshot: (%, %)")).ul(pos.x).ul(pos.y).CStr());

			if (m_croppingWnd.IsSelecting())
      {
				m_croppingWnd.UpdateSelection(cursorPos.x, cursorPos.y);
        SyncZoomWindowSelection();
      }

			m_zoomWnd.UpdateBitmapCursorPos(pos);
		}
		else
		{
			SetWindowText(TEXT("Crop Screenshot"));
		}

		return 0;
	}

  void SyncZoomWindowSelection()
  {
    RECT rc;
    if(m_croppingWnd.GetSelection(rc))
    {
      m_zoomWnd.UpdateBitmapSelectionBox(rc);
    }
    else
    {
      m_zoomWnd.RemoveSelectionBox();
    }
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
      SetCursor(LoadCursor(0, IDC_CROSS));
			m_croppingWnd.BeginSelection(cursorPos.x, cursorPos.y);
      SyncZoomWindowSelection();
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
      SetCursor(LoadCursor(0, IDC_CROSS));
			m_croppingWnd.EndSelection(cursorPos.x, cursorPos.y);
      SyncZoomWindowSelection();
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

	LRESULT OnConfigure(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
	  CDestinationDlg dialog(m_options, _T("Ok"));
	  dialog.DoModal();
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

    // save window placement
    WINDOWPLACEMENT wp;
    GetWindowPlacement(&wp);
    m_options.SetCroppingPlacement(wp);

		EndDialog(nVal); 
	}
private:
	bool m_didCropping;
	util::shared_ptr<Gdiplus::Bitmap> m_croppedBitmap;
	util::shared_ptr<Gdiplus::Bitmap> m_bitmap;
	CZoomWindow m_zoomWnd;
	CCroppingWindow m_croppingWnd;
  ScreenshotOptions& m_options;

  HICON m_hIcon;
  HICON m_hIconSmall;
};

#endif
