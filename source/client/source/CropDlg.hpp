//
// CropDlg.hpp - modal cropping dialog
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#ifndef _SCREENIE_CROPDLG_H_
#define _SCREENIE_CROPDLG_H_

#include "ImageEditWnd.hpp"
#include "destinationDlg.hpp"
#include "image.hpp"
#include "codec.hpp"

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
    m_zoomWnd(bitmap, 0),
    m_options(options),
    m_hIconSmall(0),
    m_hIcon(0),
		m_didCropping(false)
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
    COMMAND_ID_HANDLER(IDC_EDITINPAINT, OnEditExternally)
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

    DLGRESIZE_CONTROL(IDC_EDITINPAINT, DLSZ_MOVE_Y | DLSZ_MOVE_X)
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
  void OnCursorPositionChanged(const PointF& pf)// image coords
  {
		CPoint p = pf.Round();
    SetWindowText(LibCC::Format("Crop Screenshot (%,%)").i(p.x).i(p.y).CStr());
		m_zoomWnd.CenterOnImage(pf);
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
    m_zoomFactors.push_back(0.25f);
    m_zoomFactors.push_back(0.50f);
    m_zoomFactors.push_back(1.00f);
    m_zoomFactors.push_back(2.0f);
    m_zoomFactors.push_back(3.0f);
    m_zoomFactors.push_back(4.0f);
    m_zoomFactors.push_back(6.0f);
    m_zoomFactors.push_back(8.0f);
    m_zoomFactors.push_back(12.5f);
    m_zoomFactors.push_back(18.0f);
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
		m_zoomWnd.SetZoomFactor(1.0f);
		m_zoomWnd.SetShowCursor(true);
		m_zoomWnd.SetEnablePanning(false);
		m_zoomWnd.SetEnableTools(false);

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
		if(m_editWnd.HasSelection())
		{
	    CRect rc = m_editWnd.GetSelection();
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
			m_zoomWnd.SetSelection(rc);
    }
    else
    {
      m_zoomWnd.ClearSelection();
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

  LRESULT OnEditExternally(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// save m_bitmap
		tstd::tstring tempFileName = GetUniqueTemporaryFilename();
		// save a lossless PNG file to a memory stream
		ImageCodecsEnum codecs;
		Gdiplus::ImageCodecInfo* codecInfo = codecs.GetCodecByMimeType(_T("image/png"));
		Gdiplus::Status status = m_bitmap->Save(tempFileName.c_str(), &codecInfo->Clsid);
		if(status != Gdiplus::Ok)
		{
			MessageBox(L"There was an error saving the temp file.");
			return 0;
		}

		// launch mspaint
		tstd::tstring appname;
		GetSpecialFolderPath(appname, CSIDL_SYSTEM);
		LibCC::PathAppendX(appname, L"mspaint.exe");
		std::wstring cmdLine = LibCC::Format(L"% %").qs(appname).qs(tempFileName).Str();

		LibCC::Blob<wchar_t> blobCommandLine(cmdLine.size());
		wcscpy(blobCommandLine.GetBuffer(), cmdLine.c_str());
		STARTUPINFO si = {0};
		GetStartupInfo(&si);
		PROCESS_INFORMATION pi = {0};
		if(FALSE == CreateProcess(appname.c_str(), blobCommandLine.GetBuffer(), 0, 0, FALSE, 0, 0, 0, &si, &pi))
		{
			DeleteFile(tempFileName.c_str());
			MessageBox(L"There was an error launching mspaint.exe.");
			return 0;
		}

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		if(IDOK == MessageBox(L"Please click OK when you are done editing the image.", L"Screenie", MB_OKCANCEL | MB_ICONINFORMATION))
		{
			// reload m_bitmap
			m_bitmap = util::shared_ptr<Gdiplus::Bitmap>(Gdiplus::Bitmap::FromFile(tempFileName.c_str(), FALSE));
			m_zoomWnd.SetBitmap(m_bitmap);
			m_editWnd.SetBitmap(m_bitmap);
		}
		DeleteFile(tempFileName.c_str());
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
			if(m_editWnd.HasSelection())
			{
				CRect selectionRect = m_editWnd.GetSelection();
				m_croppedBitmap = m_editWnd.GetBitmapRect(selectionRect);
				m_didCropping = true;
			}
		}

    // save window placement
    WINDOWPLACEMENT wp;
    GetWindowPlacement(&wp);
    m_options.SetCroppingPlacement(wp);

		m_options.SaveSettings();

		EndDialog(nVal); 
	}
private:
	bool m_didCropping;
	util::shared_ptr<Gdiplus::Bitmap> m_croppedBitmap;
  util::shared_ptr<Gdiplus::Bitmap> m_bitmap;
	CImageEditWindow m_zoomWnd;
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

