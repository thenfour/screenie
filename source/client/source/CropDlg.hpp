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
#include <set>

#include "ScreenshotOptions.hpp"


class CCropDlg :
	public CDialogImpl<CCropDlg>,
	public CDialogResize<CCropDlg>,
  public IImageEditWindowEvents
{
	static std::set<CCropDlg*> g_instances;// this is kinda crappy but simple. it's for the hook proc to know where to deal with stuff. Instantiated in MainWindow.cpp
	HACCEL m_hAccelerators;

public:
	enum {
		IDD = IDD_CROPDLG
	};

	CCropDlg(util::shared_ptr<Gdiplus::Bitmap> bitmap, ScreenshotOptions& options) :
    m_bitmap(bitmap),
    m_editWnd(bitmap, this),
    m_zoomWnd(bitmap, 0),
    m_options(options),
    m_hIconSmall(0),
    m_hIcon(0),
		m_didCropping(false),
		m_hHook(0),
		m_hAccelerators(0)
  {
		g_instances.insert(this);
  }

	~CCropDlg()
  {
    if(m_hIcon) DestroyIcon(m_hIcon);
    if(m_hIconSmall) DestroyIcon(m_hIconSmall);
		g_instances.erase(this);
  }

	BEGIN_MSG_MAP(CCropDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)

		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnPaste)
		COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDC_EDITINPAINT, OnEditExternally)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_CONFIGURE, OnConfigure)    
		COMMAND_ID_HANDLER(IDC_CROPPINGTOOL, OnCroppingTool)    
		COMMAND_ID_HANDLER(IDC_HIGHLIGHTTOOL, OnHighlightTool)    
		COMMAND_ID_HANDLER(IDC_RESETIMAGE, OnResetImage)    

    CHAIN_MSG_MAP(CDialogResize<CCropDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CCropDlg)
		DLGRESIZE_CONTROL(IDC_SPLITTER, DLSZ_SIZE_X | DLSZ_SIZE_Y)

		DLGRESIZE_CONTROL(IDC_CROPPINGTOOL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_HIGHLIGHTTOOL, DLSZ_MOVE_X)

		DLGRESIZE_CONTROL(IDC_INFOBOX, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CONTROLS1, DLSZ_MOVE_Y | DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CONTROLS2, DLSZ_MOVE_Y | DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CONTROLS3, DLSZ_MOVE_Y | DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CONTROLS4, DLSZ_MOVE_Y | DLSZ_SIZE_X)

    DLGRESIZE_CONTROL(IDC_EDITINPAINT, DLSZ_MOVE_Y | DLSZ_MOVE_X)
    DLGRESIZE_CONTROL(IDC_SELECTIONSTATIC, DLSZ_SIZE_X)
    DLGRESIZE_CONTROL(IDC_CONFIGURE, DLSZ_MOVE_Y | DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_ETCHEDLINE, DLSZ_MOVE_Y | DLSZ_MOVE_X)

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
		PointF pf2 = pf;
		m_editWnd.ClampToImage(pf2);
		m_infoCursorPos = pf2.Floor();
		UpdateInfoBox();

		m_zoomWnd.CenterOnImage(pf);
    SyncZoomWindowSelection();
  }
  void OnZoomFactorChanged()
  {
  }
  void OnPaste(util::shared_ptr<Gdiplus::Bitmap> n)
	{
		m_zoomWnd.SetBitmap(n);
	}

	virtual void OnToolChanging(ToolBase* tool)
	{
		if(m_editWnd.GetCurrentTool())
			CheckDlgButton(m_editWnd.GetCurrentTool()->GetResourceID(), BST_UNCHECKED);
		CheckDlgButton(tool->GetResourceID(), BST_CHECKED);
	}

  // other crap

	bool GetCroppedScreenshot(util::shared_ptr<Gdiplus::Bitmap>& croppedScreenshot)
	{
		if(!m_didCropping)
    {
			croppedScreenshot = m_bitmap;
			return true;
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

		m_infoZoomFactor = factor;
		UpdateInfoBox();
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

	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& handled)
	{
		return m_editWnd.OnCopy(0, 0, 0, handled);
	}

	LRESULT OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& handled)
	{
		return m_editWnd.OnPaste(0, 0, 0, handled);
	}

	LRESULT OnMouseWheel(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
  {
    handled = TRUE;
    int newFactorIndex = m_zoomFactorIndex + (GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
    AttemptNewFactorIndex(newFactorIndex, true);
    return 0;
  }

	LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		HDROP hDrop = (HDROP)wParam;
		wchar_t buf[1001] = {0};
		DragQueryFile(hDrop, 0, buf, 1000);

		if(!ReplaceImageWithFile(buf))
		{
			MessageBox(L"The file could not be loaded.", L"Screenie", MB_OK);
		}

		return 0;
	}

	static LRESULT CALLBACK GetMsgProc(int code, WPARAM wParam, LPARAM lParam)
	{
		MSG* p = (MSG*)lParam;
		bool noremove = code < 0;
		bool notranslate = false;
		for(std::set<CCropDlg*>::iterator i = g_instances.begin(); i != g_instances.end(); ++ i)
		{
			if((*i)->MsgProc2(p))
			{
				notranslate = true;
			}
		}

		if(noremove && notranslate)
		{
			// huge problem. TranslateAccelerator told us not to continue processing the message, but the hook proc is telling us to not remove the message from the queue.
			// i think it's best to leave it in the queue.
			MulDiv(1,1,1);
		}

		return CallNextHookEx(0, code, wParam, lParam);
	}
	// returns true if the message should be removed from queue.
	bool MsgProc2(MSG* msg)
	{
		return 0 != TranslateAccelerator(m_hWnd, m_hAccelerators, msg);// When TranslateAccelerator returns a nonzero value and the message is translated, the application should not use the TranslateMessage function to process the message again. 
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		DestroyAcceleratorTable(m_hAccelerators);
		m_hAccelerators = 0;
		UnhookWindowsHookEx(m_hHook);
		return 0;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		RECT rcSplitter;
		DragAcceptFiles();

		{
			ACCEL accel[6] = {0};
			accel[0].fVirt = FCONTROL | FVIRTKEY;
			accel[0].key = 'C';
			accel[0].cmd = ID_EDIT_COPY;

			accel[1].fVirt = FCONTROL | FVIRTKEY;
			accel[1].key = VK_INSERT;
			accel[1].cmd = ID_EDIT_COPY;

			accel[2].fVirt = FCONTROL | FVIRTKEY;
			accel[2].key = 'X';
			accel[2].cmd = ID_EDIT_COPY;

			accel[3].fVirt = FSHIFT | FVIRTKEY;
			accel[3].key = VK_DELETE;
			accel[3].cmd = ID_EDIT_COPY;

			accel[4].fVirt = FCONTROL | FVIRTKEY;
			accel[4].key = 'V';
			accel[4].cmd = ID_EDIT_PASTE;

			accel[5].fVirt = FSHIFT | FVIRTKEY;
			accel[5].key = VK_INSERT;
			accel[5].cmd = ID_EDIT_PASTE;

			m_hAccelerators = CreateAcceleratorTable(accel, 6);
			// because there is no PreTranslateMessage with WTL's implementation of DoModal, i need a way to handle accelerators manually.
			m_hHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, _Module.GetResourceInstance(), GetCurrentThreadId());
		}

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
		rcSplitter = rcImage;
    ::DestroyWindow(GetDlgItem(IDC_IMAGE));
    m_editWnd.Create(*this, rcImage, _T(""), WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE, IDC_IMAGE);

    // create zoom window
    ::GetWindowRect(GetDlgItem(IDC_ZOOM), &rcImage);
    ScreenToClient(&rcImage);
		rcSplitter.left = rcImage.left;
    ::DestroyWindow(GetDlgItem(IDC_ZOOM));
    m_zoomWnd.Create(*this, rcImage, _T(""), WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE, IDC_ZOOM);
		m_zoomWnd.SetZoomFactor(1.0f);
		m_zoomWnd.SetShowCursor(true);
		m_zoomWnd.SetEnablePanning(false);
		m_zoomWnd.SetEnableTools(false);

		::DestroyWindow(GetDlgItem(IDC_SPLITTER));
		m_splitter.Create(*this, rcSplitter, L"", WS_CHILD | WS_VISIBLE, /*exstyle*/0, IDC_SPLITTER, /*lparam*/0);
		m_splitter.SetSplitterExtendedStyle(0);
		m_zoomWnd.SetParent(m_splitter);
		m_editWnd.SetParent(m_splitter);
		m_splitter.SetSplitterPanes(m_zoomWnd, m_editWnd, false);

    DlgResize_Init(true, true, WS_CLIPCHILDREN);

    // Load window placement settings.
    if(m_options.HaveCroppingPlacement())
    {
      SetWindowPlacement(&m_options.GetCroppingPlacement());
			m_splitter.SetSplitterPos(m_options.CroppingSplitterPosition, true);
    }
		else
		{
			SetWindowPos(NULL, 0, 0, 800, 600, SWP_NOMOVE);
			CenterWindow(NULL);
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

		m_editWnd.SwitchToTool(IDC_CROPPINGTOOL);

    m_editWnd.CenterImage();

    SyncZoomWindowSelection();

		return 0;
	}

  void SyncZoomWindowSelection()
  {
		if(m_editWnd.HasSelection())
		{
	    CRect rc = m_editWnd.GetSelection();
			m_infoSelection = rc;
			UpdateInfoBox();
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

	bool ReplaceImageWithFile(const std::wstring& fileName)
	{
			Gdiplus::Bitmap* p = Gdiplus::Bitmap::FromFile(fileName.c_str(), FALSE);
			if(!p)
				return false;
			Gdiplus::Status s = p->GetLastStatus();
			if(s != Gdiplus::Ok)
				return false;

			m_bitmap = util::shared_ptr<Gdiplus::Bitmap>(p);
			m_zoomWnd.SetBitmap(m_bitmap);
			m_editWnd.SetBitmap(m_bitmap);

      SetZoomFactor(1.0f, true);

			return true;
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
			ReplaceImageWithFile(tempFileName);
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

	LRESULT OnHighlightTool(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		m_editWnd.SwitchToTool(IDC_HIGHLIGHTTOOL);
	  return 0;
	}

	LRESULT OnCroppingTool(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		m_editWnd.SwitchToTool(IDC_CROPPINGTOOL);
	  return 0;
	}

	LRESULT OnResetImage(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		m_editWnd.ResetImage();
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
		m_options.CroppingSplitterPosition = m_splitter.GetSplitterPos();

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

	HHOOK m_hHook;

	CSplitterWindow m_splitter;

  // zoom stuff
  int m_zoomFactorIndex;
  std::vector<float> m_zoomFactors;

  HICON m_hIcon;
  HICON m_hIconSmall;

	// info stuff
	CPoint m_infoCursorPos;
	float m_infoZoomFactor;
	CRect m_infoSelection;
	std::wstring m_infoText;

	void UpdateInfoBox()
	{
		RgbPixel32 pixel = m_editWnd.GetPixel_(m_infoCursorPos);

		LibCC::Format info = LibCC::Format(
			"Zoom: %^%|"
			"Pos: (%,%)|"
			"Sel: (%,%)-(%,%)|"
			"Sel: % x %|"
			"RGB: #%%% (%, %, %)"
			)
			.f<0>(m_infoZoomFactor*100)
			.i(m_infoCursorPos.x)
			.i(m_infoCursorPos.y)
      .i(m_infoSelection.left)
      .i(m_infoSelection.top)
      .i(m_infoSelection.right)
      .i(m_infoSelection.bottom)
      .i(m_infoSelection.Width())
      .i(m_infoSelection.Height())
			.i<16,2>(R(pixel))
			.i<16,2>(G(pixel))
			.i<16,2>(B(pixel))
			.i(R(pixel))
			.i(G(pixel))
			.i(B(pixel))
			;
		if(m_infoText != info.CStr())
		{
			m_infoText = info.CStr();
			SetDlgItemText(IDC_INFOBOX, m_infoText.c_str());
		}
	}

public:
  LRESULT OnStnClickedZoomCaption(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

#endif

