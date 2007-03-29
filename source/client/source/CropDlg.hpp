//
// CropDlg.hpp - modal cropping dialog
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#ifndef _SCREENIE_CROPDLG_H_
#define _SCREENIE_CROPDLG_H_

#include "ImageEditWnd.hpp"
#include "ZoomWnd.hpp"
#include "BuyDlg.h"
#include "destinationDlg.hpp"
#include "image.hpp"

#include "ScreenshotOptions.hpp"


class CCropDlg :
	public CDialogImpl<CCropDlg>,
	public CDialogResize<CCropDlg>,
  public IImageEditWindowEvents
{
public:
	enum { IDD = IDD_CROPDLG };

	CCropDlg(util::shared_ptr<Gdiplus::Bitmap> bitmap, ScreenshotOptions& options) :
    m_bitmap(bitmap),
    m_editWnd(bitmap, this),
    m_zoomWnd(bitmap.get()),
    m_options(options),
    m_hIconSmall(0),
    m_hIcon(0)
  {
  }

	~CCropDlg()
  {
    if(m_hIcon) DestroyIcon(m_hIcon);
    if(m_hIconSmall) DestroyIcon(m_hIconSmall);
  }

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP(CCropDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
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

		DLGRESIZE_CONTROL(IDC_CONTROLS1, DLSZ_MOVE_Y | DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CONTROLS2, DLSZ_MOVE_Y | DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CONTROLS3, DLSZ_MOVE_Y | DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CONTROLS4, DLSZ_MOVE_Y | DLSZ_SIZE_X)

    DLGRESIZE_CONTROL(IDC_ZOOM_CAPTION, DLSZ_MOVE_X)

    DLGRESIZE_CONTROL(IDC_SELECTALL, DLSZ_MOVE_Y | DLSZ_MOVE_X)
    DLGRESIZE_CONTROL(IDC_SELECTIONSTATIC, DLSZ_SIZE_X)
    DLGRESIZE_CONTROL(IDC_CONFIGURE, DLSZ_MOVE_Y | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_Y | DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

  // IImageEditWindowEvents methods
  void OnSelectionChanged()
  {
    SyncZoomWindowSelection();
  }
  void OnCursorPositionChanged(int x, int y)// image coords
  {
    SetWindowText(LibCC::Format("Crop Screenshot (%,%)").i(x).i(y).CStr());
    m_zoomWnd.UpdateBitmapCursorPos(CPoint(x,y));
    SyncZoomWindowSelection();
  }
  void OnZoomFactorChanged()
  {
  }

  // other crap

	bool GetCroppedScreenshot(util::shared_ptr<Gdiplus::Bitmap>& croppedScreenshot)
	{
		if(!m_didCropping)
    {
			return false;
    }
		croppedScreenshot = m_croppedBitmap;
		return true;
	}

  void AttemptNewFactorIndex(int n, bool updateTrackbar)
  {
    m_zoomFactorIndex = n;
    // clamp
    if(m_zoomFactorIndex < 0) m_zoomFactorIndex = 0;
    if((size_t)m_zoomFactorIndex >= m_zoomFactors.size()) m_zoomFactorIndex = m_zoomFactors.size() - 1;

    float factor = m_zoomFactors[m_zoomFactorIndex];
    m_editWnd.SetZoomFactor(factor);
    m_options.CroppingZoomFactor(factor);

    SetDlgItemText(IDC_ZOOM_CAPTION, LibCC::Format("Zoom: %^%").f<0>(factor*100).CStr());
  }

  void SetZoomFactor(float ideal, bool updateTrackbar)
  {
	size_t i;
    for(i = 0; i < m_zoomFactors.size(); ++ i)
    {
      if(m_zoomFactors[i] > ideal) break;
    }
    int newid = i - 1;
    if(newid < 0) newid = 0;
    if((size_t)newid >= m_zoomFactors.size()) newid = m_zoomFactors.size() - 1;
    AttemptNewFactorIndex(newid, updateTrackbar);
  }

	LRESULT OnMouseWheel(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
  {
    handled = TRUE;
    int newFactorIndex = m_zoomFactorIndex + (GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
    AttemptNewFactorIndex(newFactorIndex, true);
    return 0;
  }
  
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
    // populate the list of zoom factors.
    bool bHitOne = false;
    m_zoomFactors.push_back(0.1f);
    m_zoomFactors.push_back(0.15f);
    m_zoomFactors.push_back(0.25f);
    m_zoomFactors.push_back(0.40f);
    m_zoomFactors.push_back(0.666667f);
    m_zoomFactors.push_back(1.00f);
    m_zoomFactors.push_back(1.5f);
    m_zoomFactors.push_back(2.0f);
    m_zoomFactors.push_back(3.0f);
    m_zoomFactors.push_back(4.0f);
    m_zoomFactors.push_back(6.0f);
    m_zoomFactors.push_back(8.0f);
    m_zoomFactors.push_back(10.0f);
    m_zoomFactors.push_back(12.5f);
    m_zoomFactors.push_back(15.0f);
    m_zoomFactors.push_back(18.0f);
    m_zoomFactors.push_back(23.0f);
    m_zoomFactors.push_back(30.0f);

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
    m_editWnd.Create(*this, rcImage, _T(""), WS_CHILD | WS_VISIBLE, 0, IDC_IMAGE);

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

    // set up image edit window
    BOOL temp;
    m_editWnd.SetZoomFactor(1.0f);// just temporary, to give it *something*
    m_editWnd.OnSize(0,0,0,temp);

    // determine the best zoom factor.
    CRect rcClient;
    m_editWnd.GetClientRect(&rcClient);
    float ideal = (float)rcClient.Width() / m_bitmap->GetWidth();
    if(ideal > 1.0f) ideal = 1.0f;
    SetZoomFactor(ideal, true);

    m_editWnd.CenterImage();

    SyncZoomWindowSelection();
		return 0;
	}

  void SyncZoomWindowSelection()
  {
    CRect rc;
    if(m_editWnd.GetVirtualSelection(rc))
    {
      SetDlgItemText(IDC_SELECTIONSTATIC,
        LibCC::Format("(%,%)-(%,%); % x %")
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
		CloseDialog(IDCANCEL);
    return 0;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(IDOK);
		return 0;
	}

  LRESULT OnSelectAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    m_editWnd.ClearSelection();
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(IDCANCEL);
		return 0;
	}

	LRESULT OnConfigure(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
	  CDestinationDlg dialog(m_options, _T("OK"));
	  dialog.DoModal();
	  return 0;
	}

	void CloseDialog(int nVal)
	{
		if (nVal == IDOK)
		{
			RECT selectionRect = { 0 };
			if (m_editWnd.GetVirtualSelection(selectionRect))
			{
				m_croppedBitmap = m_editWnd.GetBitmapRect(selectionRect);
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
	CImageEditWindow m_editWnd;
  ScreenshotOptions& m_options;

  // zoom stuff
  int m_zoomFactorIndex;
  std::vector<float> m_zoomFactors;

  HICON m_hIcon;
  HICON m_hIconSmall;
public:
  LRESULT OnStnClickedZoomCaption(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

#endif

