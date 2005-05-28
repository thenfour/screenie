#ifndef SCREENIE_STATUSDLG_HPP
#define SCREENIE_STATUSDLG_HPP

#include "ScreenshotOptions.hpp"


// interface for passing to things that d
struct StatusWindow
{
	enum MessageType
	{
		MSG_INFO,
		MSG_ERROR
	};

	virtual ~StatusWindow() { }

	virtual void ClearMessages() = 0;
	virtual void PrintMessage(const MessageType type, const tstd::tstring& destination,
		const tstd::tstring& message) = 0;
};

class CStatusDlg :
	public StatusWindow,
	public CDialogImpl<CStatusDlg>,
	public CMessageFilter,
	public CDialogResize<CStatusDlg>
{
private:
	CImageList m_imageList;
	CListViewCtrl m_listView;
  ScreenshotOptions& m_options;
public:
	enum { IDD = IDD_STATUS };

  CStatusDlg(ScreenshotOptions& options) : m_options(options) { }
	virtual ~CStatusDlg() { }

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	void CloseDialog(int nVal);
	
	BEGIN_MSG_MAP(CStatusDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		NOTIFY_HANDLER(IDC_MESSAGES, NM_RCLICK, OnRightClick)
		COMMAND_ID_HANDLER(ID_STATUSCONTEXTMENU_COPYTOCLIPBOARD, OnCopy)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		CHAIN_MSG_MAP(CDialogResize<CStatusDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CStatusDlg)
		DLGRESIZE_CONTROL(IDC_MESSAGES, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y | DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	//
	// StatusWindow implementation
	//

	void ClearMessages();
	void PrintMessage(const MessageType type, const tstd::tstring& destination,
		const tstd::tstring& message);

	//
	// message handlers and whatnot
	//

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnRightClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

#endif