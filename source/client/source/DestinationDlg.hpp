//
// DestinationDlg.cpp - screenie's main options window
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#ifndef _SCREENIE_DESTINATIONDLG_H_
#define _SCREENIE_DESTINATIONDLG_H_

#include "ScreenshotDestination.hpp"
#include "ScreenshotOptions.hpp"
#include "resource.h"

class CDestinationDlg :
	public CDialogImpl<CDestinationDlg>,
	public CDialogResize<CDestinationDlg>
{
public:
	enum { IDD = IDD_MAINDLG };

	CDestinationDlg(ScreenshotOptions& options, const tstd::tstring& OKbuttonText) :
    m_optionsCopy(options),
    m_optionsFinal(options),
    m_OKbuttonText(OKbuttonText),
    m_hIconSmall(0),
    m_hIcon(0)
	{
	}

	~CDestinationDlg()
	{
    if(m_hIcon) DestroyIcon(m_hIcon);
    if(m_hIconSmall) DestroyIcon(m_hIconSmall);
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(CDestinationDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)

		NOTIFY_HANDLER(IDC_DESTINATIONS, LVN_ITEMCHANGED, OnItemChanged)
		NOTIFY_HANDLER(IDC_DESTINATIONS, LVN_ITEMACTIVATE, OnItemActivated)

		COMMAND_HANDLER(IDC_SHOWSTATUS, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CONFIRM, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CROPPING, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_INCLUDECURSOR, BN_CLICKED, OnCheckboxClicked)
    COMMAND_HANDLER(IDC_AUTOSTART, BN_CLICKED, OnCheckboxClicked)
    COMMAND_HANDLER(IDC_SHOWSPLASH, BN_CLICKED, OnCheckboxClicked)
    COMMAND_HANDLER(IDC_ENABLEARCHIVE, BN_CLICKED, OnCheckboxClicked)

		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_NEW, OnNewDestination)
		COMMAND_ID_HANDLER(IDC_DUPLICATE, OnDuplicateDestination)
		COMMAND_ID_HANDLER(IDC_EDIT, OnEditDestination)
		COMMAND_ID_HANDLER(IDC_REMOVE, OnRemoveDestination)
		COMMAND_ID_HANDLER(IDOK, OnOK)

		COMMAND_HANDLER(IDC_MOVEUP, BN_CLICKED, OnBnClickedMoveup)
		COMMAND_HANDLER(IDC_MOVEDOWN, BN_CLICKED, OnBnClickedMovedown)
		CHAIN_MSG_MAP(CDialogResize<CDestinationDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CDestinationDlg)
    DLGRESIZE_CONTROL(IDC_GENERAL_GROUP, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_SCREENSHOTACTION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_INCLUDECURSOR, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_SHOWSTATUS, DLSZ_SIZE_X)

		DLGRESIZE_CONTROL(IDC_MOVEUP, DLSZ_MOVE_X)
    DLGRESIZE_CONTROL(IDC_MOVEDOWN, DLSZ_MOVE_X)

    DLGRESIZE_CONTROL(IDC_DESTINATIONS_GROUP, DLSZ_SIZE_Y | DLSZ_SIZE_X)
    DLGRESIZE_CONTROL(IDC_DESTINATIONS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_NEW, DLSZ_MOVE_Y | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_EDIT, DLSZ_MOVE_Y | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_DUPLICATE, DLSZ_MOVE_Y | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_REMOVE, DLSZ_MOVE_Y | DLSZ_MOVE_X)

    DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_Y | DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	void DisplayListContextMenu();

	int GetSelectedDestination();
	void PopulateDestinationList(Guid idSelection, bool select);
	void PopulateDestinationList() { PopulateDestinationList(Guid(), false); }
	void SetEnabledButtons();

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnItemActivated(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
  
	LRESULT OnNewDestination(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditDestination(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDuplicateDestination(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRemoveDestination(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCheckboxClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnableArchive(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void CloseDialog(bool bSaveOptions);
private:
	CListViewCtrl m_listView;
	CButton m_editButton;
	CButton m_duplicateButton;
	CButton m_removeButton;
	CButton m_moveUp;
	CButton m_moveDown;
	ScreenshotOptions m_optionsCopy;
	ScreenshotOptions& m_optionsFinal;// this is the external ref that we copy to when the user hits OK
  tstd::tstring m_OKbuttonText;

  HICON m_hIcon;
  HICON m_hIconSmall;

	void MoveSelectedDestination(int direction);

public:
	LRESULT OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedMoveup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedMovedown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

bool IsDestinationsDialogVisible();

#endif