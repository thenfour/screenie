//
// CropDlg.hpp - modal cropping dialog
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//

#ifndef _SCREENIE_CROPDLG_H_
#define _SCREENIE_CROPDLG_H_

#include "ImageEditWnd.hpp"
#include "destinationDlg.hpp"
#include "image.hpp"
#include "codec.hpp"
#include <set>

#include "ScreenshotOptions.hpp"


inline void RGBtoHSV( float r, float g, float b, float *h, float *s, float *v )
{
	float min_, max_, delta;
	min_ = min( min(r, g), b );
	max_ = max( max(r, g), b );
	*v = max_;				// v
	delta = max_ - min_;
	if( max_ != 0 )
		*s = delta / max_;		// s
	else {
		// r = g = b = 0		// s = 0, v is undefined
		*s = 0;
		*h = LibCC::SinglePrecisionFloat::BuildQNaN().m_BasicVal;
		return;
	}
	if( r == max_ )
		*h = ( g - b ) / delta;		// between yellow & magenta
	else if( g == max_ )
		*h = 2 + ( b - r ) / delta;	// between cyan & yellow
	else
		*h = 4 + ( r - g ) / delta;	// between magenta & cyan
	*h *= 60;				// degrees
	while( *h < 0 )
		*h += 360;
}


struct CPNGButton :
	CButton
{
	CImageList imgList;
	Gdiplus::BitmapPtr myImage;

	~CPNGButton()
	{
		imgList.Destroy();
	}

	void AddImage(Gdiplus::BitmapPtr masterbmp, int yoffset, int xoffset, int buttonWidth, int buttonHeight)
	{
		HICON hi;
		Gdiplus::Bitmap* temp = masterbmp->Clone(xoffset, yoffset, buttonWidth, buttonHeight, PixelFormat32bppARGB);// grab the portion of the bitmap
		temp->GetHICON(&hi);// convert to icon
		delete temp;
		imgList.AddIcon(hi);// store it.
		DestroyIcon(hi);
	}

	void Init(HWND hwnd, UINT ctrlID, Gdiplus::BitmapPtr masterbmp, int yoffset, int buttonWidth, int buttonHeight)
	{
		// change the style to support icons
		Attach(::GetDlgItem(hwnd, ctrlID));
		SetWindowLong(GWL_STYLE, GetWindowLong(GWL_STYLE) | BS_ICON);

		// chop up the image into 6 icons
		imgList.Create(buttonWidth, buttonHeight, ILC_COLOR32, 6, 1);
		int xoffset = 0;
		AddImage(masterbmp, yoffset, xoffset, buttonWidth, buttonHeight);
		if((xoffset += buttonWidth) < masterbmp->GetWidth())
			AddImage(masterbmp, yoffset, xoffset, buttonWidth, buttonHeight);
		if((xoffset += buttonWidth) < masterbmp->GetWidth())
			AddImage(masterbmp, yoffset, xoffset, buttonWidth, buttonHeight);
		if((xoffset += buttonWidth) < masterbmp->GetWidth())
			AddImage(masterbmp, yoffset, xoffset, buttonWidth, buttonHeight);
		if((xoffset += buttonWidth) < masterbmp->GetWidth())
			AddImage(masterbmp, yoffset, xoffset, buttonWidth, buttonHeight);
		if((xoffset += buttonWidth) < masterbmp->GetWidth())
			AddImage(masterbmp, yoffset, xoffset, buttonWidth, buttonHeight);

		BUTTON_IMAGELIST bi = {0};
		bi.himl = imgList.m_hImageList;
		SetImageList(&bi);
	}

};

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

	enum
	{
		ID_TOGGLEINFOUPDATE = WM_APP + 1
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
		m_hAccelerators(0),
		m_infoUpdate(true)
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
		MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST,WM_MOUSELAST, OnMouse)
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
		COMMAND_ID_HANDLER(ID_TOGGLEINFOUPDATE, OnToggleInfoUpdate) 

		COMMAND_ID_HANDLER(IDC_SCREENIEHELP, OnHelp)    
		COMMAND_ID_HANDLER(IDC_ZOOMIN, OnZoomIn)    
		COMMAND_ID_HANDLER(IDC_ZOOMOUT, OnZoomOut)
		COMMAND_ID_HANDLER(IDC_NEXTDESTINATION, OnNextDestination)
		COMMAND_ID_HANDLER(IDC_PREVDESTINATION, OnPrevDestination)

    CHAIN_MSG_MAP(CDialogResize<CCropDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CCropDlg)
		DLGRESIZE_CONTROL(IDC_SPLITTER, DLSZ_SIZE_X | DLSZ_SIZE_Y)

		// toolbar stuff
		DLGRESIZE_CONTROL(IDC_CROPPINGTOOL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_HIGHLIGHTTOOL, DLSZ_MOVE_X)
    DLGRESIZE_CONTROL(IDC_EDITINPAINT, DLSZ_MOVE_X)
    DLGRESIZE_CONTROL(IDC_CONFIGURE, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_RESETIMAGE, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ZOOMOUT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ZOOMIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_SCREENIEHELP, DLSZ_MOVE_X)

		DLGRESIZE_CONTROL(IDC_INFOBOX, DLSZ_MOVE_X | DLSZ_SIZE_Y)

		DLGRESIZE_CONTROL(IDC_NEXTDESTINATION, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_PREVDESTINATION, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CURRENTDESTINATION, DLSZ_MOVE_Y | DLSZ_SIZE_X)

		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_Y | DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	LRESULT OnMouse(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		if(toolTip.IsWindow())
			toolTip.RelayEvent((LPMSG)GetCurrentMessage());
		return FALSE;
	}

  // IImageEditWindowEvents methods
  void OnSelectionChanged()
  {
  }
  void OnCursorPositionChanged(const PointF& pf)// image coords
  {
		PointF pf2 = pf;
		m_editWnd.ClampToImage(pf2);
		m_infoCursorPos = pf2.Floor();
		UpdateInfoBox();
  }
  void OnZoomFactorChanged()
  {
  }
  void OnPaste(util::shared_ptr<Gdiplus::Bitmap> n)
	{
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
			ACCEL accel[11] = {0};
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

			accel[6].fVirt = FVIRTKEY;
			accel[6].key = VK_F4;
			accel[6].cmd = ID_TOGGLEINFOUPDATE;

			accel[7].fVirt = FVIRTKEY;
			accel[7].key = VK_ADD;
			accel[7].cmd = IDC_NEXTDESTINATION;

			accel[8].fVirt = FVIRTKEY;
			accel[8].key = VK_SUBTRACT;
			accel[8].cmd = IDC_PREVDESTINATION;

			accel[9].fVirt = FVIRTKEY;
			accel[9].key = VK_F6;
			accel[9].cmd = IDC_ZOOMOUT;

			accel[10].fVirt = FVIRTKEY;
			accel[10].key = VK_F7;
			accel[10].cmd = IDC_ZOOMIN;

			m_hAccelerators = CreateAcceleratorTable(accel, LibCC::SizeofStaticArray(accel));
			// because there is no PreTranslateMessage with WTL's implementation of DoModal, i need a way to handle accelerators manually.
			m_hHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, _Module.GetResourceInstance(), GetCurrentThreadId());
		}

    // populate the list of zoom factors.
    bool bHitOne = false;
    m_zoomFactors.push_back(0.1f);
    m_zoomFactors.push_back(0.1677f);
    m_zoomFactors.push_back(0.25f);
    m_zoomFactors.push_back(0.333f);
    m_zoomFactors.push_back(0.50f);
    m_zoomFactors.push_back(0.75f);
    m_zoomFactors.push_back(1.00f);
    m_zoomFactors.push_back(1.50f);
    m_zoomFactors.push_back(2.0f);
    m_zoomFactors.push_back(3.0f);
    m_zoomFactors.push_back(4.0f);
    m_zoomFactors.push_back(6.0f);
    m_zoomFactors.push_back(8.0f);
    m_zoomFactors.push_back(12.5f);
    m_zoomFactors.push_back(18.0f);
    m_zoomFactors.push_back(24.0f);
    //m_zoomFactors.push_back(30.0f);

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

		m_zoomWnd.SetMaster(&m_editWnd);

		::DestroyWindow(GetDlgItem(IDC_SPLITTER));
		m_splitter.Create(*this, rcSplitter, L"", WS_CHILD | WS_VISIBLE, /*exstyle*/0, IDC_SPLITTER, /*lparam*/0);
		m_splitter.SetSplitterExtendedStyle(0);
		m_zoomWnd.SetParent(m_splitter);
		m_editWnd.SetParent(m_splitter);
		m_splitter.SetSplitterPanes(m_zoomWnd, m_editWnd, false);

		// set up toolbar
		const UINT tbButtonCtrls[] = { IDC_RESETIMAGE, IDC_EDITINPAINT, IDC_CROPPINGTOOL, IDC_HIGHLIGHTTOOL, IDC_ZOOMOUT, IDC_ZOOMIN, IDC_SCREENIEHELP, IDC_CONFIGURE };

		MemStream backgroundStream(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_TOOLBAR), L"PNG");
		Gdiplus::BitmapPtr masterBitmap(Gdiplus::Bitmap::FromStream(backgroundStream.m_pStream));

		for(int i = 0; i < ToolbarButtonCount; ++ i)
		{
			tbButons[i].Init(this->m_hWnd, tbButtonCtrls[i], masterBitmap, i * ToolbarButtonImageSize, ToolbarButtonImageSize, ToolbarButtonImageSize);
		}

		// tooltips
		toolTip.Create( m_hWnd, NULL, NULL, TTS_NOPREFIX );
		toolTip.SetMaxTipWidth( 0 );
		toolTip.Activate( TRUE );
		toolTip.SetDelayTime(TTDT_INITIAL, 300);
		toolTip.SetDelayTime(TTDT_AUTOPOP, 3000);
		toolTip.SetDelayTime(TTDT_RESHOW, 60);

		toolTip.AddTool(&CreateToolInfo(IDC_RESETIMAGE, L"Reset Image"));
		toolTip.AddTool(&CreateToolInfo(IDC_EDITINPAINT, L"Edit the image in an external editor"));
		toolTip.AddTool(&CreateToolInfo(IDC_CROPPINGTOOL, L"Crop tool"));
		toolTip.AddTool(&CreateToolInfo(IDC_HIGHLIGHTTOOL, L"Highlight tool"));
		toolTip.AddTool(&CreateToolInfo(IDC_ZOOMOUT, L"Zoom out further"));
		toolTip.AddTool(&CreateToolInfo(IDC_ZOOMIN, L"Zoom in closer"));
		toolTip.AddTool(&CreateToolInfo(IDC_SCREENIEHELP, L"Open Screenie help"));
		toolTip.AddTool(&CreateToolInfo(IDC_CONFIGURE, L"Open the options dialog"));

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

		UpdateCurrentDestination();
		return 0;
	}

	TOOLINFO CreateToolInfo(UINT ctrlID, const std::wstring& text)
	{
		TOOLINFO ret = {0};
		ret.cbSize = sizeof(ret);
		ret.uFlags = TTF_SUBCLASS | TTF_IDISHWND | TTF_TRANSPARENT;
		ret.hwnd = m_hWnd;
		ret.uId = (UINT_PTR)::GetDlgItem(m_hWnd, ctrlID);
		//ret.rect = ignored.
		ret.lpszText = const_cast<PWSTR>(text.c_str());
		return ret;
	};



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
			//m_zoomWnd.SetBitmap(m_bitmap);
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
		appname = LibCC::PathAppendX(appname, L"mspaint.exe");
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

	LRESULT OnHelp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		::ShellExecute(0, _T("open"), _T("http://screenie.net/help"), 0, 0, SW_SHOWNORMAL);
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
		UpdateCurrentDestination();
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

	bool m_infoUpdate;

	LRESULT OnToggleInfoUpdate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		m_infoUpdate = !m_infoUpdate;
		UpdateInfoBox();
	  return 0;
	}

	LRESULT OnZoomIn(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    AttemptNewFactorIndex(m_zoomFactorIndex + 1, true);
	  return 0;
	}

	LRESULT OnZoomOut(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    AttemptNewFactorIndex(m_zoomFactorIndex - 1, true);
	  return 0;
	}

	void UpdateCurrentDestination()
	{
		int found = 0;
		int total = (int)m_options.GetNumDestinations();
		std::wstring nameOfSelected = L"(none)";
		for(int i = 0; i < total; i ++)
		{
			ScreenshotDestination& dest(m_options.GetDestination(i));
			if(dest.enabled)
			{
				nameOfSelected = LibCC::Format(L"[%/%] % (%)")(i+1)(total).s(dest.general.name).s(dest.GetGeneralInfo()).Str();
				found ++;
			}
		}

		if(found > 1)
		{
			nameOfSelected = LibCC::Format(L"% destinations selected.")(found).Str();
		}

		SetDlgItemText(IDC_CURRENTDESTINATION, nameOfSelected.c_str());
	}

	void SelectNewDestination(int delta)
	{
		int total = (int)m_options.GetNumDestinations();

		if(total == 0)
			return;

		// figure out the current selected index.
		int sel = -1;
		for(size_t i = 0; i < total; i ++)
		{
			ScreenshotDestination& dest(m_options.GetDestination(i));
			if(dest.enabled)
			{
				sel = i;
				break;
			}
		}

		if(sel == -1)
		{
			sel = 0;
		}
		else
		{
			sel += delta;
			// wrap the value.
			while(sel < 0) sel += total;
			while(sel >= total) sel -= total;
		}

		// disable all
		for(size_t i = 0; i < total; i ++)
		{
			ScreenshotDestination& dest(m_options.GetDestination(i));
			dest.enabled = false;
		}

		// enable the destination one.
		ScreenshotDestination& dest(m_options.GetDestination(sel));
		dest.enabled = true;

		UpdateCurrentDestination();
	}

	LRESULT OnNextDestination(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		SelectNewDestination(1);
		return 0;
	}

	LRESULT OnPrevDestination(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		SelectNewDestination(-1);
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

	static const int ToolbarButtonCount = 8;
	static const int ToolbarButtonImageSize = 24;
	CPNGButton tbButons[ToolbarButtonCount];

	CToolTipCtrl toolTip;

  // zoom stuff
  int m_zoomFactorIndex;
  std::vector<float> m_zoomFactors;

  HICON m_hIcon;
  HICON m_hIconSmall;

	// info stuff
	CPoint m_infoCursorPos;
	float m_infoZoomFactor;
	//CRect m_infoSelection;
	std::wstring m_infoText;

	void UpdateInfoBox()
	{
		if(!m_infoUpdate)
			return;

		RgbPixel32 pixel = m_editWnd.GetPixel_(m_infoCursorPos);
    CRect m_infoSelection = m_editWnd.GetSelection();

		float h = 0, s = 0, v = 0;
		RGBtoHSV((float)R(pixel) / 255, (float)G(pixel) / 255,(float)B(pixel) / 255, &h, &s, &v);

		std::wstring hue;
		if(LibCC::SinglePrecisionFloat(h).IsQNaN())
		{
			hue = L"n/a";
		}
		else
		{
			hue = LibCC::Format(L"%%").i((int)h).c(0x00B0).Str();
		}

		LibCC::Format info = LibCC::Format(
			"Zoom:|%^%||"
			"Pos:|(%,%)||"
			"Sel:|TL(%,%)|BR(%,%)|% x %||"
			"RGB:|#%%%|(%,%,%)||"
			"H:%|S:%^%|V:%^%||"
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
			(hue)
			.i((int)(s*100.0f))
			.i((int)(v*100.0f))
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

