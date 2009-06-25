//
// CropDlg.hpp - modal cropping dialog
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//

#ifndef _SCREENIE_CROPDLG_H_
#define _SCREENIE_CROPDLG_H_

#include "resource.h"
#include "ImageEditCtrl.hpp"

class CCropDlg :
	public CDialogImpl<CCropDlg>,
	public CDialogResize<CCropDlg>
{
public:
	enum {
		IDD = IDD_DIALOG1
	};

	CCropDlg()
  {
  }

	~CCropDlg()
  {
  }

	BEGIN_MSG_MAP(CCropDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)

		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnPaste)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
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

//    DLGRESIZE_CONTROL(IDC_EDITINPAINT, DLSZ_MOVE_Y | DLSZ_MOVE_X)
//    DLGRESIZE_CONTROL(IDC_SELECTIONSTATIC, DLSZ_SIZE_X)
//    DLGRESIZE_CONTROL(IDC_CONFIGURE, DLSZ_MOVE_Y | DLSZ_MOVE_X)
//	DLGRESIZE_CONTROL(IDC_ETCHEDLINE, DLSZ_MOVE_Y | DLSZ_MOVE_X)

		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_Y | DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()


	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& handled)
	{
		//return m_editWnd.OnCopy(0, 0, 0, handled);
		return 0;
	}

	LRESULT OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& handled)
	{
		//return m_editWnd.OnPaste(0, 0, 0, handled);
		return 0;
	}

	LRESULT OnMouseWheel(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
  {
    //handled = TRUE;
    //int newFactorIndex = m_zoomFactorIndex + (GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
    //AttemptNewFactorIndex(newFactorIndex, true);
    return 0;
  }

	LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//HDROP hDrop = (HDROP)wParam;
		//wchar_t buf[1001] = {0};
		//DragQueryFile(hDrop, 0, buf, 1000);

		//if(!ReplaceImageWithFile(buf))
		//{
		//	MessageBox(L"The file could not be loaded.", L"Screenie", MB_OK);
		//}

		return 0;
	}


	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		RECT rcSplitter;
		DragAcceptFiles();

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

    // create the image window.
    RECT rcImage;
    ::GetWindowRect(GetDlgItem(IDC_IMAGE), &rcImage);
    ScreenToClient(&rcImage);
		rcSplitter = rcImage;
    ::DestroyWindow(GetDlgItem(IDC_IMAGE));
    m_editWnd.Create(*this, rcImage, _T(""), WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE, IDC_IMAGE);

		m_editWnd.SetDocument(Gdiplus::Bitmap(L"test.bmp"));

    // create zoom window
    ::GetWindowRect(GetDlgItem(IDC_PREVIEW), &rcImage);
    ScreenToClient(&rcImage);
		rcSplitter.left = rcImage.left;
    ::DestroyWindow(GetDlgItem(IDC_PREVIEW));
    m_zoomWnd.Create(*this, rcImage, _T(""), WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE, IDC_PREVIEW);
		m_zoomWnd.ShowWindow(SW_HIDE);
		m_zoomWnd.SetDocument(Gdiplus::Bitmap(L"test.bmp"));


		::DestroyWindow(GetDlgItem(IDC_SPLITTER));
		m_splitter.Create(*this, rcSplitter, L"", WS_CHILD | WS_VISIBLE, /*exstyle*/0, IDC_SPLITTER, /*lparam*/0);
		m_splitter.SetSplitterExtendedStyle(0);
		m_zoomWnd.SetParent(m_splitter);
		m_editWnd.SetParent(m_splitter);
		m_splitter.SetSplitterPanes(m_zoomWnd, m_editWnd, false);

    DlgResize_Init(true, true, WS_CLIPCHILDREN);

		// Load window placement settings.
		SetWindowPos(NULL, 0, 0, 800, 600, SWP_NOMOVE);
		CenterWindow(NULL);

		return 0;
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

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(IDCANCEL);
		return 0;
	}

	LRESULT OnHighlightTool(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
	  return 0;
	}

	LRESULT OnCroppingTool(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
	  return 0;
	}

	LRESULT OnResetImage(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
	  return 0;
	}

	void CloseDialog(int nVal)
	{
		EndDialog(nVal); 
	}
private:

	CImageEditWindow m_editWnd;
	CImageEditWindow m_zoomWnd;

	bool m_didCropping;
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
		//RgbPixel32 pixel = m_editWnd.GetPixel_(m_infoCursorPos);

		//LibCC::Format info = LibCC::Format(
		//	"Zoom: %^%|"
		//	"Pos: (%,%)|"
		//	"Sel: (%,%)-(%,%)|"
		//	"Sel: % x %|"
		//	"RGB: #%%% (%, %, %)"
		//	)
		//	.f<0>(m_infoZoomFactor*100)
		//	.i(m_infoCursorPos.x)
		//	.i(m_infoCursorPos.y)
  //    .i(m_infoSelection.left)
  //    .i(m_infoSelection.top)
  //    .i(m_infoSelection.right)
  //    .i(m_infoSelection.bottom)
  //    .i(m_infoSelection.Width())
  //    .i(m_infoSelection.Height())
		//	.i<16,2>(R(pixel))
		//	.i<16,2>(G(pixel))
		//	.i<16,2>(B(pixel))
		//	.i(R(pixel))
		//	.i(G(pixel))
		//	.i(B(pixel))
		//	;
		//if(m_infoText != info.CStr())
		//{
		//	m_infoText = info.CStr();
		//	SetDlgItemText(IDC_INFOBOX, m_infoText.c_str());
		//}
	}

public:
  LRESULT OnStnClickedZoomCaption(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

#endif

