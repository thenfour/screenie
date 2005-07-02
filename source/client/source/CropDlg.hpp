//
// CropDlg.hpp - modal cropping dialog
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#ifndef _SCREENIE_CROPDLG_H_
#define _SCREENIE_CROPDLG_H_

#include "ImageEditWnd.hpp"
#include "ZoomWnd.hpp"
#include "destinationDlg.hpp"
#include "image.hpp"

#include "ScreenshotOptions.hpp"


class CCropDlg :
	public CDialogImpl<CCropDlg>,
	public CDialogResize<CCropDlg>,
  public IImageEditWindowEvents,
  public IZoomWindowEvents
{
public:
	enum { IDD = IDD_CROPDLG };

	CCropDlg(util::shared_ptr<Gdiplus::Bitmap> bitmap, ScreenshotOptions& options) :
    m_bitmap(bitmap),
    m_didCropping(false),
    m_croppingWnd(bitmap, this),
    m_zoomWnd(bitmap.get(), this),
    m_options(options),
    m_hIconSmall(0),
    m_hIcon(0),
    m_zoomFactorIndex(0)
  {
    for(float f = 0.05f; f < 30.0f; f *= 1.10)
    {
      m_zoomFactors.push_back(f);
    }
  }

	~CCropDlg()
  {
    if(m_hIcon) DestroyIcon(m_hIcon);
    if(m_hIconSmall) DestroyIcon(m_hIconSmall);
  }

  // IZoomWindowEvents methods
  void OnZoomScaleFactorChanged(int factor)
  {
    SetDlgItemText(IDC_ZOOM_CAPTION, LibCC::Format("%x zoom view:").i(factor).CStr());
    m_options.CroppingZoomFactor(factor);
    ::SendMessage(GetDlgItem(IDC_ZOOMFACTOR), TBM_SETPOS, TRUE, m_options.CroppingZoomFactor());
  }

  // IImageEditWindowEvents methods
  void OnCroppingSelectionChanged()
  {
    SyncZoomWindowSelection();
  }
  void OnCursorPositionChanged(int x, int y)// image coords
  {
    SetWindowText(LibCC::Format("Crop Screenshot (%,%)").i(x).i(y).CStr());
    m_zoomWnd.UpdateBitmapCursorPos(CPoint(x,y));
    SyncZoomWindowSelection();
  }
  void OnZoomScaleFactorChanged2(int factor)
  {
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
    MESSAGE_HANDLER(WM_HSCROLL, OnZoomFactorChanged);
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)

		COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDC_SELECTALL, OnSelectAll)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_CONFIGURE, OnConfigure)    

		CHAIN_MSG_MAP(CDialogResize<CCropDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CCropDlg)
		DLGRESIZE_CONTROL(IDC_ZOOM, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_IMAGE, DLSZ_SIZE_X | DLSZ_SIZE_Y)

    DLGRESIZE_CONTROL(IDC_SELECTALL, DLSZ_MOVE_Y | DLSZ_MOVE_X)
    DLGRESIZE_CONTROL(IDC_SELECTIONSTATIC, DLSZ_MOVE_Y | DLSZ_MOVE_X)
    DLGRESIZE_CONTROL(IDC_CONFIGURE, DLSZ_MOVE_Y | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_Y | DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

  void AttemptNewFactorIndex(int n)
  {
    m_zoomFactorIndex = n;
    // clamp
    if(m_zoomFactorIndex < 0) m_zoomFactorIndex = 0;
    if(m_zoomFactorIndex >= m_zoomFactors.size()) m_zoomFactorIndex = m_zoomFactors.size() - 1;
    // notify
    float factor = m_zoomFactors[m_zoomFactorIndex];
    //m_zoomWnd.SetFactor(factor);
    m_croppingWnd.SetZoomFactor(factor);
    //options
    m_options.CroppingZoomFactor(factor);
  }

	LRESULT OnMouseWheel(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
  {
    handled = TRUE;
    int newFactorIndex = m_zoomFactorIndex + (GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
    AttemptNewFactorIndex(newFactorIndex);
    return 0;
  }

	LRESULT OnZoomFactorChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
  {
    /*
      A trackbar notifies its parent window of user actions by sending the parent a WM_HSCROLL or WM_VSCROLL message.
      A trackbar with the TBS_HORZ style sends WM_HSCROLL messages.
      A trackbar with the TBS_VERT style sends WM_VSCROLL messages.
      
      The low-order word of the wParam parameter of WM_HSCROLL or WM_VSCROLL contains the notification code.
      For the TB_THUMBPOSITION and TB_THUMBTRACK notifications, the high-order word of the wParam parameter specifies the position of the slider.
      For all other notifications, the high-order word is zero;
      send the TBM_GETPOS message to determine the slider position.
      The lParam parameter is the handle to the trackbar. 
    */
    if(((HWND)lParam) == GetDlgItem(IDC_ZOOMFACTOR).m_hWnd)
    {
      int newFactorIndex = ::SendMessage(GetDlgItem(IDC_ZOOMFACTOR), TBM_GETPOS, 0, 0);
      AttemptNewFactorIndex(newFactorIndex);
    }
    return 0;
  }
  
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
	  // set icons
    if(m_hIcon) DestroyIcon(m_hIcon);
    if(m_hIconSmall) DestroyIcon(m_hIconSmall);
	  m_hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SCREENIE), 
		  IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	  SetIcon(m_hIcon, TRUE);
	  m_hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SCREENIE), 
		  IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	  SetIcon(m_hIconSmall, FALSE);

    // create the image window.
    RECT rcImage;
    ::GetWindowRect(GetDlgItem(IDC_IMAGE), &rcImage);
    ScreenToClient(&rcImage);
    ::DestroyWindow(GetDlgItem(IDC_IMAGE));
    m_croppingWnd.Create(*this, rcImage, _T(""), WS_CHILD | WS_VISIBLE, 0, IDC_IMAGE);

    // create zoom window
    rcImage;
    ::GetWindowRect(GetDlgItem(IDC_ZOOM), &rcImage);
    ScreenToClient(&rcImage);
    ::DestroyWindow(GetDlgItem(IDC_ZOOM));
    m_zoomWnd.Create(*this, rcImage, _T(""), WS_CHILD | WS_VISIBLE, 0, IDC_ZOOM);

    DlgResize_Init(true, true, WS_CLIPCHILDREN);

    // Load window placement settings.
    if(m_options.HaveCroppingPlacement())
    {
      SetWindowPlacement(&m_options.GetCroppingPlacement());
    }

    // set up zoom slider
    HWND hSlider = GetDlgItem(IDC_ZOOMFACTOR);
    SendMessage(hSlider, TBM_SETRANGE, FALSE, MAKELONG (1, m_zoomFactors.size()));
    SendMessage(hSlider, TBM_SETPOS, TRUE, m_options.CroppingZoomFactor());

    float price = m_options.CroppingZoomFactor();

    // find the closest zoom factor index without going over.
    for(int i = 0; i < m_zoomFactors.size(); ++ i)
    {
      if(m_zoomFactors[i] > price) break;
    }
    m_zoomFactorIndex = i - 1;
    if(m_zoomFactorIndex < 0) m_zoomFactorIndex = 0;
    if(m_zoomFactorIndex >= m_zoomFactors.size()) m_zoomFactorIndex = m_zoomFactors.size() - 1;

    OnZoomScaleFactorChanged(m_options.CroppingZoomFactor());
    m_zoomWnd.SetFactor(1);

    BOOL temp;

    m_croppingWnd.SetZoomFactor(m_options.CroppingZoomFactor());
    m_croppingWnd.OnSize(0,0,0,temp);
    m_croppingWnd.InvalidateRect(0);

    SetForegroundWindow(*this);

    SyncZoomWindowSelection();

		return 0;
	}

  void SyncZoomWindowSelection()
  {
    CRect rc;
    if(m_croppingWnd.GetVirtualSelection(rc))
    {
      SetDlgItemText(IDC_SELECTIONSTATIC,
        LibCC::Format("(%,%)-(%,%)|% x %")
          .i(rc.left)
          .i(rc.top)
          .i(rc.right)
          .i(rc.bottom)
          .i(rc.Width())
          .i(rc.Height())
          .CStr()
        );
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

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(IDOK);

		return 0;
	}

  LRESULT OnSelectAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    m_croppingWnd.ClearSelection();
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
			if (m_croppingWnd.GetVirtualSelection(selectionRect))
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
	CImageEditWindow m_croppingWnd;
  ScreenshotOptions& m_options;

  HICON m_hIcon;
  HICON m_hIconSmall;

  int m_zoomFactorIndex;
  std::vector<float> m_zoomFactors;
};

#endif

