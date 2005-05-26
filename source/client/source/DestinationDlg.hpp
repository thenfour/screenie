//
// DestinationDlg.cpp - screenie's main options window
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#ifndef _SCREENIE_DESTINATIONDLG_H_
#define _SCREENIE_DESTINATIONDLG_H_

#include "ScreenshotDestination.hpp"
#include "ScreenshotOptions.hpp"

class CDestinationDlg :
	public CDialogImpl<CDestinationDlg>,
	public CDialogResize<CDestinationDlg>
{
public:
	enum { IDD = IDD_MAINDLG };

	CDestinationDlg(ScreenshotOptions& options)
		: m_screenshotOptions(options)
	{
	}

	~CDestinationDlg()
	{
	}

	ScreenshotOptions GetOptions() const
	{
		return m_screenshotOptions;
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(CDestinationDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)

		NOTIFY_HANDLER(IDC_DESTINATIONS, LVN_ITEMCHANGED, OnItemChanged)

		COMMAND_HANDLER(IDC_SHOWSTATUS, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CONFIRM, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CROPPING, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_INCLUDECURSOR, BN_CLICKED, OnCheckboxClicked)

		COMMAND_ID_HANDLER(IDC_NEW, OnNewDestination)
		COMMAND_ID_HANDLER(IDC_EDIT, OnEditDestination)
		COMMAND_ID_HANDLER(IDC_REMOVE, OnRemoveDestination)
		COMMAND_ID_HANDLER(IDOK, OnOK)

		CHAIN_MSG_MAP(CDialogResize<CDestinationDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CDestinationDlg)
		DLGRESIZE_CONTROL(IDC_DESTINATIONS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CONFIRM, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CROPPING, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_INCLUDECURSOR, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_SHOWSTATUS, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_NEW, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_EDIT, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_REMOVE, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y | DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	void DisplayListContextMenu();

	int GetSelectedDestination();
	void PopulateDestinationList();

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnNewDestination(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditDestination(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRemoveDestination(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCheckboxClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void CloseDialog(int nVal);
private:
	CListViewCtrl m_listView;
	ScreenshotOptions& m_screenshotOptions;
};

#endif